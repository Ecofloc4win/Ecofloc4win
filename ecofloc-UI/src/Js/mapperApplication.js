/**
 * @file mapperApplication.js
 * @brief Defines the MapperApplication class for mapping JSON data to Application objects.
 * @author Ecofloc's Team
 * @lastupdate 2025-02-18
 */

import Application from './ClassApplication';

/**
 * @class MapperApplication
 * @brief A class responsible for mapping JSON data to Application objects.
 */
class MapperApplication 
{
    /**
     * @brief Maps JSON data to an array of Application objects.
     * @function mapperApplicationsFromJson
     * @param {Array<Object>} dataJson - The JSON data containing application details.
     * @returns {Array<Application>} An array of Application objects.
     */
    static mapperApplicationsFromJson (dataJson)
    {
        let resultat = [];  // Array of Application
        dataJson.forEach(process => {
            let uneApplication = new Application(process.name,process.pids, process.categorie);

            if(uneApplication) 
            {
                resultat.push(uneApplication);
            }
            else
            {
                console.error("Data incorrect", process);
            }
        });
        return resultat;
    }
}

export default MapperApplication; 