import { describe, it, expect, beforeEach, vi } from 'vitest';
import { filterProcesses, updateProcessList } from '../Js/process-filter.js';

describe('Process Filter', () => {
    beforeEach(() => {
        document.body.innerHTML = `
            <div id="ListeProcessus"></div>
            <input id="SearchBar" value="" />
            <input id="AllFilter" type="checkbox" />
            <input id="OfficeApplicationFilter" type="checkbox" />
            <input id="NavigateurFilter" type="checkbox" />
            <input id="OtherFilter" type="checkbox" />
        `;
    });

    describe('filterProcesses', () => {
        it('should filter by search text', () => {
            const processes = [
                { name: 'chrome.exe', type: 'Browser' },
                { name: 'word.exe', type: 'Office' }
            ];

            document.querySelector('#SearchBar').value = 'chrome';
            const filtered = filterProcesses(processes);

            expect(filtered).toHaveLength(1);
            expect(filtered[0].name).toBe('chrome.exe');
        });

        it('should filter by type', () => {
            const processes = [
                { name: 'chrome.exe', type: 'Browser' },
                { name: 'word.exe', type: 'Office' }
            ];

            document.querySelector('#NavigateurFilter').checked = true;
            const filtered = filterProcesses(processes);

            expect(filtered).toHaveLength(1);
            expect(filtered[0].type).toBe('Browser');
        });
    });

    describe('updateProcessList', () => {
        it('should update DOM with filtered processes', () => {
            const processes = [
                { name: 'chrome.exe', type: 'Browser' }
            ];

            updateProcessList(processes);

            const list = document.querySelector('#ListeProcessus');
            expect(list.innerHTML).toContain('chrome.exe');
        });
    });
}); 