import { describe, it, expect, beforeEach, vi } from 'vitest';
import DynamicGraph from '../Js/ClassDynamicGraph';

global.Plotly = {
    newPlot: vi.fn(),
    addTraces: vi.fn(),
    update: vi.fn()
};

describe('DynamicGraph', () => {
    let graph;

    beforeEach(() => {
        graph = new DynamicGraph('graphDiv'); 
        vi.clearAllMocks();
    });

    it('add a trace for a new PID', () => {
        // GIVEN
        const PID = '123';
        const initialTraces = Object.keys(graph.traceIndices).length;

        // WHEN
        graph.updatePlot(PID, 10, 1, 'blue');

        // THEN
        expect(Object.keys(graph.traceIndices).length).toBe(initialTraces + 1);
        expect(graph.data[PID].y[0]).toBe(10);
        expect(graph.traceIndices[PID]).toBe(0);
        expect(Plotly.addTraces).toHaveBeenCalledTimes(1);
    });

    it('trace added for PID TOTAL', () => {
        // GIVEN
        const PID = 'TOTAL';
        const value = 50;
        
        // WHEN
        graph.updatePlot(PID, value, 1, 'green');

        // THEN
        expect(graph.data[PID].y[0]).toBe(value);
        expect(Object.keys(graph.traceIndices).length).toBe(1);
        expect(graph.traceIndices[PID]).toBe(0);
        expect(Plotly.addTraces).toHaveBeenCalledTimes(1);
    });

    it('updating an existing trace', () => {
        //GIVEN
        const PID = '456';

        // WHEN
        graph.updatePlot(PID, 20, 1, 'red');
        graph.updatePlot(PID, 30, 2, 'red');

        // THEN
        expect(graph.data[PID].y).toEqual([20, 30]);
    });

    it('random color generation', () => {
        // WHEN
        const color = graph.getRandomColor();
        
        // THEN
        expect(color).toMatch(/^rgb\(\d{1,3}, \d{1,3}, \d{1,3}\)$/);
    });
});
