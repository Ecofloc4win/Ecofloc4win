import { describe, it, expect } from 'vitest';
import { areSetsEqual } from '../Js/areSetsEqual';

describe('areSetsEqual', () => {
    it('areSetsEqual deux sets égaux', () => {
        // GIVEN
        const set1 = new Set();
        const set2 = new Set();
        
        // WHEN
        set1.add("All");
        set2.add("Other");
        set1.add("Other");
        set2.add("All");

        // THEN
        expect(areSetsEqual(set1, set2)).toBe(true);
        expect(areSetsEqual(set2, set1)).toBe(true);
    });

    it('areSetsEqual deux sets de même taille mais non égaux', () => {
        // GIVEN
        const set1 = new Set();
        const set2 = new Set();
        
        // WHEN
        set1.add("All");
        set2.add("Other");
        set1.add("Test");
        set2.add("All");

        // THEN
        expect(areSetsEqual(set1, set2)).toBe(false);
        expect(areSetsEqual(set2, set1)).toBe(false);
    });

    it('areSetsEqual deux sets différents', () => {
        // GIVEN
        const set1 = new Set();
        const set2 = new Set();
        
        // WHEN
        set1.add("All");
        set1.add("Test");
        set2.add("All");
        set2.add("Other");
        set2.add("Test");

        // THEN
        expect(areSetsEqual(set1, set2)).toBe(false);
        expect(areSetsEqual(set2, set1)).toBe(false);
    });
});
