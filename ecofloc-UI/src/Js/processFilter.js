/**
 * @file processFilter.js
 * @brief This file contains JavaScript code for filtering and managing process data in a web application.
 *        It fetches process data from a server, handles filtering and state changes of processes,
 *        and updates the user interface dynamically based on user input.
 * @author Ecofloc's Team
 * @lastupdate 2025-02-18
 */

import { areSetsEqual } from './areSetsEqual';
import MapperApplication from './mapperApplication';

/**
 * @var {HTMLElement} listProcessHtmlElement
 * @brief The DOM element that holds the list of processes.
 */
const listProcessHtmlElement = document.getElementById("ListeProcessus");

/**
 * @var {HTMLElement} tableFilterHtmlElement
 * @brief The DOM element that holds the filter options.
 */
const tableFilterHtmlElement = document.getElementById("TableFilter");

/**
 * @var {HTMLElement} checkBoxSelectAllProcElement
 * @brief The checkbox element to select all processes.
 */
const checkBoxSelectAllProcElement = document.getElementById("SelectAllProc");

/**
 * @var {Array} myProcesses
 * @brief An array containing all the processes to be displayed.
 */
let myProcesses = [];

/**
 * @var {Set} setCategory
 * @brief A set containing the categories used for filtering.
 */
let setCategory = new Set();

/**
 * @function makeGroupApplication
 * @brief Creates filter checkboxes for each category and manages the filtering logic.
 * 
 * This function dynamically generates a group of checkboxes based on the categories of processes.
 * It listens for changes to the checkboxes to filter the process list accordingly.
 */
function makeGroupApplication(){
    if(tableFilterHtmlElement) {
        tableFilterHtmlElement.innerHTML = '';
        for (const item of setCategory) {
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
                showProcessList();
            });
        }
    }
}

/**
 * @function parseDataToMyProcesses
 * @brief Parses the fetched JSON data and updates the process list.
 * @param {Object} data - The JSON data fetched from the server.
 * 
 * This function processes the data and updates the `myProcesses` array and the `setCategory` set.
 * It also calls `makeGroupApplication` if categories have changed.
 */
function parseDataToMyProcesses(data)
{
    if(data)
    {
        myProcesses = [];
        const oldCategory = setCategory;
        setCategory = new Set();
        setCategory.add("All");
        setCategory.add("Other");
        myProcesses = MapperApplication.mapperApplicationsFromJson(data);
        for(let aProcess of myProcesses)
        {
            if(aProcess.setCategory != "") 
            {
                setCategory.add(aProcess.setCategory);
            }
        }
        if(!areSetsEqual(oldCategory, setCategory)) {
            makeGroupApplication();
        }
        showProcessList();
    }
}

fetch('../Json/process.json')
.then(response => {
    // Check if the answer is correct
    if (!response.ok) {
        throw new Error('Error loading JSON file');
    }
    return response.json(); // return the JSON
})
.then(data => {
    parseDataToMyProcesses(data);
})
.catch(error => {
    console.error('Error:', error);
});

/**
 * @function getFilterCategory
 * @brief Checks whether a category filter is selected.
 * @param {string} nameCategory - The name of the category to check.
 * @return {boolean} Returns true if the category filter is checked, false otherwise.
 */
function getFilterCategory(nameCategory) 
{
    for(let filter of tableFilterHtmlElement.querySelectorAll("input")){
        if(filter.value == nameCategory){
            return filter.checked;
        }
    }
    const otherFilterElement = document.getElementById("OtherFilter");
    return otherFilterElement.checked;
}

/**
 * @function changePidState
 * @brief Changes the state of a process identified by its PID.
 * @param {string} nameProc - The name of the process.
 * @param {number} pidProc - The PID of the process.
 * @param {boolean} state - The new state of the process (checked or unchecked).
 * 
 * This function sends a request to the server to update the state of the process with the given PID.
 */
function changePidState(nameProc, pidProc, state) 
{
    const serverUrl = 'http://localhost:3030/changePidState';
    fetch(serverUrl, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ nameProc, pidProc, state }),
    })
        .then(response => response.json())
        .then(data => {
            if (!data.success) {
                console.error('Error during execution :', data.message);
                alert(`Error : ${data.message}`);
            }
        })
        .catch(error => {
            console.error('Network or server error :', error);
            alert('Unable to contact the server.');
        });
}

/**
 * @function showProcessList
 * @brief Displays the list of processes according to the selected filters.
 * 
 * This function dynamically generates the list of processes, filtering by search text and selected categories.
 * It updates the `selectAllProcElement` checkbox based on the state of individual process checkboxes.
 */
function showProcessList() 
{
    while (listProcessHtmlElement.firstChild) 
    {
        listProcessHtmlElement.removeChild(listProcessHtmlElement.firstChild);
    }
    const searchText = document.getElementById("SearchBar").value == "" ? "":document.getElementById("SearchBar").value.toLowerCase();
    let atLeastOneChecked = false;
    let allChecked = true;
    for(let unProcessus of myProcesses)
    {
        if (!unProcessus.getName().toLowerCase().includes(searchText))
        {
            continue;
        }
        if(getFilterCategory(unProcessus.categorie))
        {
            for(let unPid of unProcessus.getListePid())
            {
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
                if(unPid["checked"])
                {
                    atLeastOneChecked = true;
                }
                else
                {
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
                listProcessHtmlElement.appendChild(lineDiv);
            }
        }
    }
    checkBoxSelectAllProcElement.checked = atLeastOneChecked;
    checkBoxSelectAllProcElement.indeterminate = (!allChecked && atLeastOneChecked); 
}

/**
 * @function selectAllPid
 * @brief Selects or deselects all PIDs based on the given state.
 * @param {boolean} etat - The state to set for all PIDs (true for checked, false for unchecked).
 * 
 * This function updates the state of all PIDs visible in the process list and sends the update to the server.
 */
function selectAllPid(etat) 
{
    const searchText = document.getElementById("SearchBar").value.toLowerCase();
    let listePidAChanger = new Set();

    // Collect all visible processes based on search and filters
    for(let unProcessus of myProcesses) 
    {
        if (searchText && !unProcessus.getName().toLowerCase().includes(searchText)) 
        {
            continue;
        }
        if(!getFilterCategory(unProcessus.categorie)) 
        {
            continue;
        }
        listePidAChanger.add(unProcessus.getName());
    }

    // Update UI immediately for better responsiveness
    for(let unProcessus of myProcesses) 
    {
        if(listePidAChanger.has(unProcessus.getName())) 
        {
            for(let unPid of unProcessus.getListePid()) 
            {
                unPid.checked = etat;
            }
        }
    }
    showProcessList();

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

// Event listener for the "Select All" checkbox to trigger the selectAllPid function.
checkBoxSelectAllProcElement.addEventListener("change", (event) => {
    const checked = event.target.checked;
    selectAllPid(checked);
});

// Event source to listen for server events and update the process list.
const eventSource = new EventSource('http://localhost:3030/events');
eventSource.onmessage = (event) => {
    try {        
        if (event.data[0] === "[")
        {
            const data = JSON.parse(event.data);
            if(!data)
            {
                console.error("Empty data error");
            }
            else
            {
                parseDataToMyProcesses(data);
            }
        }
        
    } catch (err) {
        console.error('Data parsing error:', err);
    }
};

// Event listener for the "Search Bar" input to filter the process list based on user input.
document.getElementById("SearchBar").addEventListener("keyup", () => {
    showProcessList();
});