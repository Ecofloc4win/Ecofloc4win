import {graphCPU,graphGPU,graphNIC,graphSD} from './affichage-graphique.js'; 

document.getElementById('exportCSV').addEventListener('click', async () => {
    try {
        if (!graphCPU || !graphGPU || !graphNIC || !graphSD) {
            throw new Error('graphs does not exists');
        }

        monitoringStartTime = getStartMonitoringTime();

        // Collect data from all graphs with x and y values
        const graphData = recupGraphData(monitoringStartTime);

        blob = getCsv(graphData);

        launchDownload(blob);

    } catch (error) {
        console.error('Error exporting data:', error);
        alert(`Erreur d'export : ${error.message}`);
    }
});


const recupGraphData = (startTime) => {
    return {
        CPU: Object.fromEntries(
            Object.entries(graphCPU.data).map(([pid, data]) => [pid,
            {
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

const launchDownload = (blob) => {
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    const now = new Date();
    const date = now.toISOString().split('T')[0];
    const time = `${now.getHours().toString().padStart(2, '0')}-${now.getMinutes().toString().padStart(2, '0')}`;
    
    a.download = `monitoring_data_${date}_${time}.csv`;
    
    document.body.appendChild(a);
    a.click();
    window.URL.revokeObjectURL(url);
    document.body.removeChild(a);
}


const getStartMonitoringTime = () => {
    let actualDateInSecond = new Date().getTime() / 1000;
    let monitoringDuration = Math.max(
                            ...Object.values(graphCPU.data)
                            .flatMap(obj => obj.x)
                        );
    return actualDateInSecond - monitoringDuration;
}

async function getCsv(graph){
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
    return await response.blob();
}