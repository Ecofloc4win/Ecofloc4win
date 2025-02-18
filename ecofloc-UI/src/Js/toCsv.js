/**
 * @file toCsv.js
 * @brief This file contains JavaScript code for exporting data from graphs (CPU, GPU, NIC, SD) into a CSV file format.
 *        It listens for a click event on the "Export CSV" button and collects graph data, then sends it to the server to generate a downloadable CSV file.
 * @author Ecofloc's Team
 * @lastupdate 2025-02-18
 */

import {graphCPU,graphGPU,graphNIC,graphSD,actualDateInSecond} from './graphicDisplay.js'; 

// Event listener for the "Export CSV" button to trigger the export process
document.getElementById('exportCSV').addEventListener('click', async () => {
    try {
        if (!graphCPU || !graphGPU || !graphNIC || !graphSD) {
            throw new Error('graphs does not exists');
        }

        let monitoringStartTime = getStartMonitoringTime();

        // Collect data from all graphs with x and y values
        const graphData = recupGraphData(monitoringStartTime);

        let blob = await getCsv(graphData);

        launchDownload(blob);

    } catch (error) {
        console.error('Error exporting data:', error);
        alert(`Erreur d'export : ${error.message}`);
    }
});

/**
 * @function recupGraphData
 * @brief Collects the graph data for CPU, GPU, NIC, and SD and adjusts the timestamp based on the monitoring start time.
 * @param {number} startTime - The start time of the monitoring session.
 * @return {Object} The collected graph data with adjusted timestamps.
 */
const recupGraphData = (startTime) => {
    return {
        CPU: Object.fromEntries(
            Object.entries(graphCPU.data).map(([pid, data]) => [pid,
            {
                // Ajuste chaque élément de la liste x pour inclure la date de début
                x: data.x.map(timestamp => timestamp + startTime) || [],
                y: data.y || [],
            },])),
        GPU: Object.fromEntries(
            Object.entries(graphGPU.data).map(([pid, data]) => [pid,
            {
                x: data.x.map(timestamp => timestamp + startTime) || [],
                y: data.y || [],
            },])),
        NIC: Object.fromEntries(
            Object.entries(graphNIC.data).map(([pid, data]) => [pid,
            {
                x: data.x.map(timestamp => timestamp + startTime) || [],
                y: data.y || [],
            },])),
        SD: Object.fromEntries(
            Object.entries(graphSD.data).map(([pid, data]) => [pid,
            {
                x: data.x.map(timestamp => timestamp + startTime) || [],
                y: data.y || [],
            },]))
        };
} 

/**
 * @function launchDownload
 * @brief Initiates the download of the generated CSV file.
 * @param {Blob} file - The Blob object containing the CSV data.
 * 
 * This function creates a temporary download link and triggers the file download.
 */
const launchDownload = (file) => {
    try {
        
        if (!(file instanceof Blob)) {
            throw new Error('Invalid file type - expected Blob');
        }
        
        let url = window.URL.createObjectURL(file);
        let a = document.createElement('a');
        a.href = url;

        let now = new Date();
        let date = now.toISOString().split('T')[0];
        let time = `${now.getHours().toString().padStart(2, '0')}-${now.getMinutes().toString().padStart(2, '0')}`;
        
        a.download = `monitoring_data_${date}_${time}.csv`;
        
        document.body.appendChild(a);
        a.click();
        window.URL.revokeObjectURL(url);
        document.body.removeChild(a);
    } catch (error) {
        console.error('Error in launchDownload:', error);
        throw error;
    }
}

/**
 * @function launchDownload
 * @brief Initiates the download of the generated CSV file.
 * @param {Blob} file - The Blob object containing the CSV data.
 * 
 * This function creates a temporary download link and triggers the file download.
 */
const getStartMonitoringTime = () => {
    let monitoringDuration = Math.max(
                            ...Object.values(graphCPU.data)
                            .flatMap(obj => obj.x)
                        );
    return actualDateInSecond - monitoringDuration;
}

/**
 * @function getCsv
 * @brief Sends the collected graph data to the server to generate a CSV file.
 * @param {Object} graph - The collected graph data.
 * @return {Blob} The generated CSV file as a Blob object.
 * 
 * This function sends the graph data to a server endpoint for CSV export and retrieves the CSV file as a Blob.
 */
async function getCsv(graph){
    try {

        const response = await fetch('http://localhost:3030/export-csv', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(graph)
        });

        if (!response.ok) {
            const errorText = await response.text();
            console.error('Server error:', errorText);
            throw new Error(`Export failed: ${errorText}`);
        }

        const blob = await response.blob();

        if (!blob || blob.size === 0) {
            throw new Error('Received empty blob from server');
        }

        return blob;
    } catch (error) {
        console.error('Error in getCsv:', error);
        throw error;
    }
}