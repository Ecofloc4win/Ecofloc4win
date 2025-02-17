const { getCheckedPIDs, launchMonitoring } = require("./Js/Utils.cjs");
const express = require('express');
const { exec } = require('child_process');
const cors = require('cors');
const fs = require('fs');
const path = require('path');

// Server configuration
const APP = express();
const PORT = 3030;
const PATHS = {
    PROCESS_JSON: './Json/process.json',
    METRICS_DIRECTORY: path.join(__dirname, 'Json/'),
    MONITORING: path.join(__dirname, 'Ecofloc4Win.exe'),
    CONFIGURATOR: path.join(__dirname, '..', '..', 'EcoflocConfigurator.exe'),
    PID_RECUP: path.join(__dirname, 'PIDRecup.exe')
};

// Process state management
let IS_PROCESS_RUNNING = false;
let IS_CONFIGURATOR_RUNNING = false;
global.STOPPING = false;

// File system utilities
const fileUtils = {
    readJSON: (filePath) => {
        try {
            return fs.existsSync(filePath) ? JSON.parse(fs.readFileSync(filePath, 'utf8')) : null;
        } catch (error) {
            console.error(`Error reading JSON file ${filePath}:`, error);
            return null;
        }
    },

    writeJSON: (filePath, data) => {
        try {
            fs.writeFileSync(filePath, JSON.stringify(data, null, 4), 'utf8');
            return true;
        } catch (error) {
            console.error(`Error writing JSON file ${filePath}:`, error);
            return false;
        }
    },

    resetMonitoringFiles: (pids) => {
        pids.forEach(pid => {
            const filePath = path.join(PATHS.METRICS_DIRECTORY, `${pid}.json`);
            if (!fs.existsSync(filePath) || fs.statSync(filePath).size === 0) {
                fileUtils.writeJSON(filePath, {
                    apps: [
                        {
                            pid: pid,
                            power_w_CPU: 0,
                            power_w_GPU: 0,
                            power_w_NIC: 0,
                            power_w_SD: 0,
                            color: '#' + Math.floor(Math.random()*16777215).toString(16).padStart(6, '0')
                        }
                    ],
                    time: Math.floor(Date.now() / 1000)
                });
            }
        });
    },

    exportToCSV: (graphData) => {
        console.log('Received graph data:', Object.keys(graphData));
        
        const header = ['Timestamp', 'Component', 'PID', 'Value'];
        const rows = [];
        
        // Add header
        rows.push(header.join(','));
        
        try {
            // Get all timestamps from the data
            const allTimestamps = new Set();
            
            // Collect all timestamps first
            Object.entries(graphData).forEach(([component, data]) => {
                Object.entries(data).forEach(([pid, trace]) => {
                    if (trace && Array.isArray(trace.x)) {
                        trace.x.forEach(timestamp => allTimestamps.add(timestamp));
                    }
                });
            });

            // Sort timestamps
            const sortedTimestamps = Array.from(allTimestamps).sort((a, b) => a - b);
            
            // Add data rows
            sortedTimestamps.forEach(timestamp => {
                Object.entries(graphData).forEach(([component, data]) => {
                    Object.entries(data).forEach(([pid, trace]) => {
                        if (trace && Array.isArray(trace.x) && Array.isArray(trace.y)) {
                            const index = trace.x.indexOf(timestamp);
                            if (index !== -1 && !isNaN(trace.y[index])) {
                                const row = [
                                    new Date(timestamp * 1000).toLocaleString(),
                                    component,
                                    pid,
                                    trace.y[index].toFixed(2)
                                ];
                                rows.push(row.join(','));
                            }
                        }
                    });
                });
            });
        } catch (error) {
            console.error('Error processing data:', error);
            throw new Error(`Failed to process data: ${error.message}`);
        }
        
        return rows.join('\n');
    }
};

// Au début du fichier, ajoutons une map pour suivre les processus
const monitoringProcesses = new Map();

// Process management utilities
const processUtils = {
    cleanup: async () => {
        global.STOPPING = true;

        // Utiliser tasklist pour obtenir les PIDs
        const { stdout } = await new Promise((resolve, reject) => {
            exec('tasklist /FI "IMAGENAME eq Ecofloc4Win.exe" /FO CSV /NH', (error, stdout, stderr) => {
                if (error) reject(error);
                else resolve({ stdout, stderr });
            });
        });

        // Extraire les PIDs
        const pids = stdout.split('\n')
            .map(line => line.trim())
            .filter(line => line.startsWith('"Ecofloc4Win.exe"'))
            .map(line => line.split(',')[1].replace(/"/g, '').trim());

        // Tuer chaque processus individuellement
        for (const pid of pids) {
            try {
                await new Promise((resolve, reject) => {
                    exec(`taskkill /F /PID ${pid}`, (error) => {
                        if (error) console.error(`Error killing PID ${pid}:`, error);
                        resolve();
                    });
                });
            } catch (error) {
                console.error(`Failed to kill process ${pid}:`, error);
            }
        }

        // Nettoyer la map des processus
        if (global.monitoringProcesses) {
            global.monitoringProcesses.clear();
        }

        // Supprimer les fichiers JSON
        try {
            const files = fs.readdirSync(PATHS.METRICS_DIRECTORY);
            files.forEach(file => {
                if (file.endsWith('.json') && file !== 'process.json') {
                    const filePath = path.join(PATHS.METRICS_DIRECTORY, file);
                    fs.unlinkSync(filePath);
                }
            });
        } catch (error) {
            console.error('Error cleaning JSON files:', error);
        }

        IS_PROCESS_RUNNING = false;
        global.STOPPING = false;
    },

    killProcess: (pid) => {
        if (global.monitoringProcesses && global.monitoringProcesses.has(pid)) {
            const processes = global.monitoringProcesses.get(pid);
            processes.forEach(proc => {
                try {
                    exec(`taskkill /F /T /PID ${proc.pid}`);
                } catch (error) {
                    console.error(`Error killing process for PID ${pid}:`, error);
                }
            });
            global.monitoringProcesses.delete(pid);
        }
    }
};

// Initialize server
APP.use(cors());
APP.use(express.json());

// Set working directory
const srcPath = path.resolve('./ecofloc-UI/src');
if (fs.existsSync(srcPath)) {
    process.chdir(srcPath);
} else {
    console.error(`Directory ${srcPath} does not exist`);
    process.exit(1);
}

// Clean shutdown handlers
process.on('SIGINT', processUtils.cleanup);
process.on('SIGTERM', processUtils.cleanup);

// API endpoints
APP.post('/execute', async (req, res) => {
    if (IS_PROCESS_RUNNING) {
        return res.status(400).json({ success: false, message: 'Process already running' });
    }

    try {
        // S'assurer que le nettoyage est terminé avant de continuer
        await processUtils.cleanup();
        
        // Réinitialiser le flag d'arrêt
        global.STOPPING = false;
        
        const pidsToExecute = await getCheckedPIDs();
        
        if (!pidsToExecute || pidsToExecute.length === 0) {
            return res.status(400).json({ success: false, message: 'No PIDs selected for monitoring' });
        }

        // Réinitialiser les fichiers de monitoring
        fileUtils.resetMonitoringFiles(pidsToExecute);

        // Utiliser un intervalle par défaut de 1000ms si non spécifié
        const interval = req.body.interval || 1000;

        // Lancer les processus de monitoring
        const processes = pidsToExecute.map(pid => 
            launchMonitoring(pid, PATHS.MONITORING, PATHS.METRICS_DIRECTORY, interval)
        );

        if (processes.some(p => p === null)) {
            throw new Error('Failed to start one or more monitoring processes');
        }

        IS_PROCESS_RUNNING = true;
        res.json({ success: true });
    } catch (error) {
        console.error(`Unexpected error: ${error.message}`);
        await processUtils.cleanup();
        res.status(500).json({ success: false, message: 'Error launching process' });
    }
});

APP.post('/stop', (req, res) => {
    try {
        processUtils.cleanup();
        res.json({ success: true, message: 'All monitoring processes stopped and files cleaned' });
    } catch (error) {
        console.error(`Error stopping processes: ${error.message}`);
        res.status(500).json({ 
            success: false, 
            message: 'Error stopping processes and cleaning files' 
        });
    }
});

APP.post('/configurator', (req, res) => {
    if (IS_CONFIGURATOR_RUNNING) {
        return res.status(400).json({ success: false, message: 'Configurator already running' });
    }

    try {
        exec(`"${PATHS.CONFIGURATOR}"`, (error) => {
            IS_CONFIGURATOR_RUNNING = false;
            if (error) {
                console.error(`Execution error: ${error.message}`);
                return res.status(500).json({ success: false, message: 'Error launching configurator' });
            }
            res.json({ success: true, message: 'Configurator launched successfully' });
        });

        IS_CONFIGURATOR_RUNNING = true;
    } catch (error) {
        console.error(`Unexpected error: ${error.message}`);
        IS_CONFIGURATOR_RUNNING = false;
        res.status(500).json({ success: false, message: 'Error launching configurator' });
    }
});

APP.post('/updateProcessListState', (req, res) => {
    const { list, state } = req.body;
    const data = fileUtils.readJSON(PATHS.PROCESS_JSON);
    
    if (data) {
        data.forEach(process => {
            if (list.includes(process.name)) {
                process.pids.forEach(pidInfo => pidInfo.checked = state);
            }
        });
        fileUtils.writeJSON(PATHS.PROCESS_JSON, data);
    }
    
    res.json({ success: true, message: 'Process list updated', list });
});

APP.post('/changePidState', (req, res) => {
    const { processName, pid, state } = req.body;
    const data = fileUtils.readJSON(PATHS.PROCESS_JSON);
    if (data) {
        data.forEach(process => {
            if (process.name === processName) {
                process.pids.forEach(pidInfo => {
                    if (pidInfo.numeroPid == pid) {
                        pidInfo.checked = state;
                    }
                });
            }
        });
        fileUtils.writeJSON(PATHS.PROCESS_JSON, data);
    }else 
    {
        console.error("JSON File impossible to open");
        return;
    }
    
    res.json({ success: true, message: 'PID state updated' });
});

APP.post('/changeListePidState', (req, res) => {
    const { list, state } = req.body;
    console.log('Length receive :', list.length);
    console.log('state :', state);
    const data = fileUtils.readJSON(PATHS.PROCESS_JSON);
    if (data) {
        data.forEach(process => {
            if (list.includes(process.name)) {
                process.pids.forEach(pidInfo => {
                    pidInfo.checked = state;
                });
            }
        });
        fileUtils.writeJSON(PATHS.PROCESS_JSON, data);
    }
    
    console.log('Valeur modifiée avec succès.');
    res.json({ message: 'Liste reçue avec succès', receivedList: list });
});

APP.get('/events', (req, res) => {
    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection', 'keep-alive');

    console.log('Client SSE connected');

    res.write(`data: ${JSON.stringify({ message: 'Connexion established' })}\n\n`);

    const filePath = './Json/process.json';

    const watcher = fs.watch(filePath, (eventType) => {
        if (eventType === 'change') {
            console.log(`File modified: ${filePath}`);

            fs.readFile(filePath, 'utf8', (err, data) => {
                if (err) {
                    console.error('Error while reading the file :', err);
                    return;
                }
                try {                    
                    const jsonData = JSON.parse(data);
                    res.write(`data: ${JSON.stringify(jsonData)}\n\n`);
                } catch (parseErr) {
                    // console.error('Error of parsing JSON:', parseErr);
                    // res.write(`data: ${JSON.stringify({ error: 'Invalid JSON format' })}\n\n`);
                }
            });
        }
    });

    req.on('close', () => {
        console.log('Client SSE deconnected');
        watcher.close();
    });
});

APP.post('/export-csv', (req, res) => {
    try {
        const graphData = req.body;
        
        if (!graphData || typeof graphData !== 'object') {
            throw new Error('Invalid data format received');
        }
        
        const csvContent = fileUtils.exportToCSV(graphData);
        
        if (!csvContent) {
            throw new Error('No data to export');
        }
        
        res.setHeader('Content-Type', 'text/csv');
        res.setHeader('Content-Disposition', 'attachment; filename=monitoring_data.csv');
        res.send(csvContent);
    } catch (error) {
        console.error('Error exporting CSV:', error);
        res.status(500).send(error.message);
    }
});

APP.post('/update', (req, res) => {
    try {
        const command = `"${PATHS.PID_RECUP}" ${PATHS.PROCESS_JSON}`;
        const process = exec(command, (error, stdout, stderr) => {
            IS_CONFIGURATOR_RUNNING = false;
            if (error) {
                console.error(`Error of execution : ${error.message}`);
                return;
            }
            if (stderr) {
                console.warn(`Stderr : ${stderr}`);
                return;
            }
            console.log(`Stdout : ${stdout}`);
        });

        
    } catch (error) {
        console.error(`Unexpected error : ${error.message}`);
        IS_CONFIGURATOR_RUNNING = false;
        return res.status(500).json({ success: false, message: 'Error during the update' });
    }
});

APP.get('/process-data', (req, res) => {
    try {
        const data = fileUtils.readJSON(PATHS.PROCESS_JSON);
        if (!data) {
            return res.status(404).json({ error: 'No process data found' });
        }
        res.json(data);
    } catch (error) {
        console.error('Error reading process data:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

APP.get('/monitored-pids', (req, res) => {
    try {
        const files = fs.readdirSync(PATHS.METRICS_DIRECTORY);
        const pids = files
            .filter(file => file.endsWith('.json'))
            .map(file => file.replace('.json', ''));
        res.json(pids);
    } catch (error) {
        console.error('Error reading monitored PIDs:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

APP.get('/metrics/:pid', (req, res) => {
    try {
        const filePath = path.join(PATHS.METRICS_DIRECTORY, `${req.params.pid}.json`);
        const data = fileUtils.readJSON(filePath);
        if (!data) {
            return res.status(404).json({ error: 'No metrics found for this PID' });
        }
        res.json(data);
    } catch (error) {
        console.error('Error reading metrics:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

// Start server
APP.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
    
    if (!fs.existsSync(PATHS.MONITORING)) {
        console.error(`Ecofloc4Win.exe not found at ${PATHS.MONITORING}`);
        process.exit(1);
    }
    
    if (!fs.existsSync(PATHS.METRICS_DIRECTORY)) {
        fs.mkdirSync(PATHS.METRICS_DIRECTORY, { recursive: true });
    }
    
    fileUtils.resetMonitoringFiles([]);
});

