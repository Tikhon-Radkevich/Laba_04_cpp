#include <fstream>
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>

int main() {
    std::string file_name;
    int number_of_notes;
    std::fstream file;
    int number_of_senders;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    LPWSTR lpwstrSenderProcessCommandLine;
    std::cout << "Input binary file name:\n";
    std::cin >> file_name;
    std::cout << "Input number of notes:\n";
    std::cin >> number_of_notes;
    file.open(file_name, std::ios::out);
    file.close();
   
    std::cout << "Input number of Sender Processes:\n";
    std::cin >> number_of_senders;

    HANDLE hInputReadySemaphore = CreateSemaphore(NULL, 0, number_of_notes, "Input Semaphore started");
    if (hInputReadySemaphore == NULL)
        return GetLastError();
    HANDLE hOutputReadySemaphore = CreateSemaphore(NULL, 0, number_of_notes, "Output Semaphore started");
    if (hOutputReadySemaphore == NULL)
        return GetLastError();
    HANDLE hMutex = CreateMutex(NULL, 0, "mut ex");
    HANDLE* hEventStarted = new HANDLE[number_of_senders];

    for (int i = 0; i < number_of_senders; ++i)
    {
        // std::string sender_cmd = "Sender.exe " + file_name;
        // std::wstring converting_sender_to_lpwstr = std::wstring(sender_cmd.begin(), sender_cmd.end());
        // lpwstrSenderProcessCommandLine = &converting_sender_to_lpwstr[0];

        std::string sender_cmd = "Sender.exe " + file_name;
        LPSTR lpwstrSenderProcessCommandLine = const_cast<LPSTR>(sender_cmd.c_str());

        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        if (!CreateProcess(NULL, lpwstrSenderProcessCommandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            std::cout << "The Sender Process is not started.\n";
            return GetLastError();
        }
        hEventStarted[i] = CreateEvent(NULL, FALSE, FALSE, "Process Started");
        if (hEventStarted[i] == NULL)
            return GetLastError();
        CloseHandle(pi.hProcess);
    }

    WaitForMultipleObjects(number_of_senders, hEventStarted, TRUE, INFINITE);
    ReleaseMutex(hMutex);
    std::cout << "\nInput 1 to read message;\nInput 0 to exit process\n";
    int key;
    int counter = 0;
    std::cin >> key;
    file.open(file_name, std::ios::in);
    while (true) {
        if (key == 1) {
            std::string message;
            WaitForSingleObject(hInputReadySemaphore, INFINITE);
            WaitForSingleObject(hMutex, INFINITE);
            std::getline(file, message);
            std::cout << message;
            ReleaseSemaphore(hOutputReadySemaphore, 1, NULL);
            ReleaseMutex(hMutex);
            //file.close();
            std::cout << "\nInput 1 to read message;\nInput 0 to exit process\n";
            std::cin >> key;
        }
        if (key != 0 && key != 1) {
            std::cout << "\nIncorrect value!\nInput 1 to read message;\nInput 0 to exit process\n";
            std::cin >> key;
        }
        if (key == 0) {
            std::cout << "Process ended.";
            break;
        }
    }
    CloseHandle(hInputReadySemaphore);
    CloseHandle(hOutputReadySemaphore);
    for (int i = 0; i < number_of_senders; i++)
    {
        CloseHandle(hEventStarted[i]);
    }

    return 0;
}