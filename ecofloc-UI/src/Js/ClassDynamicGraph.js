class DynamicGraph {
    constructor(nomGraphique) {
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

        this.data = {}; // PID series storage
        this.traceIndices = {}; // Link between PID and trace index
        this.nomGraphique = nomGraphique;

        // Graph initialization
        Plotly.newPlot(this.nomGraphique, [], this.layout, { responsive: true, displayModeBar: false });
    }

    updatePlot(PID, value, time, color) {
        
        // Checks if the PID already exists otherwise create it
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
            
            // Adds a new trace for this PID
            Plotly.addTraces(this.nomGraphique, this.data[PID]);
            this.traceIndices[PID] = Object.keys(this.traceIndices).length; // Associate an index with this PID
        }

        // Adds the value and timestamp to the corresponding series
        this.data[PID].x.push(time);
        this.data[PID].y.push(value);

        // Checks if an index exists for this PID
        const index = this.traceIndices[PID];
        if (index === undefined) {
            console.error(`Error: PID ${PID} not found in trace indices`);
            return;
        }

        // Updates the corresponding trace with the two tables x and y
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

    getRandomColor() {
        const r = Math.floor(Math.random() * 200)+55;
        const g = Math.floor(Math.random() * 200)+55;
        const b = Math.floor(Math.random() * 200)+55;
        return `rgb(${r}, ${g}, ${b})`;
    }

}

export default DynamicGraph;
