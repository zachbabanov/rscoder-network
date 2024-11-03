#include <iostream>
#include <fstream>
#include <cstdlib>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "rscoder/gf.hpp"
#include "rscoder/poly.hpp"
#include "rscoder/rs.hpp"

#include "auxiliary.hpp"

int main(int argc, char *argv[])
{
    RS::ReedSolomon<128, 127> decoder;

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return -2;
    }

    int port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    char hostname[1024]; ///< This machine hostname (either global DNS, or local name)
    gethostname(hostname, sizeof(hostname));
    struct hostent *he = gethostbyname(hostname); ///< Get all ip associated with hostname
    if (he == NULL)
    {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server IP: " << inet_ntoa(*(struct in_addr *)he->h_addr_list[0]) << std::endl;

    char buffer[MAXLINE] = {0};

    std::string currentTime = getCurrentTimestamp();
    std::ofstream outputFile;
    std::ofstream benchFile;

    int currentFileIndex = -1;

/*
    // Add vector to store all file names by their index and switch between them when getting new file or data from another file
    // May be use tuple, to store client addresses as well, to be able to get files with same index from different clients
*/

    while (true)
    {
        auto len = sizeof(cliaddr);
        int numberOfBytesReceived = recvfrom(sockfd, (char *)&buffer, MAXLINE,
                                             MSG_WAITALL, (struct sockaddr *)&cliaddr,
                         (socklen_t*)&len);

        buffer[numberOfBytesReceived] = '\0';

        char* clientFileName;
        std::ofstream fileFromClient;

        if (buffer[0] == 0x01)
        {

        }
        else if (buffer[0] == 0x02)
        {

            if (currentFileIndex == buffer[1])
            {
                fileFromClient.open(clientFileName);
            }
            else
            {
                clientFileName = new char[numberOfBytesReceived + 6];
                memset(&clientFileName[0], 0, strlen(clientFileName));
                strcat(clientFileName, "files/");
                strcat(clientFileName, buffer + 2);
                fileFromClient.open(clientFileName);
            }

            currentFileIndex = buffer[1];

            std::cout << "Received file name from " << ntohs(cliaddr.sin_port) << ": " << clientFileName << std::endl;
        }
        else if (buffer[0] == 0x03)
        {
            std::cout << "Received file packet from " << ntohs(cliaddr.sin_port) << std::endl;

            if (outputFile.is_open())
            {
                long pos = outputFile.tellp();
                if (pos == 0)
                {
                    currentTime = getCurrentTimestamp();
                    outputFile.open("csv/" + currentTime + "_receiver_send_output.csv", std::ios::app);
                }
            }
            else
            {
                currentTime = getCurrentTimestamp();
                outputFile.open("csv/" + currentTime + "_receiver_send_output.csv", std::ios::app);
            }

            char16_t currentPacketIndex;
            if (buffer[2] == '\377')
                currentPacketIndex = (unsigned char)buffer[3];
            else
                currentPacketIndex = static_cast<char16_t>((unsigned char)(buffer[2]) << 8 | (unsigned char)buffer[3]);

            // Check if file is the same

            if (buffer[1] == currentFileIndex)
            {
                if (buffer[4] > 1)
                {
                    int bandwidth = buffer[4];
                    char decodedMSG[128];

                    int numberOfErrors = 0;
                    for (int currentMSG = 0; currentMSG < bandwidth; currentMSG++)
                    {
                        int currentOffset = currentMSG * 255 + 5;
                        char MSG[255];
                        memcpy(&MSG[0], &buffer[currentOffset], 255);

                        decoder.Decode(MSG, decodedMSG);
                        numberOfErrors += decoder.polynoms[3].length;


                        fileFromClient.open(clientFileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);

                        if (fileFromClient.is_open())
                            fileFromClient.write(decodedMSG, 128);

                        memset(&decodedMSG[0], 0, 128);
                    }

                    std::string timestamp = getCurrentTimestamp();
                    outputFile << (int)currentPacketIndex << "," << int(bandwidth) << "," << timestamp << "," << numberOfErrors << std::endl;
                }
                else
                {
                    char decodedMSG[128];
                    char MSG[255];
                    memcpy(&MSG[0], &buffer[5], 255);

                    decoder.Decode(MSG, decodedMSG);
                    int numberOfErrors = decoder.polynoms[3].length;

                    fileFromClient.open(clientFileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);

                    if (fileFromClient.is_open())
                        fileFromClient.write(decodedMSG, 128);

                    std::string timestamp = getCurrentTimestamp();
                    outputFile << (int)currentPacketIndex << "," << 1 << "," << timestamp << "," << numberOfErrors << std::endl;
                }
            }
            else
            {
                if (buffer[4] > 1)
                {
                    int bandwidth = buffer[4];
                    char decodedMSG[128];

                    int numberOfErrors = 0;
                    for (int currentMSG = 0; currentMSG < bandwidth; currentMSG++)
                    {
                        int currentOffset = currentMSG * 255 + 5;
                        char MSG[255];
                        memcpy(&MSG[0], &buffer[currentOffset], 255);

                        decoder.Decode(MSG, decodedMSG);
                        numberOfErrors += decoder.polynoms[3].length;

                        fileFromClient.open(clientFileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);

                        if (fileFromClient.is_open())
                            fileFromClient.write(decodedMSG, 128);

                        memset(&decodedMSG[0], 0, 128);
                    }

                    std::string timestamp = getCurrentTimestamp();
                    outputFile << (int)currentPacketIndex << "," << int(bandwidth) << "," << timestamp << "," << numberOfErrors << std::endl;
                }
                else
                {
                    char decodedMSG[128];
                    char MSG[255];
                    memcpy(&MSG[0], &buffer[5], 255);

                    decoder.Decode(MSG, decodedMSG);
                    int numberOfErrors = decoder.polynoms[3].length;

                    fileFromClient.open(clientFileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);

                    if (fileFromClient.is_open())
                        fileFromClient.write(decodedMSG, 128);

                    std::string timestamp = getCurrentTimestamp();
                    outputFile << (int)currentPacketIndex << "," << 1 << "," << timestamp << "," << numberOfErrors << std::endl;
                }
            }
        }
        else if (buffer[0] == 0x04)
        {
            if (fileFromClient.is_open())
                fileFromClient.close();

            if (benchFile.is_open())
                benchFile.close();

            if (outputFile.is_open())
                outputFile.close();

            std::cout << "End of file packets from " << ntohs(cliaddr.sin_port) << std::endl;
        }
        else if (buffer[0] == 0x05)
        {
            std::cout << "Received benchmarking packet from " << ntohs(cliaddr.sin_port) << std::endl;

            int numberOfErrors = 0;
            for (int currentByte = 3; currentByte < numberOfBytesReceived; currentByte++)
            {
                if (buffer[currentByte] != 1)
                    numberOfErrors++;
            }

            if (benchFile.is_open())
            {
                long pos = benchFile.tellp();
                if (pos == 0)
                {
                    currentTime = getCurrentTimestamp();
                    benchFile.open("csv/" + currentTime + "_receiver_bench_output.csv", std::ios::app);
                }
            }
            else
            {
                currentTime = getCurrentTimestamp();
                benchFile.open("csv/" + currentTime + "_receiver_bench_output.csv", std::ios::app);
            }

            char16_t currentPacketIndex;
            if (buffer[2] == '\377')
                currentPacketIndex = (unsigned char)buffer[3];
            else
                currentPacketIndex = static_cast<char16_t>(((unsigned char)buffer[2]) << 8 | (unsigned char)buffer[3]);

            std::string timestamp = getCurrentTimestamp();
            benchFile << (int)currentPacketIndex << "," << (int)(unsigned char)buffer[1] << "," << timestamp << numberOfErrors << std::endl;
        }
        else
        {
            std::cout << "Wrong message type or broken message" << std:: endl;
        }

        memset(&buffer[0], 0, sizeof(buffer));
    }

    outputFile.close();
    return 0;
}
