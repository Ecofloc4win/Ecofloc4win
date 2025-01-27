import { describe, it, expect, beforeEach, vi } from 'vitest';
import DynamicGraph from '../Js/ClassDynamicGraph';  // Importation avec ES6 (modifié pour Vitest)

// Mock de Plotly
global.Plotly = {
    newPlot: vi.fn(),
    addTraces: vi.fn(),
    update: vi.fn()
};

describe('DynamicGraph', () => {
    let graph;

    // Initialisation avant chaque test
    beforeEach(() => {
        graph = new DynamicGraph('graphDiv'); // Crée une nouvelle instance de DynamicGraph
    });

    it('ajout d\'une trace pour un nouveau PID', () => {
        const PID = '123';
        const initialTraces = Object.keys(graph.traceIndices).length;

        // Mise à jour du graphique avec un nouveau PID
        graph.updatePlot(PID, 10, 1, 'blue');

        // Vérifie que la trace a bien été ajoutée
        expect(Object.keys(graph.traceIndices).length).toBe(initialTraces + 1);
        expect(graph.traceIndices[PID]).toBe(0);  // Le PID "123" devrait avoir l'index 0
        expect(Plotly.addTraces).toHaveBeenCalledTimes(1); // Vérifie que Plotly.addTraces a été appelé une fois
    });

    it('ajout de la trace pour le PID TOTAL', () => {
        // Test pour le PID TOTAL
        graph.updatePlot('TOTAL', 50, 1, 'green');

        // Vérifie que la trace TOTAL est bien ajoutée avec l'index approprié
        expect(Object.keys(graph.traceIndices).length).toBe(1);  // Total devrait être la première trace
        expect(graph.traceIndices['TOTAL']).toBe(0);
    });

    it('mise à jour d\'une trace existante', () => {
        const PID = '456';
        graph.updatePlot(PID, 20, 1, 'red');

        // Mise à jour avec une nouvelle valeur
        graph.updatePlot(PID, 30, 2, 'red');

        // Vérifie que la nouvelle valeur a bien été ajoutée à la trace
        expect(graph.data[PID].y).toEqual([20, 30]);
    });

    it('génération d\'une couleur aléatoire', () => {
        const color = graph.getRandomColor();

        // Vérifie si la couleur est une chaîne de caractères de format rgb(r, g, b)
        expect(color).toMatch(/^rgb\(\d{1,3}, \d{1,3}, \d{1,3}\)$/);
    });
});
