
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

		client.write(packRadixLinkBuffer(data));
		
	});

	webSocket = socket;

});

client.connect(27016, '127.0.0.1', function() {

	console.log("Client Connected!");
});


client.on('data', function(data) {

	//This check is here to so that we skip the total size header.
	if(data.length > 4) {

		var str = unpackRadixLinkBuffer(data);

		console.log(str);

		webSocket.emit("result", str);
	}
});

const SIZE_HEADER   = 4;
const SIZE_TYPE	    = 4;
const SIZE_JOBID    = 4;
const SIZE_PAYLOAD  = 4;

function packRadixLinkBuffer(data) {
	
	console.log("Outgoing: " + data);

	data = data;

	var currOffset = 0;

	var totalSize = SIZE_HEADER + SIZE_TYPE + SIZE_JOBID + SIZE_PAYLOAD + data.length;

	var buff = Buffer.alloc(totalSize);

	//Write transaction size to buffer
	buff.writeInt32LE(totalSize - SIZE_HEADER, currOffset);

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

	var str = "";
	var payloadSize = 0;

	//Payload is 32bit int
	payloadSize = buffer.readInt32LE(SIZE_TYPE + SIZE_JOBID);

	var offset = SIZE_TYPE + SIZE_JOBID + SIZE_PAYLOAD;

	var msg = buffer.slice(offset, offset + payloadSize)

	return msg.toString();

}

// app.post('/', function(req, res, next) {});

