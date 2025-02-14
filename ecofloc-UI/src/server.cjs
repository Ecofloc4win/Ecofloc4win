const express = require('express');
const { exec } = require('child_process');
const cors = require('cors');
const fs = require('fs');
const path = require('path');

// Server configuration
const app = express();
const PORT = 3030;
const PATHS = {
    processJson: './Json/process.json',
    systemMonitoring: './Json/system_monitoring.json',
    generator: path.join(__dirname, 'Generator.exe'),
    configurator: path.join(__dirname, '..', '..', 'EcoflocConfigurator.exe'),
    pidRecup: path.join(__dirname, 'PIDRecup.exe')
};

// Process state management
let currentProcess = null;
let processRunning = false;
let configuratorRunning = false;

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

    resetMonitoringFile: () => {
        // Only reset if the file doesn't exist or is empty
        if (!fs.existsSync(PATHS.systemMonitoring) || 
            fs.statSync(PATHS.systemMonitoring).size === 0) {
            fileUtils.writeJSON(PATHS.systemMonitoring, { apps: [] });
        }
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

// Process management utilities
const processUtils = {
    cleanup: () => {
        if (currentProcess) {
            try {
                process.kill(currentProcess.pid, 0);
                process.kill(currentProcess.pid);
            } catch (error) {
                if (error.code !== 'ESRCH') {
                    console.error('Error during process cleanup:', error);
                }
            } finally {
                currentProcess = null;
                processRunning = false;
            }
        }
        processRunning = false;
    },

    killProcess: (pid, callback) => {
        exec(`taskkill /PID ${pid} /T /F`, callback);
    }
};

// Initialize server
app.use(cors());
app.use(express.json());

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
app.post('/execute', (req, res) => {
    if (processRunning) {
        return res.status(400).json({ success: false, message: 'Process already running' });
    }

    try {
        processUtils.cleanup();
        fileUtils.resetMonitoringFile();

        currentProcess = exec(`"${PATHS.generator}" ${PATHS.systemMonitoring}`);
        
        currentProcess.on('error', (error) => {
            console.error(`Process error: ${error.message}`);
            processUtils.cleanup();
            res.status(500).json({ success: false, message: 'Process error occurred' });
        });

        currentProcess.on('exit', (code, signal) => {
            console.log(`Process exited with code ${code} and signal ${signal}`);
            processUtils.cleanup();
        });

        processRunning = true;
        res.json({ success: true});
    } catch (error) {
        console.error(`Unexpected error: ${error.message}`);
        processUtils.cleanup();
        res.status(500).json({ success: false, message: 'Error launching process' });
    }
});

app.post('/stop', (req, res) => {
    if (!processRunning || !currentProcess) {
        return res.status(400).json({ success: false, message: 'No process running' });
    }

    try {
        processUtils.killProcess(currentProcess.pid, (error) => {
            if (error) {
                console.error('Error killing process:', error);
                res.status(500).json({ success: false, message: 'Error stopping process' });
            } else {
                processUtils.cleanup();
                res.json({ success: true });
            }
        });
    } catch (error) {
        console.error(`Error stopping process: ${error.message}`);
        processUtils.cleanup();
        res.status(500).json({ success: false, message: 'Error stopping process' });
    }
});

app.post('/configurator', (req, res) => {
    if (configuratorRunning) {
        return res.status(400).json({ success: false, message: 'Configurator already running' });
    }

    try {
        exec(`"${PATHS.configurator}"`, (error) => {
            configuratorRunning = false;
            if (error) {
                console.error(`Execution error: ${error.message}`);
                return res.status(500).json({ success: false, message: 'Error launching configurator' });
            }
            res.json({ success: true, message: 'Configurator launched successfully' });
        });

        configuratorRunning = true;
    } catch (error) {
        console.error(`Unexpected error: ${error.message}`);
        configuratorRunning = false;
        res.status(500).json({ success: false, message: 'Error launching configurator' });
    }
});

app.post('/updateProcessListState', (req, res) => {
    const { list, state } = req.body;
    const data = fileUtils.readJSON(PATHS.processJson);
    
    if (data) {
        data.forEach(process => {
            if (list.includes(process.name)) {
                process.pids.forEach(pidInfo => pidInfo.checked = state);
            }
        });
        fileUtils.writeJSON(PATHS.processJson, data);
    }
    
    res.json({ success: true, message: 'Process list updated', list });
});

app.post('/changePidState', (req, res) => {
    const { processName, pid, state } = req.body;
    const data = fileUtils.readJSON(PATHS.processJson);
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
        fileUtils.writeJSON(PATHS.processJson, data);
    }else 
    {
        console.error("JSON File impossible to open");
        return;
    }
    
    res.json({ success: true, message: 'PID state updated' });
});

app.post('/changeListePidState', (req, res) => {
    const { list, state } = req.body;
    console.log('Length receive :', list.length);
    console.log('state :', state);
    const data = fileUtils.readJSON(PATHS.processJson);
    if (data) {
        data.forEach(process => {
            if (list.includes(process.name)) {
                process.pids.forEach(pidInfo => {
                    pidInfo.checked = state;
                });
            }
        });
        fileUtils.writeJSON(PATHS.processJson, data);
    }
    
    console.log('Valeur modifiée avec succès.');
    res.json({ message: 'Liste reçue avec succès', receivedList: list });
});

app.get('/events', (req, res) => {
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

app.post('/export-csv', (req, res) => {
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

app.post('/update', (req, res) => {
    try {
        const command = `"${PATHS.pidRecup}" ${PATHS.processJson}`;
        const process = exec(command, (error, stdout, stderr) => {
            configuratorRunning = false;
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
        configuratorRunning = false;
        return res.status(500).json({ success: false, message: 'Error during the update' });
    }
});

// Start server
app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
    fileUtils.resetMonitoringFile();
});
