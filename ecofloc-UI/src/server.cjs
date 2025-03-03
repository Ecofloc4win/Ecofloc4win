/**
 * @file server.cjs
 * @brief Express.js server for process monitoring and execution management.
 *
 * This server provides APIs for executing and managing processes,
 * handling JSON file operations, and exporting monitoring data.
 * @author Ecofloc's Team
 * @lastupdate 2025-02-18
 */

/**
 * @mainpage Server Documentation
 *
 * @section intro_sec Introduction
 * This server is built using Express.js and provides endpoints to execute, stop,
 * and monitor system processes. It also manages configuration and monitoring files.
 */

/**
 * @defgroup Config Server Configuration
 * @{
 */

/**
 * @var {Object} express
 * @brief Express instance for server configuration.
 */
const express = require('express');

/**
 * @var {Object} exec
 * @brief Child process execution module.
 */
const { exec } = require('child_process');

/**
 * @var {Function} cors
 * @brief CORS middleware for cross-origin requests.
 */
const cors = require('cors');

/**
 * @var {Object} fs
 * @brief File system module for JSON file handling.
 */
const fs = require('fs');

/**
 * @var {Object} path
 * @brief Path module for managing file paths.
 */
const path = require('path');

/** @} */ // end of Config

/**
 * @defgroup ServerVars Server Variables
 * @{
 */

/**
 * @var {Object} app
 * @brief Express application instance.
 */
const app = express();

/**
 * @var {number} PORT
 * @brief Server listening port.
 */
const PORT = 3030;

/**
 * @var {Object} PATHS
 * @brief Paths for process execution and configuration files.
 */
const PATHS = {
    processJson: './Json/process.json',
    systemMonitoring: './Json/system_monitoring.json',
    generator: path.join(__dirname, 'Generator.exe'),
    configurator: path.join(__dirname, '..', '..', 'EcoflocConfigurator.exe'),
    pidRecup: path.join(__dirname, 'PIDRecup.exe')
};

/**
 * @var {Object|null} currentProcess
 * @brief Tracks the current running process.
 */
let currentProcess = null;

/**
 * @var {boolean} processRunning
 * @brief Indicates if a process is currently running.
 */
let processRunning = false;

/**
 * @var {boolean} configuratorRunning
 * @brief Indicates if the configurator is running.
 */
let configuratorRunning = false;

/** @} */ // end of ServerVars

/**
 * @defgroup FileUtils File Utilities
 * @{
 */

/**
 * @var {Object} fileUtils
 * @brief Utility functions for file operations.
 */
const fileUtils = {
    
    /**
     * @brief Reads a JSON file and returns its content.
     * @function readJSON
     * @param {string} filePath Path to the JSON file.
     * @return {Object|null} Parsed JSON data or null in case of error.
     */
    readJSON: (filePath) => {
        try {
            return fs.existsSync(filePath) ? JSON.parse(fs.readFileSync(filePath, 'utf8')) : null;
        } catch (error) {
            console.error(`Error reading JSON file ${filePath}:`, error);
            return null;
        }
    },

    /**
     * @brief Writes data to a JSON file.
     * @function writeJSON
     * @param {string} filePath Path to the JSON file.
     * @param {Object} data Data to write.
     * @return {boolean} True if successful, otherwise false.
     */
    writeJSON: (filePath, data) => {
        try {
            fs.writeFileSync(filePath, JSON.stringify(data, null, 4), 'utf8');
            return true;
        } catch (error) {
            console.error(`Error writing JSON file ${filePath}:`, error);
            return false;
        }
    },

    /**
     * @brief Resets the system monitoring JSON file if empty.
     * @function resetMonitoringFile
     */
    resetMonitoringFile: () => {
        // Only reset if the file doesn't exist or is empty
        if (!fs.existsSync(PATHS.systemMonitoring) || 
            fs.statSync(PATHS.systemMonitoring).size === 0) {
            fileUtils.writeJSON(PATHS.systemMonitoring, { apps: [] });
        }
    },

    /**
     * @brief Exports monitoring data to a CSV format.
     * @function exportToCSV
     * @param {Object} graphData The monitoring data structure.
     * @return {string} A string containing CSV formatted data.
     */
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

/** @} */ // end of FileUtils

/**
 * @defgroup ProcessUtils Process Management
 * @{
 */

/**
 * @var {Object} processUtils
 * @brief Utility functions for process management.
 */
const processUtils = {

    /**
     * @brief Cleans up running processes.
     * @function cleanup
     */
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

    /**
     * @brief Terminates a process by its PID.
     * @function killProcess
     * @param {number} pid Process ID to terminate.
     * @param {Function} callback Callback function after termination.
     */
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

/** @} */ // end of ProcessUtils

/**
 * @defgroup API Endpoints
 * @{
 */

/**
 * @brief Executes a new process if none is currently running.
 * @function execute
 */
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

/**
 * @brief Stops the currently running process.
 * @function stop
 */
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

/**
 * @brief Launches the configurator application.
 * @function configurator
 */
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

/**
 * @brief Updates process list state in the monitoring file.
 * @function updateProcessListState
 */
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

/**
 * @brief Changes the state of a specific PID.
 * @function changePidState
 */
app.post('/changePidState', (req, res) => {
    const { processName, pid, state } = req.body;
    const data = fileUtils.readJSON(PATHS.processJson);
    
    if (data) {
        data.forEach(process => {
            if (process.name === processName) {
                process.pids.forEach(pidInfo => {
                    if (pidInfo.numeroPid === pid) {
                        pidInfo.checked = state;
                    }
                });
            }
        });
        fileUtils.writeJSON(PATHS.processJson, data);
    }
    
    res.json({ success: true, message: 'PID state updated' });
});

/**
 * @brief Changes the state of multiple PIDs.
 * @function changeListePidState
 */
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

/**
 * @brief Provides real-time updates via Server-Sent Events.
 * @function events
 */
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

/**
 * @brief Exports monitoring data to CSV format.
 * @function exportCSV
 */
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

/**
 * @brief Updates the process monitoring file.
 * @function update
 */
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

/** @} */ // end of API

/**
 * @brief Starts the Express.js server.
 * @function startServer
 */
app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
    fileUtils.resetMonitoringFile();
});
