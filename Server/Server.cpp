#include "../tcpSocket/tcpSocket.h"
#include <bits/stdc++.h>
#include <fstream>
#include <ctime>
#include <mutex>
#include <thread>
using namespace std;

map<int, vector<string>> foodInformation; 
map<string, int> numbers;            
mutex mtx;

void Log(const vector<int>& IDs, const vector<string>& results) {
    ofstream log("log.txt", ios::app);
    time_t now = time(0);
    tm* ltm = localtime(&now);
    log << ltm->tm_hour << "-" << ltm->tm_min << "-" << ltm->tm_sec << " ";
    for (size_t i = 0; i < IDs.size(); ++i) {
        log << "ID: " << IDs[i] << " Result: " << results[i] << " ";
    }
    log << "numbers[";
    for (const auto& [ingredient, quantity] : numbers) {
        log << ingredient << " " << quantity << "; ";
    }
    log << "]\n";
}

void LoadFoodInformation(const string& filename) {
    ifstream file(filename);
    int id;
    string name, ingredient;
    while (file >> id >> name) {
        vector<string> ingredients;
        while (file.peek() != '\n' && file >> ingredient) {
            ingredients.push_back(ingredient);
        }
        foodInformation[id] = ingredients;
    }
}

void LoadNumbers(const string& filename) {
    ifstream file(filename);
    string name;
    int quantity;
    while (file >> name >> quantity) {
        numbers[name] = quantity;
    }
}

void SaveNumbers(const string& filename) {
    lock_guard<mutex> lock(mtx);
    ofstream file(filename);
    for (const auto& [ingredient, quantity] : numbers) {
        file << ingredient << " " << quantity << "\n";
    }
}

string Order(int ID) {
    lock_guard<mutex> lock(mtx);

    for (const auto& ingredient : foodInformation[ID]) {
        if (numbers[ingredient] <= 0) {
            return "-1";
        }
    }

    for (const auto& ingredient : foodInformation[ID]) {
        numbers[ingredient]--;
    }

    return "1";
}

string HandleRequest(const string& request) {
    size_t start = 0;
    size_t end = 0;
    string response;

    vector<int> IDs;        
    vector<string> results;    

    while (request.find(',', start) != string::npos) {
        end = request.find(',', start);
        string foodID = request.substr(start, end - start);
        int id = stoi(foodID);
        string result = Order(id);

        IDs.push_back(id);
        results.push_back(result);

        if (!response.empty()) response += ",";
        response += result;

        start = end + 1;
    }

    string foodID = request.substr(start);
    int id = stoi(foodID);
    string result = Order(id);

    IDs.push_back(id);
    results.push_back(result);

    if (!response.empty()) response += ",";
    response += result;

    Log(IDs, results);

    return response;
}

void HandleClient(SOCKET clientSocket) {
    char buf[BUFSIZ] = {0};

    while (recv(clientSocket, buf, BUFSIZ, 0) > 0) {
        printf("Received: %s\n", buf);

        string response = HandleRequest(buf);
        send(clientSocket, response.c_str(), response.size(), 0);
        SaveNumbers("number.txt");
    }

    closesocket(clientSocket);
}

int main() {
    InitSocket();
    LoadFoodInformation("food.txt");
    LoadNumbers("number.txt");

    SOCKET serverSocket = CreatServerSocket();
    printf("Server is running...\n");

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            err("accept");
            continue;
        }

        thread clientThread(HandleClient, clientSocket);
        clientThread.detach();
    }

    closesocket(serverSocket);
    CloseSocket();
    printf("---ServerEnd---\n");
    return 0;
}