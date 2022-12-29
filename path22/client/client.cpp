#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

void CALLBACK lpCompletionRoutine(DWORD errorCode, DWORD bytestransfered, LPOVERLAPPED lpOverlapped)
{
}

int main()
{
    HANDLE hNamedPipe;
    OVERLAPPED lpOverlapped;
    LPCWSTR lpszPipeName = L"\\\\.\\pipe\\MyPipe";

    char* c = new char[MAX_PATH];

    ZeroMemory(&lpOverlapped, sizeof(lpOverlapped));

    hNamedPipe = CreateFile(
        lpszPipeName,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hNamedPipe == INVALID_HANDLE_VALUE) {
        cout << "Error " << GetLastError() << ".\n";
        SetLastError(0);
    }
    cout << "Data from server\n";
    while (true) {
        ReadFileEx(hNamedPipe, c, 512, &lpOverlapped, lpCompletionRoutine);
        cout << c << endl << endl;
        SleepEx(INFINITE, TRUE);

    }

    return 0;
}