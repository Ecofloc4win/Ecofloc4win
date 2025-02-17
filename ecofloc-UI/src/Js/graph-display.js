import DynamicGraph from './ClassDynamicGraph.js';

// Constants
const COMPONENT_TOTALS = {
    CPU: 0,
    GPU: 0,
    NIC: 0,
    SD: 0
};

const LAST_TIMESTAMPS = new Map();
let LAST_GLOBAL_TOTAL = 0;

// State variables
let START_TIME = null;
let IS_MONITORING = false;
let HAS_DATA = false;
let NUM_SECONDS = 0;
let TOTAL_WATTS = 0;
let PREVIOUS_TIMESTAMP = 0;

// Read the JSON files and update plots
const readFile = () => {
    if (!IS_MONITORING) return;  // Arrêter la lecture si on n'est pas en monitoring
    
    if (!START_TIME) {
        START_TIME = Date.now() / 1000;
    }
    
    NUM_SECONDS = Math.floor(Date.now() / 1000 - START_TIME);
    
    // Reset totals before each read
    Object.keys(COMPONENT_TOTALS).forEach(key => {
        COMPONENT_TOTALS[key] = 0;
    });
    TOTAL_WATTS = 0;

    fetch('http://localhost:3030/monitored-pids')
        .then(response => response.ok ? response.json() : null)
        .then(async pids => {
            if (!pids || !IS_MONITORING) return;  // Double vérification
            
            let globalTotal = 0;
            let hasNewData = false;
            
            for (const pid of pids) {
                try {
                    const response = await fetch(`http://localhost:3030/metrics/${pid}`);
                    if (!response.ok) continue;
                    
                    const data = await response.json();
                    const lastTimestamp = LAST_TIMESTAMPS.get(pid) || 0;
                    
                    if (data && data.time !== lastTimestamp) {
                        LAST_TIMESTAMPS.set(pid, data.time);
                        const pidTotal = updatePlots(data, pid);
                        globalTotal += pidTotal;
                        hasNewData = true;
                    }
                } catch (error) {
                    console.error(`Error reading file for PID ${pid}:`, error);
                }
            }

            if (hasNewData) {  // Mise à jour uniquement si nouvelles données
                graphTOTAL.updateSinglePlot(globalTotal, NUM_SECONDS);
                LAST_GLOBAL_TOTAL = globalTotal;
            }
        })
        .catch(error => console.error("Error reading PIDs:", error));
};

// Update individual plots based on the JSON data
const updatePlots = (data, pid) => {
    if (!data || !data.apps || !data.apps[0]) return 0;
    
    const app = data.apps[0];
    let pidTotal = 0;

    if (app.power_w_CPU !== undefined) {
        graphCPU.updatePlot(pid, app.power_w_CPU, NUM_SECONDS, app.color);
        HAS_DATA = true;
        pidTotal += app.power_w_CPU;
    }
    
    if (app.power_w_GPU !== undefined) {
        graphGPU.updatePlot(pid, app.power_w_GPU, NUM_SECONDS, app.color);
        HAS_DATA = true;
        pidTotal += app.power_w_GPU;
    }
    
    if (app.power_w_NIC !== undefined) {
        graphNIC.updatePlot(pid, app.power_w_NIC, NUM_SECONDS, app.color);
        HAS_DATA = true;
        pidTotal += app.power_w_NIC;
    }
    
    if (app.power_w_SD !== undefined) {
        graphSD.updatePlot(pid, app.power_w_SD, NUM_SECONDS, app.color);
        HAS_DATA = true;
        pidTotal += app.power_w_SD;
    }

    return pidTotal;
};

// Graph instances
export let graphCPU = new DynamicGraph("graphCPU");
export let graphGPU = new DynamicGraph("graphGPU");
export let graphNIC = new DynamicGraph("graphNIC");
export let graphSD = new DynamicGraph("graphSD");
let graphTOTAL = new DynamicGraph("graphTOTAL");

// Control functions
function updateControlsState() {
    const START_BUTTON = document.querySelector("#start-button");
    const STOP_BUTTON = document.querySelector("#stop-button");
    const CLEAR_BUTTON = document.querySelector("#clear-button");
    const CHECKBOXES = document.querySelectorAll("input[type='checkbox']");
    const EXPORT_BUTTON = document.querySelector("#exportCSV");
    const SEARCH_INPUT = document.querySelector("#SearchBar");
    const INTERVAL_INPUT = document.querySelector("#intervalTime");

    START_BUTTON.disabled = IS_MONITORING;
    STOP_BUTTON.disabled = !IS_MONITORING;
    CLEAR_BUTTON.disabled = IS_MONITORING;
    EXPORT_BUTTON.disabled = IS_MONITORING || !HAS_DATA;
    
    CHECKBOXES.forEach(checkbox => {
        checkbox.disabled = IS_MONITORING;
    });

    SEARCH_INPUT.disabled = IS_MONITORING;
    SEARCH_INPUT.style.backgroundColor = IS_MONITORING ? '#e5e7eb' : 'white';
    INTERVAL_INPUT.disabled = IS_MONITORING;
    INTERVAL_INPUT.style.backgroundColor = IS_MONITORING ? '#e5e7eb' : 'white';
}

// Event listeners
document.querySelector("#start-button").addEventListener("click", async () => {
    if (IS_MONITORING) return;
    
    const INTERVAL = document.querySelector("#intervalTime").valueAsNumber;
    
    try {
        const response = await fetch('http://localhost:3030/execute', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ interval: INTERVAL })
        });

        const data = await response.json();
        
        if (data.success) {
            IS_MONITORING = true;
            if (!HAS_DATA) {
                START_TIME = Date.now() / 1000;
                NUM_SECONDS = 0;
                TOTAL_WATTS = 0;
                LAST_GLOBAL_TOTAL = 0;
            } else {
                START_TIME = Date.now() / 1000 - NUM_SECONDS;
            }
            
            window.monitoringInterval = setInterval(readFile, INTERVAL);
            updateControlsState();
        } else {
            console.error('Failed to start monitoring:', data.message);
        }
    } catch (error) {
        console.error('Error starting monitoring:', error);
    }
});

document.querySelector("#stop-button").addEventListener("click", async () => {
    if (!IS_MONITORING) return;
    
    try {
        await fetch('http://localhost:3030/stop', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            }
        });
        
        if (window.monitoringInterval) {
            clearInterval(window.monitoringInterval);
        }
        
        IS_MONITORING = false;
        updateControlsState();
    } catch (error) {
        console.error('Error stopping monitoring:', error);
    }
});

document.querySelector("#clear-button").addEventListener("click", () => {
    if (IS_MONITORING) return;
    
    graphCPU.clearData();
    graphGPU.clearData();
    graphNIC.clearData();
    graphSD.clearData();
    graphTOTAL.clearData();
    
    HAS_DATA = false;
    NUM_SECONDS = 0;
    TOTAL_WATTS = 0;
    START_TIME = null;
    PREVIOUS_TIMESTAMP = 0;
    LAST_GLOBAL_TOTAL = 0;
    LAST_TIMESTAMPS.clear();
    
    updateControlsState();
});

updateControlsState(); 