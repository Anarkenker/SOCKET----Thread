#include "../tcpSocket/tcpSocket.h"
#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <ctime>
#include <random>

using namespace std;

vector<int> countNumbers;
map<int, vector<string>> foodInformation;
map<string, int> numbers;

int Random(int num)
{
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, num - 1);
    return distribution(generator);
}

void LoadFoodInformation(const string &filename)
{
    ifstream file(filename);
    int id;
    string name, ingredient;
    countNumbers.clear();
    while (file >> id >> name)
    {
        vector<string> ingredients;
        while (file.peek() != '\n' && file >> ingredient)
        {
            ingredients.push_back(ingredient);
        }
        foodInformation[id] = ingredients;
        countNumbers.push_back(id);
    }
}

void LoadNumbers(const string &filename)
{
    ifstream file(filename);
    string name;
    int quantity;
    while (file >> name >> quantity)
    {
        numbers[name] = quantity;
    }
}

string GenerateOrder(size_t num)
{
    string order;
    int count = Random(num);
    for (int i = 0; i <= count; i++)
    {
        int index = Random(num);
        if (i != 0)
            order += ",";
        order += to_string(countNumbers[index]);
    }
    return order;
}

void ThreadCustomer(int customerThreadID)
{
    SOCKET fd = CreatClientSocket("127.0.0.1");

    if (fd == INVALID_SOCKET)
    {
        err("Socket");
        return;
    }

    while (1)
    {
        char recvbuf[BUFSIZ] = {0};
        string order = GenerateOrder(countNumbers.size());
        printf("customerThreadID %d: Sending: %s\n", customerThreadID, order.c_str());

        if (SOCKET_ERROR == send(fd, order.c_str(), order.size(), 0))
        {
            err("send");
            closesocket(fd);
            return;
        }

        if (0 < recv(fd, recvbuf, BUFSIZ, 0))
        {
            printf("customerThreadID %d: Received %s\n", customerThreadID, recvbuf);
        }
        this_thread::sleep_for(chrono::milliseconds(Random(5) * 100));
    }

    closesocket(fd);
}

int main()
{
    if (!InitSocket())
    {
        printf("Failed!!\n");
        return -1;
    }

    LoadFoodInformation("food.txt");
    LoadNumbers("number.txt");

    deque<thread> clientThreads;

    for (int i = 1; i <= 10; i++)
    {
        clientThreads.emplace_back(ThreadCustomer, i);
    }

    for (auto &t : clientThreads)
    {
        t.join();
    }

    CloseSocket();
    printf("---ClientEnd---\n");
    return 0;
}