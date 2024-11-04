#include <iostream>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// Handshake Constants
const std::string PROTOCOL_NAME = "BitTorrent protocol";
const int RESERVED_BYTES = 8;
const int HANDSHAKE_SIZE = 49 + PROTOCOL_NAME.size();

// Replace with actual info hash and peer ID
std::string infoHash = "<20-byte-info-hash00>";
std::string peerID = "<20-byte-peer-id00000>";

bool connectToPeer(const std::string& ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    sockaddr_in peerAddress;
    peerAddress.sin_family = AF_INET;
    peerAddress.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &peerAddress.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        close(sock);
        return false;
    }

    if (connect(sock, (struct sockaddr*)&peerAddress, sizeof(peerAddress)) < 0) {
        std::cerr << "Connection failed\n";
        close(sock);
        return false;
    }

    // Construct the handshake message
    std::string handshakeMessage;
    handshakeMessage += static_cast<char>(PROTOCOL_NAME.size()); // protocol length byte
    handshakeMessage += PROTOCOL_NAME;
    handshakeMessage += std::string(RESERVED_BYTES, '\0'); // reserved bytes
    handshakeMessage += infoHash;
    handshakeMessage += peerID;

    // Send the handshake message
    if (send(sock, handshakeMessage.c_str(), handshakeMessage.size(), 0) < 0) {
        std::cerr << "Failed to send handshake\n";
        close(sock);
        return false;
    }

    // Receive handshake response (should be the same length as sent)
    char response[HANDSHAKE_SIZE];
    int bytesReceived = recv(sock, response, HANDSHAKE_SIZE, 0);
    if (bytesReceived != HANDSHAKE_SIZE) {
        std::cerr << "Handshake failed or unexpected response\n";
        close(sock);
        return false;
    }

    std::cout << "Handshake successful with peer!\n";

    close(sock);
    return true;
}

int main() {
    std::string peerIP = "0.0.0.0"; // Replace with actual peer IP
    int peerPort = 6881;              // Replace with actual peer port

    if (connectToPeer(peerIP, peerPort)) {
        std::cout << "Connected and handshaked with peer.\n";
    } else {
        std::cout << "Failed to connect to peer.\n";
    }

    return 0;
}
