const { exec } = require('child_process');
const path = require('path');
const fs = require('fs');

async function getCheckedPIDs() {
    try {
        const PROCESS_JSON_PATH = path.join(__dirname, '..', 'Json', 'process.json');
        
        if (!fs.existsSync(PROCESS_JSON_PATH)) {
            console.error('Process JSON file not found');
            return [];
        }

        const DATA = JSON.parse(fs.readFileSync(PROCESS_JSON_PATH, 'utf8'));
        return DATA ? DATA.filter(process => process.pids)
                         .flatMap(process => process.pids)
                         .filter(pid => pid.checked)
                         .map(pid => pid.numeroPid) : [];
    } catch (error) {
        console.error('Error reading process.json:', error);
        return [];
    }
}

function launchMonitoring(PID, MONITORING_PATH, METRICS_DIRECTORY, INTERVAL) {
    // Check if process is actually running
    try {
        exec(`tasklist /FI "IMAGENAME eq Ecofloc4Win.exe" /FO CSV /NH`, (error, stdout) => {
            if (!error) {
                const RUNNING_PIDS = stdout.split('\n')
                    .map(line => line.trim())
                    .filter(line => line.startsWith('"Ecofloc4Win.exe"'))
                    .map(line => line.split(',')[1].replace(/"/g, '').trim());

                // Clean up processes that are no longer running
                if (global.MONITORING_PROCESSES) {
                    for (const [pid, processes] of global.MONITORING_PROCESSES.entries()) {
                        if (!RUNNING_PIDS.includes(pid.toString())) {
                            global.MONITORING_PROCESSES.delete(pid);
                        }
                    }
                }
            }
        });
    } catch (error) {
        console.error('Error checking running processes:', error);
    }

    const COMPONENTS = ["--cpu", "--nic", "--sd"];
    const EXPORT_FILE_PATH = path.join(METRICS_DIRECTORY, `${PID}.json`);
    const COMMAND = `"${MONITORING_PATH}" ${COMPONENTS.join(' ')} -p ${PID} -i ${INTERVAL} -f "${EXPORT_FILE_PATH}"`;
    
    console.log(`Launching command: ${COMMAND}`);
    
    try {
        const PROCESS = exec(COMMAND, {
            cwd: path.dirname(MONITORING_PATH),
            windowsHide: true,
            detached: true,
            stdio: ['ignore', 'pipe', 'pipe']
        });

        if (!global.MONITORING_PROCESSES) {
            global.MONITORING_PROCESSES = new Map();
        }
        
        if (!global.MONITORING_PROCESSES.has(PID)) {
            global.MONITORING_PROCESSES.set(PID, []);
        }

        PROCESS.on('exit', (code, signal) => {
            console.log(`[PID ${PID}] Process exited with code ${code} and signal ${signal}`);
            // Remove process from map when it terminates
            const PROCESSES = global.MONITORING_PROCESSES.get(PID);
            if (PROCESSES) {
                const INDEX = PROCESSES.indexOf(PROCESS);
                if (INDEX > -1) {
                    PROCESSES.splice(INDEX, 1);
                }
                if (PROCESSES.length === 0) {
                    global.MONITORING_PROCESSES.delete(PID);
                }
            }

            if (code !== 0 && !global.STOPPING) {
                console.log(`[PID ${PID}] Restarting process...`);
                setTimeout(() => {
                    if (!global.STOPPING) {
                        launchMonitoring(PID, MONITORING_PATH, METRICS_DIRECTORY, INTERVAL);
                    }
                }, 1000);
            }
        });

        global.MONITORING_PROCESSES.get(PID).push(PROCESS);
        return PROCESS;
    } catch (error) {
        console.error(`Failed to launch monitoring for PID ${PID}:`, error);
        return null;
    }
}

module.exports = { getCheckedPIDs, launchMonitoring };