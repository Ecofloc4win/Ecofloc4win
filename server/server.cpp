#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

// Même structure que dans l'application principale
#pragma pack(push, 1)
struct SharedEnergyData {
    double cpuEnergy;
    double gpuEnergy;
    double sdEnergy;
    double nicEnergy;
    long long timestamp;
};
#pragma pack(pop)

const char* response_template = "HTTP/1.1 200 OK\r\n"
"Content-Type: application/json\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Connection: close\r\n"
"\r\n"
"{\"cpu_energy\":%.2f,\"gpu_energy\":%.2f,\"sd_energy\":%.2f,\"nic_energy\":%.2f,\"timestamp\":%lld}";

int main() {
    // Initialiser Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed" << std::endl;
        return 1;
    }

    // Ouvrir la mémoire partagée
    HANDLE hMapFile = OpenFileMappingA(
        FILE_MAP_READ,
        FALSE,
        "Local\\EcoFlocEnergy");

    if (hMapFile == NULL) {
        std::cout << "Impossible d'ouvrir la memoire partagee. Assurez-vous que l'application principale est en cours d'execution." << std::endl;
        WSACleanup();
        return 1;
    }

    SharedEnergyData* pData = (SharedEnergyData*)MapViewOfFile(
        hMapFile,
        FILE_MAP_READ,
        0,
        0,
        sizeof(SharedEnergyData));

    if (pData == NULL) {
        std::cout << "Erreur mapping memoire" << std::endl;
        CloseHandle(hMapFile);
        WSACleanup();
        return 1;
    }

    // Créer le socket
    SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "Erreur creation socket" << std::endl;
        UnmapViewOfFile(pData);
        CloseHandle(hMapFile);
        WSACleanup();
        return 1;
    }

    // Configurer l'adresse
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPton(AF_INET, L"127.0.0.1", &service.sin_addr);
    service.sin_port = htons(13000);

    // Bind
    if (bind(ListenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "Bind failed" << std::endl;
        closesocket(ListenSocket);
        UnmapViewOfFile(pData);
        CloseHandle(hMapFile);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Listen failed" << std::endl;
        closesocket(ListenSocket);
        UnmapViewOfFile(pData);
        CloseHandle(hMapFile);
        WSACleanup();
        return 1;
    }

    std::cout << "Serveur web demarre sur http://localhost:13000" << std::endl;
    std::cout << "Ouvrez index.html dans votre navigateur pour voir les donnees" << std::endl;

    while (true) {
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            continue;
        }

        char buffer[1024];
        recv(ClientSocket, buffer, sizeof(buffer), 0);

        char response[1024];
        sprintf_s(response, sizeof(response), response_template,
            pData->cpuEnergy, pData->gpuEnergy, pData->sdEnergy, pData->nicEnergy, pData->timestamp);

        send(ClientSocket, response, strlen(response), 0);
        closesocket(ClientSocket);
    }

    // Nettoyage (ne sera jamais atteint dans cette version simple)
    UnmapViewOfFile(pData);
    CloseHandle(hMapFile);
    WSACleanup();
    return 0;
}
