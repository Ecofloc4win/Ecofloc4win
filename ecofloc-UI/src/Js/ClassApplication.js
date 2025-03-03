/**
 * @file ClassApplication.js
 * @brief Defines the Application class for managing applications.
 * @author Ecofloc's Team
 * @lastupdate 2025-02-18
 */

/**
 * @class Application
 * @brief Represents an application with its name, process IDs, and category.
 */
class Application 
{
    /**
     * @brief Constructor for the Application class.
     * @param {string} name - The name of the application.
     * @param {Array<number>} listPid - The list of process IDs associated with the application.
     * @param {string} category - The category of the application (e.g., browser or desktop).
     */
    constructor(name, listPid, category)
    {
        this.name = name;
        this.listPid = listPid;
        this.category = category;
    }

    /**
     * @function getName
     * @brief Gets the name of the application.
     * @returns {string} The name of the application.
     */
    getName()
    {
        return this.name;
    }

    /**
     * @function getListePid
     * @brief Gets the list of process IDs of the application.
     * @returns {Array<number>} The list of process IDs.
     */
    getListePid()
    {
        return this.listPid;
    }

    /**
     * @function getCategorie
     * @brief Gets the category of the application.
     * @returns {string} The category of the application.
     */
    getCategorie()
    {
        return this.category;
    }
}

export default Application;