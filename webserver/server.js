
const express = require('express');
var http = require("http");
var t = require('typebase');
var util = require('util');
var net = require('net');

var app = express();

var port = 8000;
var serverUrl = "localhost";

var requestQueue = [];

console.log("Starting web server at " + serverUrl + ":" + port);

app.use(function(req, res, next) {

	res.header("Access-Control-Allow-Origin", "*");
	res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");

	next();

});



var client = new net.Socket();
client.connect(27016, '127.0.0.1', function() {
	console.log('Connected');

});

client.on('data', function(data) {

	//This check is here to so that we skip the total size header.
	if(data.length > 4) {

		var str = unpackRadixLinkBuffer(data);

		console.log(str);
		
		if(requestQueue.length == 1) {
			var req = requestQueue.shift();

			req.setHeader('Content-Type', 'charset=utf-8');
			req.end(str);
		}
	}
});

const SIZE_HEADER   = 4;
const SIZE_TYPE	    = 4;
const SIZE_JOBID    = 4;
const SIZE_PAYLOAD  = 4;

function packRadixLinkBuffer(data) {
	
	data = data + '\0';

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

app.post('/', function(req, res, next) {

	req.on('data', function (chunk) {

		client.write(packRadixLinkBuffer(chunk.toString()));
		
		if(requestQueue.length == 0)
			requestQueue.push(res);

	});
});

app.listen(port)
