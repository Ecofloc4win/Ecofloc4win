class DynamicGraph {
    constructor(nomGraphique) {
        // Configuration du layout pour le graphique
        this.layout = {
            xaxis: {
                gridcolor: 'rgba(255,255,255,0.2)',
                tickfont: { color: 'white' },
                range: [-30, 0]
            },
            yaxis: {
                gridcolor: 'rgba(255,255,255,0.2)',
                tickfont: { color: 'white' }
            },
            margin: { l: 30, r: 30, t: 10, b: 20 },
            paper_bgcolor: 'rgba(0,0,0,0)',
            plot_bgcolor: 'rgba(0,0,0,0)',
            dragmode: false,
            showlegend: false
        };

        //this.key = 0;
        this.data = {}; // Stockage des séries par PID
        this.traceIndices = {}; // Lien entre PID et index des traces
        this.nomGraphique = nomGraphique;

        // Initialisation du graphique
        Plotly.newPlot(this.nomGraphique, [], this.layout, { responsive: true, displayModeBar: false });
    }

    updatePlot(PID, value, time, color) {
        
        // Vérifie si le PID existe déjà
        if (!this.data[PID]) {
            if (PID !== "TOTAL") {
                this.data[PID] = {
                    x: [],
                    y: [],
                    line: { color:color },
                    fill: 'none',
                    name: `PID ${PID}`,
                };
            }
            else{
                this.data["TOTAL"] = {
                    x: [],
                    y: [],
                    line: { color: "#10b981" },
                    fill: 'tozeroy',
                };
            }
            
            // Ajoute une nouvelle trace pour ce PID
            Plotly.addTraces(this.nomGraphique, this.data[PID]);
            this.traceIndices[PID] = Object.keys(this.traceIndices).length; // Associe un index à ce PID
        }

        // Ajoute la valeur et le timestamp à la série correspondante
        this.data[PID].x.push(time);
        this.data[PID].y.push(value);

        // Vérifie si un index existe pour ce PID
        const index = this.traceIndices[PID];
        if (index === undefined) {
            console.error(`Erreur : PID ${PID} non trouvé dans les indices des traces`);
            return;
        }

        // Met à jour la trace correspondante avec les deux tableaux x et y
        Plotly.update(
            this.nomGraphique, 
            { 
                x: [this.data[PID].x],
                y: [this.data[PID].y] 
            }, 
            {
                xaxis: {
                    gridcolor: 'rgba(255,255,255,0.2)', 
                    tickfont: { color: 'white' }, 
                    range: [time-30, time]
                }
            }, 
            [index]
        );
    }

    refreshGraph() {
        const traces = Object.values(this.data);
        Plotly.newPlot(this.nomGraphique, traces, this.layout, { responsive: true, displayModeBar: false });
    }

    getRandomColor() {
        const r = Math.floor(Math.random() * 200)+55;
        const g = Math.floor(Math.random() * 200)+55;
        const b = Math.floor(Math.random() * 200)+55;
        return `rgb(${r}, ${g}, ${b})`;
    }

}

export default DynamicGraph;
