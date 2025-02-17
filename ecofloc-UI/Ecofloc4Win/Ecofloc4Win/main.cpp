#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <csignal>
#include "json.hpp"
#include <mutex>
#include <condition_variable>

using json = nlohmann::json;
using namespace std;

// Global variables
bool STOP_MONITORING = false;
std::mutex MUTEX;
std::condition_variable CV;

volatile bool stop = false;
mutex fileMutex;

void handleSignal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        STOP_MONITORING = true;
    }
}

string generateRandomColor() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, 255);
    char color[8];
    snprintf(color, sizeof(color), "#%02X%02X%02X", dist(gen), dist(gen), dist(gen));
    return string(color);
}

int generateRandomPower() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(10, 100);
    return dist(gen);
}

void updateJsonFile(const string& filename, int pid, bool updateCPU, bool updateGPU, bool updateNIC, bool updateSD) {
    lock_guard<mutex> lock(fileMutex);

    json data;
    ifstream inFile(filename);
    if (inFile) {
        inFile >> data;
        inFile.close();
    }
    else {
        data = json{ {"apps", json::array()}, {"time", time(nullptr)} };
    }

    bool pidExists = false;
    for (auto& app : data["apps"]) {
        if (app["pid"] == pid) {
            pidExists = true;
            if (updateCPU) app["power_w_CPU"] = generateRandomPower();
            if (updateGPU) app["power_w_GPU"] = generateRandomPower();
            if (updateNIC) app["power_w_NIC"] = generateRandomPower();
            if (updateSD) app["power_w_SD"] = generateRandomPower();
            break;
        }
    }

    if (!pidExists) {
        json newApp = {
            {"color", generateRandomColor()},
            {"pid", pid},
            {"power_w_CPU", updateCPU ? generateRandomPower() : 0},
            {"power_w_GPU", updateGPU ? generateRandomPower() : 0},
            {"power_w_NIC", updateNIC ? generateRandomPower() : 0},
            {"power_w_SD", updateSD ? generateRandomPower() : 0}
        };
        data["apps"].push_back(newApp);
    }

    data["time"] = time(nullptr);

    ofstream outFile(filename);
    outFile << data.dump(4);
    outFile.close();
}

// Function executed in monitoring thread
void monitoringThread(string filename, int pid, int interval, bool updateCPU, bool updateGPU, bool updateNIC, bool updateSD) {
    while (!STOP_MONITORING) {
        updateJsonFile(filename, pid, updateCPU, updateGPU, updateNIC, updateSD);
        this_thread::sleep_for(chrono::milliseconds(interval));
    }
}

int main(int argc, char* argv[]) {
    bool updateCPU = false, updateGPU = false, updateNIC = false, updateSD = false;
    int pid = 0, interval = 1000;
    string filename;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--cpu") updateCPU = true;
        else if (arg == "--gpu") updateGPU = true;
        else if (arg == "--nic") updateNIC = true;
        else if (arg == "--sd") updateSD = true;
        else if (arg == "-p" && i + 1 < argc) pid = stoi(argv[++i]);
        else if (arg == "-i" && i + 1 < argc) interval = stoi(argv[++i]);
        else if (arg == "-f" && i + 1 < argc) filename = argv[++i];
    }

    if (pid == 0 || filename.empty()) {
        cerr << "Erreur : PID ou fichier non spécifié." << endl;
        return 1;
    }

    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

    // Créer le thread sans le détacher
    thread daemon([=]() { 
        monitoringThread(filename, pid, interval, updateCPU, updateGPU, updateNIC, updateSD); 
    });

    // Attendre que le signal d'arrêt soit reçu
    while (!STOP_MONITORING) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    // Attendre que le thread se termine proprement
    if (daemon.joinable()) {
        daemon.join();
    }

    return 0;
}
