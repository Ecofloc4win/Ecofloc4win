import { describe, it, expect, beforeEach, vi } from 'vitest';
import DynamicGraph from '../Js/ClassDynamicGraph.js';

describe('DynamicGraph', () => {
    let graph;
    
    beforeEach(() => {
        // Mock Plotly global object
        global.Plotly = {
            newPlot: vi.fn(),
            addTraces: vi.fn(),
            update: vi.fn()
        };
        
        graph = new DynamicGraph('testGraph');
    });

    it('should initialize with correct default values', () => {
        expect(graph.DATA).toEqual({});
        expect(graph.TRACE_INDICES).toEqual({});
        expect(graph.GRAPH_NAME).toBe('testGraph');
    });

    it('should update plot with new data', () => {
        graph.updatePlot(123, 50, 10, '#fff');
        
        expect(graph.DATA[123]).toBeDefined();
        expect(graph.DATA[123].x).toContain(10);
        expect(graph.DATA[123].y).toContain(50);
        expect(graph.DATA[123].line.color).toBe('#fff');
    });

    it('should clear all data', () => {
        graph.updatePlot(123, 50, 10, '#fff');
        graph.clearData();
        
        expect(graph.DATA).toEqual({});
        expect(graph.TRACE_INDICES).toEqual({});
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
