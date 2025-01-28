import { describe, it, expect } from 'vitest';
import MapperApplication from '../Js/mapperApplication';

describe('mapperApplication', () => {
    it('Mapping json data to an array of application', () => {
        // GIVEN
        const json = '[{"categorie": "","name": "[System Process]","pids":[{"checked":true,"numeroPid":0}]}, {"categorie": "","name": "System","pids":[{"checked":true,"numeroPid":4}]}, {"categorie": "","name": "Registry","pids":[{"checked":true,"numeroPid":108}]}, {"categorie": "","name": "smss.exe","pids":[{"checked":true,"numeroPid":1308}]}]';
        const jsonObject = JSON.parse(json);
        // WHEN 
        const mesApplications = MapperApplication.mapperApplicationsFromJson(jsonObject);

        // THEN
        expect(mesApplications.length).toBe(4);
    });

});