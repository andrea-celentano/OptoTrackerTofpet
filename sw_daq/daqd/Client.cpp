#include "Client.hpp"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
using namespace std;
using namespace DAQd;

Client::Client(int socket, FrameServer *frameServer)
: socket(socket), frameServer(frameServer)
{
}

Client::~Client()
{
	close(socket);
}

struct CmdHeader_t { uint16_t type; uint16_t length; };

int Client::handleRequest()
{	
	CmdHeader_t cmdHeader;
	
	int nBytesRead = recv(socket, socketBuffer, sizeof(cmdHeader), 0);
	if(nBytesRead !=  sizeof(cmdHeader)) {
		fprintf(stderr, "Could not read() %u bytes from client %d\n", sizeof(CmdHeader_t), socket);
		return -1;
	}
	
	memcpy(&cmdHeader, socketBuffer, sizeof(CmdHeader_t));
	
	if(cmdHeader.length > sizeof(socketBuffer))
		return -1;
	
	int nBytesNext = cmdHeader.length - sizeof(CmdHeader_t);
	if(nBytesNext > 0) {
		if(recv(socket, socketBuffer+sizeof(CmdHeader_t), nBytesNext, 0) != nBytesNext) {
			fprintf(stderr, "Could not read() %d bytes from client %d\n", nBytesNext, socket);
			return -1;
		}
	}

	int actionStatus = 0;
//	fprintf(stderr, "commandType = %d, commandLength = %d\n", int(cmdHeader.type), int(cmdHeader.length));
	if (cmdHeader.type == commandAcqOnOff)
		doAcqOnOff();
	else if(cmdHeader.type == commandGetDataFrameSharedMemoryName) 
		actionStatus = doGetDataFrameSharedMemoryName();
	else if(cmdHeader.type == commandGetDataFrameWriteReadPointer)
		actionStatus = doGetDataFrameWriteReadPointer();
	else if(cmdHeader.type == commandSetDataFrameReadPointer)
		actionStatus = doSetDataFrameReadPointer();
	else if(cmdHeader.type == commandToFrontEnd)
		actionStatus = doCommandToFrontEnd(nBytesNext);
	else if(cmdHeader.type == commandGetPortUp) 
		actionStatus = doGetPortUp();
	else if(cmdHeader.type == commandGetPortCounts) 
		actionStatus = doGetPortCounts();
	
	if(actionStatus == -1) {
		fprintf(stderr, "Error handling client %d, command was %u\n", socket, unsigned(cmdHeader.type));
		return -1;
	}
	
	return 0;
}

int Client::doCommandToFrontEnd(int commandLength)
{
	char buffer[256];
	memcpy(buffer, socketBuffer + sizeof(CmdHeader_t), commandLength);
	int portID = buffer[0];
	int slaveID = buffer[1];
	int replyLength = frameServer->sendCommand(portID, slaveID, buffer+2, sizeof(buffer), commandLength-2);
	
	uint16_t trl = replyLength;

	send(socket, &trl, sizeof(trl), MSG_NOSIGNAL);
	send(socket, buffer+2, replyLength, MSG_NOSIGNAL);
	return 0;
}

int Client::doAcqOnOff()
{

	uint16_t acqMode = 0;
	memcpy(&acqMode, socketBuffer + sizeof(CmdHeader_t), sizeof(uint16_t));
	printf("Client::doAcqOnOff() called with acqMode = %hu\n", acqMode);
	if (acqMode != 0) 
		frameServer->startAcquisition(acqMode);
	else
		frameServer->stopAcquisition();
	printf("Client::doAcqOnOff() exiting...\n");
	return 0;
}

int Client::doGetDataFrameWriteReadPointer()
{
	struct { uint16_t length; uint32_t wrPointer; uint32_t rdPointer; }  header;
	header.length = sizeof(header);
	header.wrPointer = frameServer->getDataFrameWritePointer();
	header.rdPointer = frameServer->getDataFrameReadPointer();

	int status = send(socket, &header, sizeof(header), MSG_NOSIGNAL);
	if(status < sizeof(header)) return -1;
	return 0;
}

int Client::doSetDataFrameReadPointer()
{
	uint32_t readPointer;
	memcpy(&readPointer, socketBuffer + sizeof(CmdHeader_t), sizeof(readPointer));	
	frameServer->setDataFrameReadPointer(readPointer);	
	int status = send(socket, &readPointer, sizeof(readPointer), MSG_NOSIGNAL);
	if(status < sizeof(readPointer)) return -1;
	return 0;
	
}

int Client::doGetDataFrameSharedMemoryName()
{
	const char *name = frameServer->getDataFrameSharedMemoryName();
	
	struct { uint16_t length;  uint64_t sizes[3]; } header;
	header.length = sizeof(header) + strlen(name);
	header.sizes[0] = sizeof(DataFrame);
	header.sizes[1] = 0;
	header.sizes[2] = 0;
	
	int status = 0;
	status = send(socket, &header, sizeof(header), MSG_NOSIGNAL);
	if(status < sizeof(header)) return -1;
	
	status = send(socket, name, strlen(name), MSG_NOSIGNAL);
	if(status < strlen(name)) return -1;
	
	return 0;
}

int Client::doGetPortUp()
{
	struct { uint16_t length; uint64_t channelUp; } reply;
	reply.length = sizeof(reply);
	reply.channelUp = frameServer->getPortUp();
	int status = 0;
	status = send(socket, &reply, sizeof(reply), MSG_NOSIGNAL);
	if (status < sizeof(reply)) return -1;
	
	return 0;
}

int Client::doGetPortCounts()
{
	uint16_t portID = 0;
	memcpy(&portID, socketBuffer + sizeof(CmdHeader_t), sizeof(uint16_t));
	
	
	struct { uint16_t length; uint64_t tx; uint64_t rx; uint64_t rxBad; } reply;
	reply.length = sizeof(reply);
	reply.tx = frameServer->getPortCounts(portID, 0);
	reply.rx = frameServer->getPortCounts(portID, 1);
	reply.rxBad = frameServer->getPortCounts(portID, 2);
	
	int status = 0;
	status = send(socket, &reply, sizeof(reply), MSG_NOSIGNAL);
	if (status < sizeof(reply)) return -1;
	
	return 0;
}

