import Application from './ClassApplication'; 
class MapperApplication 
{
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