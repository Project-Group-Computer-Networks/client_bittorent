#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

const std::string PROTOCOL_NAME = "BitTorrent protocol";
const int RESERVED_BYTES = 8;
const int HANDSHAKE_SIZE = 49 + PROTOCOL_NAME.size();
const int MESSAGE_ID_INTERESTED = 2;
const int MESSAGE_ID_UNCHOKE = 1;

std::string infoHash = "<20-byte-info-hash0>";
std::string peerID = "<20-byte-peer-id000>";

bool sendUnchokeMessage(int clientSock) {
    uint32_t messageLength = htonl(1); // 1 byte for the message ID
    uint8_t messageID = MESSAGE_ID_UNCHOKE;

    char buffer[5];
    memcpy(buffer, &messageLength, 4); // Length prefix
    buffer[4] = messageID;

    if (send(clientSock, buffer, 5, 0) < 0) {
        std::cerr << "Failed to send unchoke message\n";
        return false;
    }
    std::cout << "Unchoke message sent to client\n";
    return true;
}

void handleMessages(int clientSock) {
    while (true) {
        uint32_t lengthPrefix;
        if (recv(clientSock, &lengthPrefix, sizeof(lengthPrefix), 0) <= 0) {
            std::cerr << "Connection closed or error occurred\n";
            break;
        }
        lengthPrefix = ntohl(lengthPrefix);

        if (lengthPrefix == 0) {
            std::cout << "Keep-alive message received\n";
            continue;
        }

        uint8_t messageID;
        if (recv(clientSock, &messageID, sizeof(messageID), 0) <= 0) {
            std::cerr << "Failed to read message ID\n";
            break;
        }

        if (messageID == MESSAGE_ID_INTERESTED) {
            std::cout << "Received interested message from client\n";
            sendUnchokeMessage(clientSock);
        } else {
            std::cout << "Received message ID: " << (int)messageID << "\n";
        }
    }
}

void startServer(int port) {
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        std::cerr << "Failed to create server socket\n";
        return;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverSock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Binding failed\n";
        close(serverSock);
        return;
    }

    if (listen(serverSock, 1) < 0) {
        std::cerr << "Listening failed\n";
        close(serverSock);
        return;
    }

    std::cout << "Server is listening on port " << port << "...\n";

    sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    int clientSock = accept(serverSock, (struct sockaddr*)&clientAddress, &clientAddressLen);
    if (clientSock < 0) {
        std::cerr << "Failed to accept connection\n";
        close(serverSock);
        return;
    }

    std::cout << "Accepted connection from client\n";

    char handshakeMessage[HANDSHAKE_SIZE];
    int bytesReceived = recv(clientSock, handshakeMessage, HANDSHAKE_SIZE, 0);
    if (bytesReceived != HANDSHAKE_SIZE) {
        std::cerr << "Failed to receive proper handshake\n";
        close(clientSock);
        close(serverSock);
        return;
    }

    std::cout << "Received handshake from client\n";

    std::string response;
    response += static_cast<char>(PROTOCOL_NAME.size());
    response += PROTOCOL_NAME;
    response += std::string(RESERVED_BYTES, '\0');
    response += infoHash;
    response += peerID;

    if (send(clientSock, response.c_str(), response.size(), 0) < 0) {
        std::cerr << "Failed to send handshake response\n";
    } else {
        std::cout << "Handshake response sent to client\n";
    }

    handleMessages(clientSock);
    close(clientSock);
    close(serverSock);
}

int main() {
    int serverPort = 6881;
    startServer(serverPort);
    return 0;
}
