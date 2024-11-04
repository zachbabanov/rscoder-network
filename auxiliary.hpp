#ifndef AUXILIARY_H
#define AUXILIARY_H

#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>

#define MAXLINE 65000

#define ACK 0x01
#define FILE_NAME 0x02
#define FILE 0x03
#define EOT 0x04
#define BENCH 0x05

/*
 *   The structure of the header is assumed to be as follows, both at the receiver_point and at the sender_point:
 *
 *    0                   1                   2                   3                   4
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |    MSG TYPE    |   FILE INDEX  |          PACKET INDEX         | AMOUNT OF BLK  |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   MSG TYPES MAY BE: ACK, FILE NAME, FILE, END OF TRANSMISSION, BENCH { 0x01, 0x02, 0x03, 0x04, 0x05 } and so on
 *   When server starts and after a continuous delay ACK (0x01) packet with no other headers and data mus be sent.
 *   There may be no answer on that, or server can send same packet back.
 *   For the FILE (0x01) it must be FILE INDEX (hex) specified as well. When packet with FILE (0x03) type arrived on server
 *   there must be if/else check on FILE INDEX (hex), if index is same to the previous FILE (0x03) packet then stat written
 *   to the same .csv file. If packet with FILE (0x03) type arrived with another FILE INDEX (hex) and/or
 *   no EOT (0x04) type packet were sent, then new .csv file created and stat written there.
 *   EOT (0x04) type signals of end of previous file
 */

/// Composing an udp packet with the message/information itself. Presume that the ACK message and the FILE_NAME
/// message with the name of the message/file have already been transmitted and that the receiver point is ready
/// to receive and store the data
/// \param packet - Link/pointer to globally located memory area, where packet will be stored after composing
/// \param msgType - Flag of current message type. Must be FILE to transmit actual data
/// \param fileIndex - Index of current file relative to all other files sent
/// \param packetIndex - Index of current packet relative to all packets within that particular file transmission
/// \param amountOfBlock - Amount of encoded blocks stored after header in current packet
/// \param data - Link/pointer to actual data to read and compose into packet. Must be prepared to fit an actual
/// size of packet expected on receiver
/// \return Composed packet, same as stored at <b>packet</b>
char* composePacket(char* packet, const char &msgType, const char &fileIndex, const char16_t &packetIndex, const char &amountOfBlock, const char* data)
{
    memset(&packet[0], 0, strlen(data) + 5);

    char packetIndexLow = static_cast<char>(packetIndex);
    char packetIndexHigh = static_cast<char>(packetIndex >> 8);

    if (packetIndexHigh == 0)
        packetIndexHigh = -1;

    char amount = static_cast<char>(amountOfBlock);

    packet[0] = msgType;
    packet[1] = fileIndex;
    strncat(packet + 2, &packetIndexHigh, 1);
    strncat(packet + 3, &packetIndexLow, 1);
    strncat(packet + 4, &amount, 1);
    memcpy(packet + 5, data, 255 * amountOfBlock);

    return packet;
}

/// Composing an udp packet with operating data, such as ACK and EOT. Must be sent at start of work and after end of each file sent
/// \param packet - Link/pointer to globally located memory area, where packet will be stored after composing
/// \param msgType - Flag of current message type. Must be ACK, EOT to actually work.
/// \return Composed packet, same as stored at <b>packet</b>
char* composePacket(char* packet, const char &msgType)
{
    packet = new char[1];
    memset(&packet[0], 0, 1);
    packet[0] = msgType;

    return packet;
}

/// Composing an udp packet with operating data, such as FILE_NAME. Presume that the ACK message
/// have already been transmitted and that the receiver point is ready to receive another operating data
/// \param packet - Link/pointer to globally located memory area, where packet will be stored after composing
/// \param msgType - Flag of current message type. Must be FILE_NAME to prepare receiver to get a file
/// \param fileIndex - Index of current packet relative to all packets within that particular file transmission
/// \param data - Name of current file. Link/pointer or <b>char*</b> is accepted
/// \return Composed packet, same as stored at <b>packet</b>
char* composePacket(char* packet, const char &msgType, const char &fileIndex, const char* data)
{
    packet = new char[strlen(data) + 2];
    memset(&packet[0], 0, strlen(data) + 2);
    packet[0] = msgType;
    packet[1] = fileIndex;
    strcat(packet, data);

    return packet;
}

/// Composing a packet for benchmarking current network. No data need to be provided, because presumed that
/// packet was prepared with same data through all it length
/// \param packet - Link/pointer to globally located memory area, where packet will be stored after composing
/// \param msgType - Flag of current message type. Must be BENCH to prepare receiver to check benchmarking
/// \param packetIndex - Index of current packet relative to all packets within that particular benchmarking
/// \param bandwidth - Number of data blocks in single packet
/// \return Composed packet, same as stored at <b>packet</b>
char* composePacket(char* packet, const char &msgType, const char16_t &packetIndex, const int &bandwidth)
{
    packet = new char[255 * bandwidth + 6];
    memset(&packet[0], 1, 255 * bandwidth + 6);

    char packetIndexLow = static_cast<char>(packetIndex);
    char packetIndexHigh = static_cast<char>(packetIndex >> 8);

    if (packetIndexHigh == 0)
        packetIndexHigh = -1;

    packet[0] = msgType;
    packet[1] = bandwidth;
    memcpy(&packet[2], &packetIndexHigh, 1);
    memcpy(&packet[3], &packetIndexLow, 1);

    return packet;
}

/// Get current system time as hh:mm:ss.ms
/// \return String with system time
std::string getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm*
            time_info = std::localtime(&in_time_t);
    std::stringstream ss;

    ss << std::setfill('0') << std::setw(2) << time_info->tm_hour << ":"
       << std::setw(2) << time_info->tm_min << ":"
       << std::setw(2) << time_info->tm_sec << ".";

    auto fraction = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::time_point_cast<std::chrono::seconds>(now));
    ss << std::setw(3) << std::setfill('0') << fraction.count();

    return ss.str();
}

#endif //AUXILIARY_H
