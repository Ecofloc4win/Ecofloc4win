import { describe, it, expect, beforeEach, vi } from 'vitest';
const { getCheckedPIDs, launchMonitoring } = require('../Js/Utils.cjs');
const fs = require('fs');
const path = require('path');

vi.mock('fs');
vi.mock('child_process');

describe('Utils', () => {
    beforeEach(() => {
        vi.resetModules();
        vi.clearAllMocks();
    });

    describe('getCheckedPIDs', () => {
        it('should return empty array when no process.json exists', async () => {
            fs.existsSync.mockReturnValue(false);
            const pids = await getCheckedPIDs();
            expect(pids).toEqual([]);
        });

        it('should return checked PIDs from process.json', async () => {
            fs.existsSync.mockReturnValue(true);
            fs.readFileSync.mockReturnValue(JSON.stringify([
                {
                    pids: [
                        { numeroPid: 123, checked: true },
                        { numeroPid: 456, checked: false }
                    ]
                }
            ]));

            const pids = await getCheckedPIDs();
            expect(pids).toEqual([123]);
        });
    });

    describe('launchMonitoring', () => {
        it('should launch monitoring process with correct parameters', () => {
            const process = launchMonitoring(123, 'path/to/exe', 'path/to/metrics', 1000);
            expect(process).toBeDefined();
        });

        it('should handle process exit and cleanup', () => {
            global.MONITORING_PROCESSES = new Map();
            const process = launchMonitoring(123, 'path/to/exe', 'path/to/metrics', 1000);
            
            process.emit('exit', 0, null);
            
            expect(global.MONITORING_PROCESSES.has(123)).toBe(false);
        });
    });
}); 