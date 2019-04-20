
const express = require('express');
var http = require("http");
var t = require('typebase');
var util = require('util');
var net = require('net');

var app = express();

var server = require('http').Server(app);
var io = require('socket.io').listen(server);

var client = new net.Socket();

var webSocket = {};

var port = 3000;

server.listen(port);

io.on('connection', function (socket) {

	console.log("Web Socket Connected!");

	socket.on('request', function (data) {

		var buff = packRadixLinkBuffer(data);

		client.write(buff);
		
	});

	webSocket = socket;

});

client.connect(27016, '127.0.0.1', function() {

	console.log("Client Connected!");
});


client.on('data', function(data) {

	var str = unpackRadixLinkBuffer(data);

	webSocket.emit("result", str);
	
});

const SIZE_HEADER   = 4;
const SIZE_TYPE	    = 4;
const SIZE_JOBID    = 4;
const SIZE_PAYLOAD  = 4;

function packRadixLinkBuffer(data) {
	
	var currOffset = 0;

	var totalSize = SIZE_HEADER + SIZE_TYPE + SIZE_JOBID + SIZE_PAYLOAD + data.length;

	var buff = Buffer.alloc(totalSize);

	//Write transaction size to buffer
	buff.writeInt32LE(totalSize, currOffset);

	currOffset += SIZE_HEADER;

	//Write type to buffer
	buff.writeInt32LE(4, currOffset);
	
	currOffset += SIZE_TYPE;

	//Write jobID to buffer
	buff.writeUInt32LE(1, currOffset);

	currOffset += SIZE_JOBID;

	//Write payloadSize to buffer
	buff.writeInt32LE(data.length, currOffset);

	currOffset += SIZE_PAYLOAD;

	//Write payload to buffer
	buff.write(data, currOffset);

	return buff;
}

function unpackRadixLinkBuffer(buffer) {

	var payloadSize = 0;
	var offset      = 0;

	payloadSize = buffer.readInt32LE(SIZE_HEADER + SIZE_TYPE + SIZE_JOBID);

	offset = SIZE_HEADER + SIZE_TYPE + SIZE_JOBID + SIZE_PAYLOAD;

	var msg = buffer.slice(offset, offset + payloadSize)

	// console.log("-----------------------------------------------------------------------------------------");
	// console.log("----.... PAYLOAD_SIZE: " + payloadSize);
	// console.log("----... BUFFER: ");
	// console.log(buffer);
	// console.log("-----------------------------------------------------------------------------------------");

	// if(msg.toString().length == 1) {

	// 	console.log("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	// 	console.log("!!!.... PAYLOAD_SIZE: " + payloadSize);
	// 	console.log("!!!.... BUFFER: ");
	// 	console.log(buffer);
	// 	console.log("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

	// }

	return msg.toString();

}

// app.post('/', function(req, res, next) {});

