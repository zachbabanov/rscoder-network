#include <iostream>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "rscoder/gf.hpp"
#include "rscoder/poly.hpp"
#include "rscoder/rs.hpp"

#include "auxiliary.hpp"
#include "command.hpp"

int main(int argc, char *argv[])
{
    RS::ReedSolomon<128, 127> coder;

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <server_ip>" << std::endl;
        return -2;
    }

    int port = atoi(argv[1]);
    char *serverIP = argv[2];

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if(inet_pton(AF_INET, serverIP, &servaddr.sin_addr  ) <= 0)
    {
        std::cout << "Invalid address / Address not supported" << std::endl;
        return -3;
    }

    char* udpPacket;

    udpPacket = new char[0];
    udpPacket = composePacket(udpPacket, ACK);
    if (sendto(sockfd, (const char*)udpPacket, strlen(udpPacket),
               MSG_CONFIRM, (const struct sockaddr*)&servaddr,
               sizeof(servaddr)) < 0)
    {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }

    delete[] udpPacket;

    int amountOfFilesSent = 0x00;

    while (true)
    {
        char* input = readline("sender_point> ");

        if (!input)
        {
            break;
        }

        add_history(input);

        Command command = parseCommand(input);
        free(input);

        // I know what switch is. Yes. I do. But that is just the same in exact case.
        // And dont need to introduce another workaround like infinite enum just for iterating through commands
        // TODO: Think how to turn this ugly if/else into something not that ugly
        if (command.name == "bench")
        {
            std::string currentTime = getCurrentTimestamp();
            std::ofstream benchOutput("csv/" + currentTime + "_sender_bench_output.csv", std::ios::app);

            std::string bandwidth;
            if (hasFlag(command.args, "-b", bandwidth))
            {
                int integerBandwidth;
                try
                {
                    integerBandwidth = stoi(bandwidth);
                    if (integerBandwidth > 64 || integerBandwidth < 1)
                    {
                        std::cout << "Incorrect amount of blocks provided" << std::endl;
                        continue;
                    }
                }
                catch (std::invalid_argument const&)
                {
                    std::cout << "Incorrect amount of blocks provided" << std::endl;
                    continue;
                }

                std::string duration;
                if (hasFlag(command.args, "-n", duration))
                {
                    int integerDuration;
                    try
                    {
                        integerDuration = stoi(duration);
                        if (integerDuration < 1 || integerDuration > 100000)
                        {
                            std::cout << "Incorrect duration provided" << std::endl;
                            continue;
                        }
                    }
                    catch (std::invalid_argument const&)
                    {
                        std::cout << "Incorrect duration provided" << std::endl;
                        continue;
                    }

                    std::cout << "Sending benchmark packet with " << integerBandwidth << " block(s) for " << integerDuration << " times" << std::endl;

                    for (int currentPacket = 0; currentPacket < integerDuration; currentPacket++)
                    {
                        char* benchPacket = new char[255 * integerBandwidth + 4];
                        benchPacket = composePacket(benchPacket, BENCH, currentPacket + 1, integerBandwidth);

                        if (sendto(sockfd, (const char *)benchPacket,
                                   strlen(benchPacket), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                        {
                            perror("sendto failed");
                            exit(EXIT_FAILURE);
                        }

                        std::string timestamp = getCurrentTimestamp();
                        benchOutput << currentPacket + 1 << "," << integerBandwidth << "," << timestamp << std::endl;

                        delete[] benchPacket;
                    }

                    char* eotPacket = new char[1];
                    eotPacket = composePacket(eotPacket, EOT);

                    if (sendto(sockfd, (const char *)eotPacket,
                            strlen(eotPacket), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                    {
                        perror("sendto failed");
                        exit(EXIT_FAILURE);
                    }

                    delete[] eotPacket;
                }
                else
                {
                    std::cout << "Sending benchmark packet with " << integerBandwidth << " block(s) for default (2000) amount of times" << std::endl;

                    for (int currentPacket = 0; currentPacket < 2000; currentPacket++)
                    {
                        char* benchPacket = new char[255 * integerBandwidth + 4];
                        benchPacket = composePacket(benchPacket, BENCH, currentPacket + 1, integerBandwidth);

                        if (sendto(sockfd, (const char *)benchPacket,
                                   strlen(benchPacket), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                        {
                            perror("sendto failed");
                            exit(EXIT_FAILURE);
                        }

                        std::string timestamp = getCurrentTimestamp();
                        benchOutput << currentPacket + 1 << "," << integerBandwidth << "," << timestamp << std::endl;

                        delete[] benchPacket;
                    }

                    char* eotPacket = new char[1];
                    eotPacket = composePacket(eotPacket, EOT);

                    if (sendto(sockfd, (const char *)eotPacket,
                               strlen(eotPacket), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                    {
                        perror("sendto failed");
                        exit(EXIT_FAILURE);
                    }

                    delete[] eotPacket;
                }
            }
            else
            {
                std::cout << "Invalid benchmark command" << std::endl;
            }
        }
        else if (command.name == "send")
        {
            std::string currentTime = getCurrentTimestamp();
            std::ofstream sendOutput("csv/" + currentTime + "_sender_send_output.csv", std::ios::app);

            std::string filename;
            if (hasFlag(command.args, "-f", filename))
            {
                std::ifstream inputFileStream(filename);
                if (!inputFileStream.good())
                {
                    std::cout << "Bad file path" << std::endl;
                    inputFileStream.close();
                    continue;
                }

                std::string bandwidth;
                if(hasFlag(command.args, "-b", bandwidth))
                {
                    int integerBandwidth;
                    try
                    {
                        integerBandwidth = stoi(bandwidth);
                        if (integerBandwidth > 64 || integerBandwidth < 1)
                        {
                            std::cout << "Incorrect amount of blocks provided" << std::endl;
                            continue;
                        }
                    }
                    catch (std::invalid_argument const&)
                    {
                        std::cout << "Incorrect amount of blocks provided" << std::endl;
                        continue;
                    }

                    amountOfFilesSent++;

                    std::cout << "Sending file: " << filename << " with bandwidth: " << bandwidth << std::endl;

                    size_t pos = filename.rfind('/');
                    std::string nameOfFile =  filename.substr(pos + 1);

                    udpPacket = new char[nameOfFile.size() + 2];
                    udpPacket = composePacket(udpPacket, FILE_NAME, amountOfFilesSent, (char*)nameOfFile.c_str());

                    if (sendto(sockfd, (const char *)udpPacket,
                               strlen(udpPacket), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                    {
                        perror("sendto failed");
                        exit(EXIT_FAILURE);
                    }

                    delete[] udpPacket;

                    std::ifstream inputFile(filename, std::ios::in | std::ios::out | std::ios::binary);

                    /// BEGIN OF FILE TRANFER

                    inputFile.seekg(0, inputFile.end);
                    std::streampos inputFileSize = inputFile.tellg();
                    inputFile.seekg(0, inputFile.beg);

                    int numberOfChunks = inputFileSize / 128 + 1;
                    char* inputFileBytes = new char[numberOfChunks * 128];
                    memset(inputFileBytes, 0, numberOfChunks * 128);

                    inputFile.read(inputFileBytes, inputFileSize);
                    inputFileBytes[inputFileSize] = '\0';
                    inputFile.close();

                    char currentMSG[128];
                    char encodedMSG[255];

                    if (integerBandwidth > 1)
                    {
                        char encodedSequence[255 * integerBandwidth];
                        memset(&encodedSequence[0], 0, 255 * integerBandwidth);

                        for (int currentChunk = 0; currentChunk < numberOfChunks; currentChunk += integerBandwidth)
                        {
                            for (int currentPart = 0; currentPart < integerBandwidth; currentPart++)
                            {
                                memcpy(currentMSG, inputFileBytes + currentChunk * 128 + currentPart * 128, 128);
                                coder.Encode(currentMSG, encodedMSG);
                                memcpy(encodedSequence + 255 * currentPart, encodedMSG, 255);

                                memset(&currentMSG[0], 0, 128);
                                memset(&encodedMSG[0], 0, 255);
                            }

                            udpPacket = new char[255 * integerBandwidth + 5];
                            char16_t currentIndex = currentChunk / integerBandwidth + 1;
                            udpPacket = composePacket(udpPacket, FILE, amountOfFilesSent, currentIndex,
                                                      integerBandwidth, encodedSequence);

                            memset(encodedSequence, 0, 255 * integerBandwidth);

                            if (sendto(sockfd,
                                    (const char *)udpPacket, (255 * integerBandwidth + 5) * sizeof(char),
                                    0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                            {
                                perror("sendto failed");
                                exit(EXIT_FAILURE);
                            }

                            std::string timestamp = getCurrentTimestamp();
                            sendOutput << amountOfFilesSent << "," << currentIndex << "," << integerBandwidth << "," << timestamp << std::endl;

                            delete[] udpPacket;
                        }
                    }
                    else
                    {
                        udpPacket = new char[255 + 5];
                        memset(&udpPacket[0], 0, 255 + 5);
                        for (int currentChunk = 0; currentChunk < numberOfChunks; currentChunk++)
                        {
                            memcpy(currentMSG, inputFileBytes + currentChunk * 128, 128);
                            coder.Encode(currentMSG, encodedMSG);
                            memset(&currentMSG[0], 0, 128);


                            udpPacket = composePacket(udpPacket, FILE, amountOfFilesSent, currentChunk + 1, 1, encodedMSG);

                            memset(encodedMSG, 0, 255);

                            if (sendto(sockfd,
                                       (const char *)udpPacket, (255 + 5) * sizeof(char),
                                       0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                            {
                                perror("sendto failed");
                                exit(EXIT_FAILURE);
                            }

                            std::string timestamp = getCurrentTimestamp();
                            sendOutput << amountOfFilesSent << "," << currentChunk + 1 << "," << integerBandwidth << "," << timestamp << std::endl;

                            memset(&udpPacket[0], 0, 255 + 5);
                        }
                    }

                    /// END OF FILE TRANSFER

                    udpPacket = new char[sizeof(EOT)];
                    udpPacket = composePacket(udpPacket, EOT);
                    if (sendto(sockfd,
                               (const char *)udpPacket, strlen(udpPacket),
                               0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                    {
                        perror("sendto failed");
                        exit(EXIT_FAILURE);
                    }

                    delete[] udpPacket;
                    inputFileStream.close();
                }
                else
                {
                    std::cout << "Invalid send command" << std::endl;
                }
            }
            else
            {
                std::cout << "Invalid send command" << std::endl;
            }
        }
        else if (command.name == "quit" || command.name == "exit" || command.name == "q")
        {
            break;
        }
        else
        {
            std::cout << "Unknown command" << std::endl;
        }
    }

    return 1;
}
