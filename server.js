const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const { spawn } = require('child_process');
const path = require('path');

const app = express();
app.use(cors());
app.use(bodyParser.json());

// Serve the frontend
app.use(express.static(path.join(__dirname)));

// Start the compiler process
const compilerProcess = spawn('compiler.exe');

let outputBuffer = '';
compilerProcess.stdout.on('data', (data) => {
    outputBuffer += data.toString();
});

compilerProcess.stderr.on('data', (data) => {
    outputBuffer += data.toString();
});

// Route to send code to the compiler
app.post('/compile', (req, res) => {
    const code = req.body.code + '\n';
    outputBuffer = '';  // Clear previous output
    compilerProcess.stdin.write(code);  // Send input to compiler

    // Wait briefly to capture the output 
    setTimeout(() => {
        res.send(outputBuffer || "No output produced");
    }, 500);
});

// Default Route
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// Start server on port 8000
app.listen(8000, () => console.log('Server running on http://localhost:8000'));
