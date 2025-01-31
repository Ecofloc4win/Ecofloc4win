import DynamicGraph from './ClassDynamicGraph'; 
export let actualDateInSecond = 0;
let precedentTimeStamp = 0;
let totalW = 0;
let numSec = 0;

// Read the JSON file and update plots
const readFile = () => {
    fetch('../Json/system_monitoring.json')
        .then(response => response.ok ? response.json() : null)
        .then(data => {
            if (data && data.time !== precedentTimeStamp) {
                precedentTimeStamp = data.time;
                updatePlots(data);
                numSec++;
            }
        })
        .catch(error => console.error("Error reading file:", error));
};

// Update individual plots based on the JSON data
const updatePlots = (data) => {
    
    totalW = 0;
    data.apps.forEach(app => {

        graphCPU.updatePlot(app["pid"], app["power_w_CPU"], numSec, app["color"]);
        totalW +=  app["power_w_CPU"];

        graphGPU.updatePlot(app["pid"], app["power_w_GPU"], numSec, app["color"]);
        totalW +=  app["power_w_GPU"];

        graphSD.updatePlot(app["pid"], app["power_w_SD"], numSec, app["color"]);
        totalW +=  app["power_w_SD"];

        graphNIC.updatePlot(app["pid"], app["power_w_NIC"], numSec, app["color"]);
        totalW +=  app["power_w_NIC"];

    });
    graphTOTAL.updatePlot("TOTAL",totalW, numSec)
};

export let graphCPU = new DynamicGraph("graphCPU", "rgb(248, 113, 113)");
export let graphGPU = new DynamicGraph("graphGPU", "rgb(74, 222, 128)");
export let graphNIC = new DynamicGraph("graphNIC", "rgb(96, 165, 250)");
export let graphSD = new DynamicGraph("graphSD", "rgb(129, 140, 248)");
let graphTOTAL = new DynamicGraph("graphTOTAL", "rgb(192, 132, 252)");


startButton.addEventListener("click", () => {
    setInterval(readFile, 500);
    document.querySelector("#exportCSV").disabled = true;
});

stopButton.addEventListener("click", () => {
    clearInterval(readFile);
    document.querySelector("#exportCSV").disabled = false;
    actualDateInSecond = new Date().getTime() / 1000;
});
