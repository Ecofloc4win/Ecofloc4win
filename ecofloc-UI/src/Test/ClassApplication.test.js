import { describe, it, expect } from 'vitest';
import Application from '../Js/ClassApplication'; 

describe('Application', () => {
    it('Creating an Application', () => {
        // GIVEN
        let uneApplication = new Application("Test",[0,1,2], "Browser");
        
        // THEN
        expect(uneApplication.getName(), "Test");
        expect(uneApplication.getListePid(), [0,1,2]);
        expect(uneApplication.getCategorie(), "Browser");
    });

});
