import DynamicGraph from './ClassDynamicGraph'; 
export let actualDateInSecond = 0;
let precedentTimeStamp = 0;
let totalW = 0;
let numSec = 0;
let startTime = null;
let isMonitoring = false;
let hasData = false;

// Au début du fichier, ajoutons un objet pour stocker les totaux par composant
const componentTotals = {
    CPU: 0,
    GPU: 0,
    NIC: 0,
    SD: 0
};

// Au début du fichier, ajoutons un Map pour stocker les derniers timestamps par PID
const lastTimestamps = new Map();

// Ajouter une variable pour stocker le dernier total
let lastGlobalTotal = 0;

// Read the JSON files and update plots
const readFile = () => {
    if (!startTime) {
        startTime = Date.now() / 1000;
    }
    
    numSec = Math.floor(Date.now() / 1000 - startTime);
    
    // Réinitialiser les totaux avant chaque lecture
    Object.keys(componentTotals).forEach(key => {
        componentTotals[key] = 0;
    });
    totalW = 0;

    fetch('http://localhost:3030/monitored-pids')
        .then(response => response.ok ? response.json() : null)
        .then(async pids => {
            if (!pids) return;
            
            let globalTotal = 0;
            let hasNewData = false;
            
            for (const pid of pids) {
                try {
                    const response = await fetch(`http://localhost:3030/metrics/${pid}`);
                    if (!response.ok) continue;
                    
                    const data = await response.json();
                    const lastTimestamp = lastTimestamps.get(pid) || 0;
                    
                    if (data && data.time !== lastTimestamp) {
                        lastTimestamps.set(pid, data.time);
                        const pidTotal = updatePlots(data, pid);
                        globalTotal += pidTotal;
                        hasNewData = true;
                    } else {
                        // Utiliser la dernière valeur connue
                        const lastData = graphTOTAL.getLastData(pid);
                        if (lastData) {
                            globalTotal += lastData.y;
                        }
                    }
                } catch (error) {
                    console.error(`Error reading file for PID ${pid}:`, error);
                }
            }

            // Mettre à jour le total seulement si nécessaire
            if (hasNewData || Math.abs(globalTotal - lastGlobalTotal) > 0.01) {
                graphTOTAL.updateSinglePlot(globalTotal, numSec);
                lastGlobalTotal = globalTotal;
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
        graphCPU.updatePlot(pid, app.power_w_CPU, numSec, app.color);
        hasData = true;
        pidTotal += app.power_w_CPU;
    }
    
    if (app.power_w_GPU !== undefined) {
        graphGPU.updatePlot(pid, app.power_w_GPU, numSec, app.color);
        hasData = true;
        pidTotal += app.power_w_GPU;
    }
    
    if (app.power_w_SD !== undefined) {
        graphSD.updatePlot(pid, app.power_w_SD, numSec, app.color);
        hasData = true;
        pidTotal += app.power_w_SD;
    }
    
    if (app.power_w_NIC !== undefined) {
        graphNIC.updatePlot(pid, app.power_w_NIC, numSec, app.color);
        hasData = true;
        pidTotal += app.power_w_NIC;
    }

    return pidTotal;
};

export let graphCPU = new DynamicGraph("graphCPU", "rgb(248, 113, 113)");
export let graphGPU = new DynamicGraph("graphGPU", "rgb(74, 222, 128)");
export let graphNIC = new DynamicGraph("graphNIC", "rgb(96, 165, 250)");
export let graphSD = new DynamicGraph("graphSD", "rgb(129, 140, 248)");
let graphTOTAL = new DynamicGraph("graphTOTAL", "rgb(192, 132, 252)");

// Fonction pour mettre à jour l'état des contrôles
function updateControlsState() {
    const startButton = document.querySelector("#start-button");
    const stopButton = document.querySelector("#stop-button");
    const clearButton = document.querySelector("#clear-button");
    const checkboxes = document.querySelectorAll(".custom-tbody input[type='checkbox']");
    const exportButton = document.querySelector("#exportCSV");

    startButton.disabled = isMonitoring;
    stopButton.disabled = !isMonitoring;
    clearButton.disabled = isMonitoring;
    exportButton.disabled = isMonitoring || !hasData;
    
    // Désactiver les checkboxes seulement pendant le monitoring
    checkboxes.forEach(checkbox => {
        checkbox.disabled = isMonitoring || hasData;
    });
}

// Fonction pour effacer les données des graphes
function clearGraphData() {
    if (isMonitoring) {
        console.warn('Cannot clear data while monitoring is active');
        return;
    }
    
    graphCPU.clearData();
    graphGPU.clearData();
    graphNIC.clearData();
    graphSD.clearData();
    graphTOTAL.clearData();
    
    Object.keys(componentTotals).forEach(key => {
        componentTotals[key] = 0;
    });
    
    lastTimestamps.clear(); // Réinitialiser les timestamps
    
    hasData = false;
    numSec = 0;
    totalW = 0;
    startTime = null;
    precedentTimeStamp = 0;
    updateControlsState();
}

// Modifier les event listeners existants
startButton.addEventListener("click", async () => {
    if (isMonitoring) return;
    
    const interval = document.querySelector("#intervalTime").valueAsNumber;
    
    try {
        const response = await fetch('http://localhost:3030/execute', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ interval })
        });

        const data = await response.json();
        
        if (data.success) {
            isMonitoring = true;
            if (!hasData) {
                startTime = Date.now() / 1000;
                numSec = 0;
                totalW = 0;
                lastGlobalTotal = 0;
            } else {
                startTime = Date.now() / 1000 - numSec;
            }
            
            // Ajuster l'intervalle de lecture en fonction de l'intervalle des données
            const readInterval = Math.max(interval, 500); // Minimum 500ms
            window.monitoringInterval = setInterval(readFile, readInterval);
            
            updateControlsState();
        } else {
            console.error('Failed to start monitoring:', data.message);
        }
    } catch (error) {
        console.error('Error starting monitoring:', error);
    }
});

stopButton.addEventListener("click", async () => {
    if (!isMonitoring) return; // Éviter les doubles arrêts
    
    if (window.monitoringInterval) {
        clearInterval(window.monitoringInterval);
    }

    try {
        await fetch('http://localhost:3030/stop', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            }
        });
        
        isMonitoring = false;
        actualDateInSecond = new Date().getTime() / 1000;
        updateControlsState();
    } catch (error) {
        console.error('Error stopping monitoring:', error);
    }
});

// Ajouter le nouveau event listener pour le bouton clear
document.querySelector("#clear-button").addEventListener("click", clearGraphData);

// Appeler updateControlsState au chargement
updateControlsState();
