import { describe, it, expect, beforeEach, vi } from 'vitest';
import Application from '../Js/ClassApplication'; 

describe('Application', () => {
    it('Création d\'une Application', () => {
        let uneApplication = new Application("Test",[0,1,2], "Browser");
        expect(uneApplication.getName(), "Test");
        expect(uneApplication.getListePid(), [0,1,2]);
        expect(uneApplication.getCategorie(), "Browser");
    });

});
