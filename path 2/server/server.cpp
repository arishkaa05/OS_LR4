
#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

void CALLBACK lpCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    if (dwErrorCode != 0)
        cout << "Error " << dwErrorCode;

    SetEvent(lpOverlapped->hEvent);
}

int main()
{
    HANDLE hNamedPipe;
    OVERLAPPED lpOverlapped;
    LPCWSTR lpszPipeName = L"\\\\.\\pipe\\MyPipe";

    char* c = new char[MAX_PATH];

    ZeroMemory(&lpOverlapped, sizeof(lpOverlapped));
    lpOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    hNamedPipe = CreateNamedPipe(
        lpszPipeName,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, //Для чтения и записи
        PIPE_TYPE_BYTE | PIPE_WAIT, //ориентирован на передачу байт Канал будет работать в блокирующем режиме, когда процесс переводится в состояние ожидания до завершения операций в канале
        1,                          // максимальное количество реализаций канала 
        512, 512,                   // размер выходного/входного  буфера в байтах 
        0,                          // время ожидания в миллисекундах 
        NULL);                      // адрес переменной для атрибутов защиты

    ConnectNamedPipe(hNamedPipe, &lpOverlapped);

    cout << "Enter data '\n' Enter \"END\" to end the program\n";
    while (true) {
        cin >> c;
        if (strcmp(c, "END") != 0)
        {
            cout << endl;
            WriteFileEx(hNamedPipe, c, strlen(c) + 1, &lpOverlapped, lpCompletionRoutine);
            WaitForSingleObject(lpOverlapped.hEvent, INFINITE);
        }
        else
        {
            cout << "\nServer finished\n";
            break;
        }
    }
}


