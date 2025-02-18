/**
 * @file graphicDisplay.js
 * @brief Manages the graphical display of system monitoring data.
 * @author Ecofloc's Team
 * @lastupdate 2025-02-18
 */

import DynamicGraph from './ClassDynamicGraph';

/**
 * @var {number} actualDateInSecond
 * @brief Stores the current time in seconds.
 */
export let actualDateInSecond = 0;

/**
 * @var {number} previousTimeStamp
 * @brief Stores the previous timestamp to avoid redundant updates.
 */
let previousTimeStamp = 0;

/**
 * @var {number} totalW
 * @brief Stores the total power consumption.
 */
let totalW = 0;

/**
 * @var {number} numSec
 * @brief Stores the elapsed number of seconds.
 */
let numSec = 0;

/**
 * @brief Reads a JSON file and updates the graphs.
 * @function readFile
 */ 
const readFile = () => {
    fetch('../Json/system_monitoring.json')
        .then(response => response.ok ? response.json() : null)
        .then(data => {
            if (data && data.time !== previousTimeStamp)
            {
                previousTimeStamp = data.time;
                updatePlots(data);
                numSec++;
            }
        })
        .catch(error => console.error("Error reading file:", error));
};

/**
 * @brief Updates individual graphs based on the JSON data.
 * @function updatePlots
 * @param {Object} data - Data from the JSON file.
 */
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

/**
 * @var {DynamicGraph} graphCPU
 * @brief Graph displaying CPU power consumption.
 */
export let graphCPU = new DynamicGraph("graphCPU", "rgb(248, 113, 113)");

/**
 * @var {DynamicGraph} graphGPU
 * @brief Graph displaying GPU power consumption.
 */
export let graphGPU = new DynamicGraph("graphGPU", "rgb(74, 222, 128)");

/**
 * @var {DynamicGraph} graphNIC
 * @brief Graph displaying network (NIC) power consumption.
 */
export let graphNIC = new DynamicGraph("graphNIC", "rgb(96, 165, 250)");

/**
 * @var {DynamicGraph} graphSD
 * @brief Graph displaying storage (SD) power consumption.
 */
export let graphSD = new DynamicGraph("graphSD", "rgb(129, 140, 248)");

/**
 * @var {DynamicGraph} graphTOTAL
 * @brief Graph displaying total power consumption.
 */
let graphTOTAL = new DynamicGraph("graphTOTAL", "rgb(192, 132, 252)");

/**
 * @brief Starts reading the file and disables CSV export.
 */
startButton.addEventListener("click", () => {
    setInterval(readFile, 500);
    document.querySelector("#exportCSV").disabled = true;
    document.querySelector("#exportCSV").classList.add("disabled");
});

/**
 * @brief Stops reading the file and enables CSV export.
 */
stopButton.addEventListener("click", () => {
    clearInterval(readFile);
    document.querySelector("#exportCSV").disabled = false;
    document.querySelector("#exportCSV").classList.remove("disabled");
    actualDateInSecond = new Date().getTime() / 1000;
});
