class DynamicGraph 
{
    constructor(graphName) 
    {
        this.LAYOUT = 
        {
            xaxis: 
            {
                gridcolor: 'rgba(255,255,255,0.2)',
                tickfont: { color: 'white' },
                range: [-30, 0]
            },
            yaxis: 
            {
                gridcolor: 'rgba(255,255,255,0.2)',
                tickfont: { color: 'white' }
            },
            margin: { l: 30, r: 30, t: 10, b: 20 },
            paper_bgcolor: 'rgba(0,0,0,0)',
            plot_bgcolor: 'rgba(0,0,0,0)',
            dragmode: false,
            showlegend: false
        };

        this.DATA = {}; // PID series storage
        this.TRACE_INDICES = {}; // Link between PID and trace index
        this.GRAPH_NAME = graphName;

        // Graph initialization
        Plotly.newPlot(this.GRAPH_NAME, [], this.LAYOUT, { responsive: true, displayModeBar: false });
    }

    updatePlot(PID, value, time, color) 
    {
        if (!this.DATA[PID]) 
        {
            this.DATA[PID] = {
                x: [],
                y: [],
                line: { color: color },
                fill: 'none',
                name: `PID ${PID}`
            };
            
            Plotly.addTraces(this.GRAPH_NAME, this.DATA[PID]);
            this.TRACE_INDICES[PID] = Object.keys(this.TRACE_INDICES).length;
        }

        this.DATA[PID].x.push(time);
        this.DATA[PID].y.push(value);

        const index = this.TRACE_INDICES[PID];
        if (index === undefined) {
            console.error(`Error: PID ${PID} not found in trace indices`);
            return;
        }
        
        Plotly.update(
            this.GRAPH_NAME, 
            { 
                x: [this.DATA[PID].x],
                y: [this.DATA[PID].y] 
            }, 
            {
                xaxis: {
                    range: [time-30, time]
                }
            }, 
            [index]
        );
    }

    updateSinglePlot(value, time) {
        if (!this.DATA.total) {
            this.DATA.total = {
                x: [],
                y: [],
                line: { color: "#10b981" },
                fill: 'tozeroy',
                name: 'Total Consumption'
            };
            
            Plotly.newPlot(
                this.GRAPH_NAME, 
                [this.DATA.total], 
                this.LAYOUT,
                { responsive: true, displayModeBar: false }
            );
        }

        this.DATA.total.x.push(time);
        this.DATA.total.y.push(value);

        Plotly.update(
            this.GRAPH_NAME,
            {
                x: [this.DATA.total.x],
                y: [this.DATA.total.y]
            },
            {
                xaxis: {
                    range: [time-30, time]
                }
            }
        );
    }

    getRandomColor() 
    {
        const RED = Math.floor(Math.random() * 200) + 55;
        const GREEN = Math.floor(Math.random() * 200) + 55;
        const BLUE = Math.floor(Math.random() * 200) + 55;
        return `rgb(${RED}, ${GREEN}, ${BLUE})`;
    }

    clearData() {
        this.DATA = {};
        this.TRACE_INDICES = {};
        Plotly.newPlot(this.GRAPH_NAME, [], this.LAYOUT, { responsive: true, displayModeBar: false });
    }

    getLastData(PID) {
        if (this.DATA[PID] && this.DATA[PID].y.length > 0) {
            return {
                y: this.DATA[PID].y[this.DATA[PID].y.length - 1],
                time: this.DATA[PID].x[this.DATA[PID].x.length - 1]
            };
        }
        return null;
    }
}

export default DynamicGraph;
