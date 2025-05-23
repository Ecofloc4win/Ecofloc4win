/**
 * @file faststart.js
 * @brief Script to automate server and development environment startup.
 *
 * This script executes multiple commands sequentially to start the Node.js server,
 * the Vite development server, and open the browser automatically.
 * @author Ecofloc's Team
 * @lastupdate 2025-02-18
 */

/**
 * @var {Object} exec
 * @brief Module to execute shell commands.
 */
const { exec } = require('child_process');

/**
 * @var {Object} readline
 * @brief Module to handle user input in the terminal.
 */
const readline = require('readline');

/**
 * @brief Executes a shell command as a promise.
 * @function runCommand
 * @param {string} command The command to execute.
 * @param {string} description Description of the process.
 * @return {Promise<void>} Resolves when the command completes, rejects on error.
 */
function runCommand(command, description) {
  return new Promise((resolve, reject) => {
    console.log(`Starting: ${description}`);
    const process = exec(command, (err, stdout, stderr) => {
      if (err) {
        console.error(`Error in ${description}:`, err.message);
        reject(err);
        return;
      }
      if (stdout) console.log(`${description} stdout: ${stdout}`);
      if (stderr) console.error(`${description} stderr: ${stderr}`);
      resolve();
    });

    process.stdout.on('data', (data) => {
      console.log(`[${description}] stdout: ${data.toString()}`);
    });

    process.stderr.on('data', (data) => {
      console.error(`[${description}] stderr: ${data.toString()}`);
    });
  });
}

/**
 * @brief Main function to execute startup steps sequentially.
 * @function main
 * @return {Promise<void>} Resolves when all steps are completed.
 */
async function main() {
  try {
    // Step 1: Start the Node.js server
    runCommand('node ./ecofloc-UI/src/server.cjs', 'Node.js Server');

    // Step 2: Start the Vite server
    runCommand('npm run dev --prefix ./ecofloc-UI', 'Vite Development Server');

    // Step 3: Open the browser
    await runCommand('start http://localhost:5173/src/', 'Open Browser');
  } catch (err) {
    console.error('An error occurred during execution:', err.message);
  } finally {
    // Keep terminal window open
    readline
      .createInterface({
        input: process.stdin,
        output: process.stdout,
      })
      .question('Press ENTER to exit...', () => {
        process.exit();
      });
  }
}

// Run the main script
main();
