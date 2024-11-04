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
const int MESSAGE_ID_REQUEST = 6;
const int MESSAGE_ID_PIECE = 7;

std::string infoHash = "<20-byte-info-hash0>";
std::string peerID = "<20-byte-peer-id000>";

bool requestPiece(int sock, int index, int begin, int length) {
    uint32_t messageLength = htonl(13); // 13 bytes for the payload
    uint8_t messageID = MESSAGE_ID_REQUEST;

    char buffer[17];
    memcpy(buffer, &messageLength, 4); // Length prefix
    buffer[4] = messageID;             // Message ID
    memcpy(buffer + 5, &htonl(index), 4); // Piece index
    memcpy(buffer + 9, &htonl(begin), 4); // Offset within the piece
    memcpy(buffer + 13, &htonl(length), 4); // Request length

    if (send(sock, buffer, 17, 0) < 0) {
        std::cerr << "Failed to send piece request\n";
        return false;
    }
    std::cout << "Piece request sent for index " << index << ", offset " << begin << ", length " << length << "\n";
    return true;
}

bool receivePiece(int sock, int& index, int& begin, std::vector<uint8_t>& block) {
    uint32_t lengthPrefix;
    if (recv(sock, &lengthPrefix, sizeof(lengthPrefix), 0) <= 0) {
        std::cerr << "Failed to receive piece message length\n";
        return false;
    }
    lengthPrefix = ntohl(lengthPrefix);

    uint8_t messageID;
    if (recv(sock, &messageID, sizeof(messageID), 0) <= 0 || messageID != MESSAGE_ID_PIECE) {
        std::cerr << "Failed to receive piece message ID or unexpected message\n";
        return false;
    }

    uint32_t pieceIndex, pieceBegin;
    recv(sock, &pieceIndex, 4, 0); // Piece index
    recv(sock, &pieceBegin, 4, 0); // Piece begin offset
    pieceIndex = ntohl(pieceIndex);
    pieceBegin = ntohl(pieceBegin);

    int blockSize = lengthPrefix - 9; // Remaining bytes are the piece data
    block.resize(blockSize);

    if (recv(sock, block.data(), blockSize, 0) < blockSize) {
        std::cerr << "Failed to receive entire piece block\n";
        return false;
    }

    std::cout << "Received piece data for index " << pieceIndex << " from offset " << pieceBegin << " with size " << blockSize << "\n";
    index = pieceIndex;
    begin = pieceBegin;
    return true;
}


bool sendInterestedMessage(int sock) {
    uint32_t messageLength = htonl(1); // 1 byte for the message ID
    uint8_t messageID = MESSAGE_ID_INTERESTED;

    char buffer[5];
    memcpy(buffer, &messageLength, 4); // Length prefix
    buffer[4] = messageID;

    if (send(sock, buffer, 5, 0) < 0) {
        std::cerr << "Failed to send interested message\n";
        return false;
    }
    std::cout << "Interested message sent\n";
    return true;
}

bool receiveUnchokeMessage(int sock) {
    uint32_t lengthPrefix;
    if (recv(sock, &lengthPrefix, sizeof(lengthPrefix), 0) <= 0) {
        std::cerr << "Failed to receive message length\n";
        return false;
    }
    lengthPrefix = ntohl(lengthPrefix);

    if (lengthPrefix != 1) {
        std::cerr << "Unexpected message length\n";
        return false;
    }

    uint8_t messageID;
    if (recv(sock, &messageID, sizeof(messageID), 0) <= 0) {
        std::cerr << "Failed to receive message ID\n";
        return false;
    }

    if (messageID == MESSAGE_ID_UNCHOKE) {
        std::cout << "Received unchoke message from peer\n";
        return true;
    } else {
        std::cerr << "Received unexpected message ID: " << (int)messageID << "\n";
        return false;
    }
}

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

    // Construct handshake
    std::string handshakeMessage;
    handshakeMessage += static_cast<char>(PROTOCOL_NAME.size());
    handshakeMessage += PROTOCOL_NAME;
    handshakeMessage += std::string(RESERVED_BYTES, '\0');
    handshakeMessage += infoHash;
    handshakeMessage += peerID;

    // Send handshake message
    if (send(sock, handshakeMessage.c_str(), handshakeMessage.size(), 0) < 0) {
        std::cerr << "Failed to send handshake\n";
        close(sock);
        return false;
    }
    std::cout << "Handshake sent\n";

    // Receive handshake response
    char response[HANDSHAKE_SIZE];
    int bytesReceived = recv(sock, response, HANDSHAKE_SIZE, 0);
    if (bytesReceived != HANDSHAKE_SIZE) {
        std::cerr << "Handshake failed or unexpected response\n";
        close(sock);
        return false;
    }
    std::cout << "Handshake successful with peer!\n";

    // Send interested message
    if (sendInterestedMessage(sock)) {
        std::cout << "Client sent interested message successfully.\n";
    }

    // Wait for unchoke message
    if (receiveUnchokeMessage(sock)) {
        std::cout << "Unchoke message received, ready to request pieces.\n";
    } else {
        std::cerr << "Did not receive unchoke message\n";
    }

    if (receiveUnchokeMessage(sock)) {
        int pieceIndex = 0; // example piece index to request
        int begin = 0; // start at the beginning of the piece
        int blockSize = 16 * 1024; // request 16 KB

        if (requestPiece(sock, pieceIndex, begin, blockSize)) {
            std::vector<uint8_t> pieceData;
            int receivedIndex, receivedBegin;
            if (receivePiece(sock, receivedIndex, receivedBegin, pieceData)) {
                std::cout << "Piece received successfully.\n";
            }
        }
    }

    close(sock);
    return true;
}

int main() {
    std::string peerIP = "127.0.0.1";
    int peerPort = 6881;

    if (connectToPeer(peerIP, peerPort)) {
        std::cout << "Connected and interacted with peer.\n";
    } else {
        std::cerr << "Failed to connect to peer.\n";
    }

    return 0;
}
