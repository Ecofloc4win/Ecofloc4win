import { describe, it, expect, beforeEach, vi } from 'vitest';
import { readFile, updatePlots, updateControlsState } from '../Js/graph-display.js';

describe('graph-display', () => {
    beforeEach(() => {
        // Mock fetch
        global.fetch = vi.fn(() => Promise.resolve({
            ok: true,
            json: () => Promise.resolve([])
        }));

        // Mock DOM elements
        document.body.innerHTML = `
            <button id="start-button"></button>
            <button id="stop-button"></button>
            <button id="clear-button"></button>
            <button id="exportCSV"></button>
            <input id="SearchBar" />
            <input id="intervalTime" type="number" value="1000" />
            <input type="checkbox" class="green-checkbox" />
        `;

        // Reset global state
        global.IS_MONITORING = false;
        global.HAS_DATA = false;
    });

    describe('updateControlsState', () => {
        it('should disable controls when monitoring is active', () => {
            global.IS_MONITORING = true;
            updateControlsState();
            
            expect(document.querySelector('#start-button').disabled).toBe(true);
            expect(document.querySelector('#stop-button').disabled).toBe(false);
            expect(document.querySelector('#clear-button').disabled).toBe(true);
            expect(document.querySelector('#SearchBar').disabled).toBe(true);
            expect(document.querySelector('#intervalTime').disabled).toBe(true);
        });

        it('should enable controls when monitoring is inactive', () => {
            global.IS_MONITORING = false;
            updateControlsState();
            
            expect(document.querySelector('#start-button').disabled).toBe(false);
            expect(document.querySelector('#stop-button').disabled).toBe(true);
            expect(document.querySelector('#clear-button').disabled).toBe(false);
        });
    });

    describe('readFile', () => {
        it('should not fetch data when monitoring is inactive', async () => {
            global.IS_MONITORING = false;
            await readFile();
            expect(global.fetch).not.toHaveBeenCalled();
        });

        it('should fetch and process data when monitoring is active', async () => {
            global.IS_MONITORING = true;
            global.fetch.mockImplementationOnce(() => Promise.resolve({
                ok: true,
                json: () => Promise.resolve(['123', '456'])
            }));

            await readFile();
            expect(global.fetch).toHaveBeenCalledWith('http://localhost:3030/monitored-pids');
        });
    });

    describe('start monitoring', () => {
        it('should send correct interval to server', async () => {
            const startButton = document.querySelector('#start-button');
            const intervalInput = document.querySelector('#intervalTime');
            intervalInput.value = '2000';

            await startButton.click();

            expect(global.fetch).toHaveBeenCalledWith(
                'http://localhost:3030/execute',
                expect.objectContaining({
                    method: 'POST',
                    body: JSON.stringify({ interval: 2000 })
                })
            );
        });
    });

    describe('stop monitoring', () => {
        it('should clear interval and update state', async () => {
            global.IS_MONITORING = true;
            const stopButton = document.querySelector('#stop-button');

            await stopButton.click();

            expect(global.IS_MONITORING).toBe(false);
            expect(global.fetch).toHaveBeenCalledWith(
                'http://localhost:3030/stop',
                expect.objectContaining({
                    method: 'POST'
                })
            );
        });
    });
}); 