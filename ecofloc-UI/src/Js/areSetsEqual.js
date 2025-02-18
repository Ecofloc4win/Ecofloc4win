/**
 * @file areSetsEqual.js
 * @brief Provides a utility function to compare two sets.
 * @author Ecofloc's Team
 * @lastupdate 2025-02-18
 */

/**
 * @brief Determines if two sets are equal.
 * @function areSetsEqual
 * @param {Set} set1 - The first set.
 * @param {Set} set2 - The second set.
 * @returns {boolean} True if the sets are equal, false otherwise.
 */
export function areSetsEqual(set1, set2) {
    if (set1.size !== set2.size) {
        return false;
    }
    for (let item of set1) {
        if (!set2.has(item)) {
            return false;
        }
    }
    return true;
}