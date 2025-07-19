
#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
//#include <string>
//#include <mutex>
//#include <iostream>
//#include "../xrCore/log.h"

#pragma comment(lib, "Ws2_32.lib")
class ChezzeClient {
    public:
        static ChezzeClient& Instance() {
            static ChezzeClient instance;
            return instance;
        }

        bool Connect(const xr_string& ip, int port) {
            if (fatal) return false;
            if (connected) return true;

            sockaddr_in serverAddr{};
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

            sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock == INVALID_SOCKET) {
                //Msg("ERROR socket()\n");
                return false;
            }

            if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                //Msg("ERROR connect()\n");
                closesocket(sock);
                sock = INVALID_SOCKET;
                return false;
            }

            connected = true;
            return true;
        }

        bool Send(const std::string& data) {
            if (!connected || fatal) return false;
            return send(sock, data.c_str(), static_cast<int>(data.size()), 0) != SOCKET_ERROR;
        }
        void SplashInfo(const std::string& persent, const std::string& info) {
            Send("spl_start");
            auto rec = Receive();
            if (rec != "OK")
                return;
            Send(persent);
            rec = Receive();
            if (rec != "OK")
                return;
            Send(info);
            rec = Receive();
        }
        xr_string Receive(int bufferSize = 1024) {
            if (!connected) return "";

            char* buffer = new char[bufferSize];
            memset(buffer, 0, bufferSize);

            int bytesReceived = recv(sock, buffer, bufferSize, 0);
            xr_string result;
            if (bytesReceived > 0) {
                result.assign(buffer, bytesReceived);
            }

            delete[] buffer;
            return result;
        }

        void Disconnect() {
            if (connected) {
                closesocket(sock);
                sock = INVALID_SOCKET;
                connected = false;
            }
        }

        ~ChezzeClient() {
            Disconnect();
            WSACleanup();
        }

    private:
        SOCKET sock = INVALID_SOCKET;
        bool connected = false;
        bool fatal = false;
        ChezzeClient() {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                //Msg("Error WSAStartup\n");
                fatal = true;
            }
        }

        ChezzeClient(const ChezzeClient&) = delete;
        ChezzeClient& operator=(const ChezzeClient&) = delete;
    };
