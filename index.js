const express = require('express');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const http = require('http');
const socketIo = require('socket.io');
const fs = require('fs');
const path = require('path');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

app.use(express.static('public')); // serve frontend

const LOG_FILE = path.join(__dirname, 'logs.txt');

// Function to automatically detect the serial port
const detectSerialPort = () => {
  return new Promise((resolve, reject) => {
    SerialPort.list((err, ports) => {
      if (err) {
        reject('Error detecting serial ports');
      } else {
        let serialPort = null;
        
        // Search for the appropriate port based on OS and common patterns
        if (process.platform === 'win32') {
          serialPort = ports.find(port => port.path.includes('COM'));
        } else if (process.platform === 'darwin') {
          serialPort = ports.find(port => port.path.includes('/dev/cu.'));
        } else if (process.platform === 'linux') {
          serialPort = ports.find(port => port.path.includes('/dev/ttyACM') || port.path.includes('/dev/ttyUSB'));
        }

        if (serialPort) {
          resolve(serialPort.path);
        } else {
          reject('No serial port found');
        }
      }
    });
  });
};

// Initialize the Serial Port
const initializeSerialPort = async () => {
  try {
    const SERIAL_PATH = await detectSerialPort();
    console.log(`✅ Detected serial port: ${SERIAL_PATH}`);

    const port = new SerialPort({
      path: SERIAL_PATH,
      baudRate: 9600,
      autoOpen: false,
    });

    const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

    // Open the serial port
    port.open((err) => {
      if (err) {
        console.error('❌ Error opening serial port:', err.message);
      } else {
        console.log(`✅ Serial port opened at ${SERIAL_PATH}`);
      }
    });

    // Read data from Arduino
    parser.on('data', (data) => {
      const cleanData = data.trim();
      const timestamp = new Date().toISOString();
      const logLine = `${timestamp} 📥 ${cleanData}`;
      console.log(logLine);

      io.emit('serial-data', cleanData);

      fs.appendFile(LOG_FILE, logLine + '\n', (err) => {
        if (err) console.error('❌ Log file write error:', err);
      });
    });

    return port;
  } catch (error) {
    console.error('❌ Failed to initialize serial port:', error);
    return null;
  }
};

// WebSocket communication
io.on('connection', (socket) => {
  console.log('⚡ Web client connected');

  // Send the current log on connection
  fs.readFile(LOG_FILE, 'utf8', (err, data) => {
    if (!err && data) socket.emit('log-update', data);
  });

  // Message from web UI -> Arduino
  socket.on('send-to-arduino', (msg) => {
    const message = msg.trim();
    if (message.length > 0) {
      const port = socket.port;
      if (port && port.isOpen) {
        console.log(`📤 Sending to Arduino: ${message}`);
        port.write(message + '\n', (err) => {
          if (err) console.error('❌ Error writing to serial:', err.message);
        });
      }
    }
  });

  socket.on('disconnect', () => {
    console.log('❌ Web client disconnected');
  });
});

// Periodically push log updates
setInterval(() => {
  fs.readFile(LOG_FILE, 'utf8', (err, data) => {
    if (!err && data) io.emit('log-update', data);
  });
}, 2000);

// Start the web server
const PORT = 3000;
server.listen(PORT, async () => {
  console.log(`🌐 Server running at http://localhost:${PORT}`);
  const port = await initializeSerialPort();
  if (port) {
    // Attach the port to socket connections
    io.on('connection', (socket) => {
      socket.port = port; // Attach the port to the socket
    });
  }
});

