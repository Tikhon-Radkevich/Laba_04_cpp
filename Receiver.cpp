#include <fstream>
#include <iostream>
#include <windows.h>
#include <string>


using std::cout;
using std::cin;


void CreateSenderProcesses(const std::string& file_name, int number_of_senders, HANDLE* hEventStarted);

void HandleMessages(const std::string& file_name, HANDLE hInputReadySemaphore, HANDLE hOutputReadySemaphore, HANDLE hMutex);

int main() {
    std::string file_name;
    int number_of_notes;
    std::fstream file;
    int number_of_senders;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    cout << "Enter binary file name: ";
    cin >> file_name;
    cout << "Enter number of notes: ";
    cin >> number_of_notes;
    file.open(file_name, std::ios::out);
    file.close();

    cout << "Enter number of Sender Processes: ";
    cin >> number_of_senders;

    HANDLE hInputReadySemaphore = CreateSemaphore(NULL, 0, number_of_notes, "EnterSemaphoreStarted");
    if (hInputReadySemaphore == NULL)
        return GetLastError();
    HANDLE hOutputReadySemaphore = CreateSemaphore(NULL, 0, number_of_notes, "OutputSemaphoreStarted");
    if (hOutputReadySemaphore == NULL)
        return GetLastError();
    HANDLE hMutex = CreateMutex(NULL, 0, "mut ex");
    HANDLE* hEventStarted = new HANDLE[number_of_senders];

    CreateSenderProcesses(file_name, number_of_senders, hEventStarted);

    WaitForMultipleObjects(number_of_senders, hEventStarted, TRUE, INFINITE);
    ReleaseMutex(hMutex);

    HandleMessages(file_name, hInputReadySemaphore, hOutputReadySemaphore, hMutex);

    CloseHandle(hInputReadySemaphore);
    CloseHandle(hOutputReadySemaphore);
    for (int i = 0; i < number_of_senders; i++) {
        CloseHandle(hEventStarted[i]);
    }

    delete[] hEventStarted;

    return 0;
}

void CreateSenderProcesses(const std::string& file_name, int number_of_senders, HANDLE* hEventStarted) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    for (int i = 0; i < number_of_senders; ++i) {
        std::string sender_cmd = "Sender.exe " + file_name;
        LPSTR lpwstrSenderProcessCommandLine = const_cast<LPSTR>(sender_cmd.c_str());

        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        if (!CreateProcess(NULL, lpwstrSenderProcessCommandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            cout << "The Sender Process is not started.\n";
            exit(GetLastError());
        }
        hEventStarted[i] = CreateEvent(NULL, FALSE, FALSE, "StartPocess");
        if (hEventStarted[i] == NULL)
            exit(GetLastError());
        CloseHandle(pi.hProcess);
    }
}

void HandleMessages(const std::string& file_name, HANDLE hInputReadySemaphore, HANDLE hOutputReadySemaphore, HANDLE hMutex) {
    std::fstream file;
    cout << "\nEnter 1 to read message;\nEnter 0 to exit \n";
    int key;
    cin >> key;
    file.open(file_name, std::ios::in);

    while (true) {
        if (key == 1) {
            std::string message;
            WaitForSingleObject(hInputReadySemaphore, INFINITE);
            WaitForSingleObject(hMutex, INFINITE);
            std::getline(file, message);
            cout << message;
            ReleaseSemaphore(hOutputReadySemaphore, 1, NULL);
            ReleaseMutex(hMutex);
            cout << "\nEnter 1 to read message; \nEnter 0 to exit \n";
            cin >> key;
        }
        if (key != 0 && key != 1) {
            cout << "\nValue Error!\nEnter 1 to read message;\nEnter 0 to exit \n";
            cin >> key;
        }
        if (key == 0) {
            cout << "Process ended";
            break;
        }
    }
    file.close();
}
