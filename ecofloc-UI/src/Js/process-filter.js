import Application from './ClassApplication'; 
import { areSetsEqual } from './areSetsEqual';

const listeProcessusHtmlElement = document.getElementById("ListeProcessus");
const tableFilterHtmlElement = document.getElementById("TableFilter");
const checkBoxSelectAllProcElement = document.getElementById("SelectAllProc");

let mesProcessus = [];
let setCategorie = new Set();


function makeGroupApplication(){
    if(tableFilterHtmlElement) {
        tableFilterHtmlElement.innerHTML = '';
        for (const item of setCategorie) {
            const lineDiv = document.createElement('div');
            lineDiv.classList.add('line', 'col-3');

            const labelDiv = document.createElement('div');
            labelDiv.textContent = item;

            const input = document.createElement('input');
            input.type = 'checkbox';
            input.id = item+'Filter';
            input.value = item;
            input.checked = true;

            lineDiv.appendChild(labelDiv);
            lineDiv.appendChild(input);

            tableFilterHtmlElement.appendChild(lineDiv);
        }
        for(let filter of tableFilterHtmlElement.querySelectorAll("input")){
            filter.checked = true;
            filter.indeterminate = false;
            filter.addEventListener("change", (event) => {
                const currentFilter = event.target;
                if(currentFilter.value == "All"){
                    for(let filter of tableFilterHtmlElement.querySelectorAll("input")) {
                        filter.checked = currentFilter.checked;
                    }
                }
                else{
                    let all = true;
                    let atLeastOne = false;
                    for(let filter of tableFilterHtmlElement.querySelectorAll("input")) {
                        if(filter.checked && filter.value != "All"){
                            atLeastOne = true;
                        }
                        if(!filter.checked && filter.value != "All"){
                            all = false;
                        }
                    }
                    const allFilterElement = document.getElementById("AllFilter");
                    allFilterElement.checked = atLeastOne;
                    allFilterElement.indeterminate = (!all && atLeastOne); 
                }
                afficherListeProcessus();
            });
        }
    }
}

function parseDataToMyProcesses(data){
    if(data){
        mesProcessus = [];
        const oldCategorie = setCategorie;
        setCategorie = new Set();
        setCategorie.add("All");
        setCategorie.add("Other");
        data.forEach(process => {
            let uneApplication = new Application(process.name,process.pids, process.categorie);
            mesProcessus.push(uneApplication);
            if(uneApplication.categorie != "") {
                setCategorie.add(uneApplication.categorie);
            }
        });
        if(!areSetsEqual(oldCategorie, setCategorie)) {
            makeGroupApplication();
        }
        afficherListeProcessus();
    }
}

fetch('../Json/process.json')
.then(response => {
    // Check if the answer is correct
    if (!response.ok) {
        throw new Error('Erreur de chargement du fichier JSON');
    }
    return response.json(); // return the JSON
})
.then(data => {
    parseDataToMyProcesses(data);
})
.catch(error => {
    console.error('Erreur:', error);
});

function getFilterCategorie(nomCategorie) {
    for(let filter of tableFilterHtmlElement.querySelectorAll("input")){
        if(filter.value == nomCategorie){
            return filter.checked;
        }
    }
    const otherFilterElement = document.getElementById("OtherFilter");
    return otherFilterElement.checked;
}

function changePidState(nomProc, pidProc, etat) {
    const serverUrl = 'http://localhost:3030/changePidState';
    fetch(serverUrl, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ nomProc, pidProc, etat }),
    })
        .then(response => response.json())
        .then(data => {
            if (!data.success) {
                console.error('Erreur lors de l\'exécution :', data.message);
                alert(`Erreur : ${data.message}`);
            }
        })
        .catch(error => {
            console.error('Erreur réseau ou serveur :', error);
            alert('Impossible de contacter le serveur.');
        });
}



function afficherListeProcessus() {
    while (listeProcessusHtmlElement.firstChild) {
        listeProcessusHtmlElement.removeChild(listeProcessusHtmlElement.firstChild);
    }
    const searchText = document.getElementById("SearchBar").value == "" ? "":document.getElementById("SearchBar").value.toLowerCase();
    let atLeastOneChecked = false;
    let allChecked = true;
    for(let unProcessus of mesProcessus){
        if (!unProcessus.getName().toLowerCase().includes(searchText)){
            continue;
        }
        if(getFilterCategorie(unProcessus.categorie)){
            for(let unPid of unProcessus.getListePid()){
                // Create html elements for pids
                const lineDiv = document.createElement("div");
                lineDiv.className = "line";

                const col1Div = document.createElement("div");
                col1Div.className = "col-1 cell";
                col1Div.textContent = unProcessus.getName();

                const col2Div = document.createElement("div");
                col2Div.className = "col-2 cell";

                const inputCheckbox = document.createElement("input");
                inputCheckbox.type = "checkbox";
                col2Div.textContent = unPid["numeroPid"];
                if(unPid["checked"]){
                    atLeastOneChecked = true;
                }else{
                    allChecked = false;
                }
                inputCheckbox.checked = unPid["checked"];
                inputCheckbox.setAttribute("data-nom-processus", unProcessus.getName());
                inputCheckbox.setAttribute("data-numero-pids", unPid["numeroPid"]);
                inputCheckbox.addEventListener('click', function(event) {
                    let clickedCheckbox = event.target;
                    const dataNomProcessus = event.target.getAttribute("data-nom-processus");
                    const dataNumeroPid = event.target.getAttribute("data-numero-pids");
                    changePidState(dataNomProcessus, dataNumeroPid,clickedCheckbox.checked);
                });
                col2Div.appendChild(inputCheckbox); 

                // Add columns to line
                lineDiv.appendChild(col1Div);
                lineDiv.appendChild(col2Div);

                // Add line to parent element
                listeProcessusHtmlElement.appendChild(lineDiv);
            }
        }
    }
    checkBoxSelectAllProcElement.checked = atLeastOneChecked;
    checkBoxSelectAllProcElement.indeterminate = (!allChecked && atLeastOneChecked); 
}

function selectAllPid(etat) {
    const searchText = document.getElementById("SearchBar").value.toLowerCase();
    let listePidAChanger = new Set();

    // Collect all visible processes based on search and filters
    for(let unProcessus of mesProcessus) {
        if (searchText && !unProcessus.getName().toLowerCase().includes(searchText)) {
            continue;
        }
        if(!getFilterCategorie(unProcessus.categorie)) {
            continue;
        }
        listePidAChanger.add(unProcessus.getName());
    }

    // Update UI immediately for better responsiveness
    for(let unProcessus of mesProcessus) {
        if(listePidAChanger.has(unProcessus.getName())) {
            for(let unPid of unProcessus.getListePid()) {
                unPid.checked = etat;
            }
        }
    }
    afficherListeProcessus();

    // Send update to server
    fetch('http://localhost:3030/updateProcessListState', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            list: Array.from(listePidAChanger),
            state: etat
        })
    })
    .then(response => response.json())
    .then(data => {
        console.log('Server response:', data);
    })
    .catch(error => {
        console.error('Error updating process states:', error);
    });
}

// Update the checkbox event listener
checkBoxSelectAllProcElement.addEventListener("change", (event) => {
    const checked = event.target.checked;
    selectAllPid(checked);
});

const eventSource = new EventSource('http://localhost:3030/events');

eventSource.onmessage = (event) => {
    try {        
        if (event.data[0] === "[")
        {
            const data = JSON.parse(event.data);
            if(!data)
            {
                console.error("Erreur data vide");
            }
            else
            {
                parseDataToMyProcesses(data);
            }
        }
        
    } catch (err) {
        console.error('Erreur de parsing des données:', err);
    }
};

document.getElementById("SearchBar").addEventListener("keyup", () => {
    afficherListeProcessus();
});