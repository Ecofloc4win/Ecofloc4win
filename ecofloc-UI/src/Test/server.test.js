import { describe, it, expect, beforeEach, vi } from 'vitest';
import express from 'express';
import fs from 'fs';
import path from 'path';

vi.mock('express');
vi.mock('fs');
vi.mock('path');
vi.mock('child_process');

describe('Server', () => {
    beforeEach(() => {
        vi.resetModules();
        vi.clearAllMocks();
    });

    describe('API Endpoints', () => {
        describe('POST /execute', () => {
            it('should handle process already running', async () => {
                const req = { body: {} };
                const res = {
                    status: vi.fn().mockReturnThis(),
                    json: vi.fn()
                };

                global.IS_PROCESS_RUNNING = true;
                await executeHandler(req, res);

                expect(res.status).toHaveBeenCalledWith(400);
                expect(res.json).toHaveBeenCalledWith({
                    success: false,
                    message: 'Process already running'
                });
            });
        });

        describe('POST /stop', () => {
            it('should stop all monitoring processes', async () => {
                const req = {};
                const res = {
                    json: vi.fn()
                };

                await stopHandler(req, res);

                expect(res.json).toHaveBeenCalledWith({
                    success: true,
                    message: 'All monitoring processes stopped and files cleaned'
                });
            });
        });
    });

    describe('File Utils', () => {
        describe('resetMonitoringFiles', () => {
            it('should create initial JSON files for PIDs', () => {
                const pids = [123, 456];
                fileUtils.resetMonitoringFiles(pids);

                expect(fs.writeFileSync).toHaveBeenCalledTimes(2);
            });
        });

        describe('exportToCSV', () => {
            it('should format data correctly', () => {
                const data = {
                    CPU: {
                        123: {
                            x: [1000],
                            y: [50]
                        }
                    }
                };

                const csv = fileUtils.exportToCSV(data);
                expect(csv).toContain('Timestamp,Component,PID,Value');
            });
        });
    });
}); 