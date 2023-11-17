#include <fstream>
#include <windows.h>
#include <conio.h>
#include <iostream>


using std::cout;
using std::cin;



void processMessages(const std::string& file_name, HANDLE hStartEvent, HANDLE hInputReadySemaphore, HANDLE hOutputReadySemaphore, HANDLE hMutex);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <file_name>\n";
        return 1;
    }

    std::string file_name = argv[1];
    std::fstream file;
    HANDLE hStartEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "StartPocess");

    if (hStartEvent == NULL) {
        cout << "Open event failed. \nPress any key to exit.\n";
        cin.get();
        return GetLastError();
    }

    HANDLE hInputReadySemaphore = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, "EnterSemaphoreStarted");
    if (hInputReadySemaphore == NULL)
        return GetLastError();

    HANDLE hOutputReadySemaphore = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, "OutputSemaphoreStarted");
    if (hOutputReadySemaphore == NULL)
        return GetLastError();

    HANDLE hMutex = OpenMutex(SYNCHRONIZE, FALSE, "mut ex");

    SetEvent(hStartEvent);
    cout << "Event was started\n";

    processMessages(file_name, hStartEvent, hInputReadySemaphore, hOutputReadySemaphore, hMutex);

    CloseHandle(hInputReadySemaphore);
    CloseHandle(hOutputReadySemaphore);
    CloseHandle(hMutex);

    return 0;
}

void processMessages(const std::string& file_name, HANDLE hStartEvent, HANDLE hInputReadySemaphore, HANDLE hOutputReadySemaphore, HANDLE hMutex) {
    std::fstream file;
    cout << "Print 1 to write message;\nInput 0 to exit process\n";
    std::string key;
    cin >> key;

    while (true) {
        if (key == "1") {
            WaitForSingleObject(hMutex, INFINITE);
            file.open(file_name, std::ios::out | std::ios::app);

            std::string msg;
            cout << "Type in message: ";
            cin >> msg;

            char message[21];
            for (int i = 0; i < msg.length(); i++) {
                message[i] = msg[i];
            }

            for (int i = msg.length(); i < 21; i++) {
                message[i] = '\0';
            }

            message[20] = '\n';
            ReleaseMutex(hMutex);
            ReleaseSemaphore(hOutputReadySemaphore, 1, NULL);

            if (ReleaseSemaphore(hInputReadySemaphore, 1, NULL) != 1) {
                cout << "you have exceeded the number of messages!";
                ReleaseSemaphore(hOutputReadySemaphore, 1, NULL);
                WaitForSingleObject(hOutputReadySemaphore, INFINITE);
                ReleaseSemaphore(hOutputReadySemaphore, 1, NULL);
                ReleaseSemaphore(hInputReadySemaphore, 1, NULL);

                for (int i = 0; i < 21; i++) {
                    file << message[i];
                }
            } else {
                for (int i = 0; i < 21; i++) {
                    file << message[i];
                }
            }

            file.close();
            cout << "\nInput 1 to write message;\nInput 0 to exit process\n";
            cin >> key;
        }

        if (key != "0" && key != "1") {
            cout << "\nIncorrect value!\nInput 1 to write message;\nInput 0 to exit process\n";
            cin >> key;
        }

        if (key == "0") {
            cout << "Process ended.";
            break;
        }
    }
}
