import {graphCPU,graphGPU,graphNIC,graphSD,actualDateInSecond} from './affichage-graphique.js'; 

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

const getStartMonitoringTime = () => {
    let monitoringDuration = Math.max(
                            ...Object.values(graphCPU.data)
                            .flatMap(obj => obj.x)
                        );
    return actualDateInSecond - monitoringDuration;
}

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