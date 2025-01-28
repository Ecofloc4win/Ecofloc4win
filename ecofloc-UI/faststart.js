const { exec } = require('child_process');
const readline = require('readline');

// Function to execute a command as a promise
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

// Main function to execute steps sequentially
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
