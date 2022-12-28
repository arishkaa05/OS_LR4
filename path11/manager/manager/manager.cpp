
#include <Windows.h>

#define PAGES 11

int main()
{
	DWORD pageSize;
	HANDLE file, hMapping;
	HANDLE readerSemaphore[PAGES];
	HANDLE writerSemaphore[PAGES];
	HANDLE processes[PAGES * 2];
	PROCESS_INFORMATION writerInfo[PAGES];
	PROCESS_INFORMATION readerInfo[PAGES];
	SYSTEM_INFO siSysInfo;
	STARTUPINFO startInfo;
	SECURITY_ATTRIBUTES semaphoreSecurityAttributes = { sizeof(semaphoreSecurityAttributes), NULL, TRUE };
	wchar_t buffer[100] = L"";

	GetSystemInfo(&siSysInfo);
	pageSize = siSysInfo.dwPageSize;

	file = CreateFile(
		L"D:\\OS_LR4\\path 1\\file.txt",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	hMapping = CreateFileMapping(
		file,
		NULL,
		PAGE_READWRITE,
		0,
		pageSize * PAGES,
		L"Local\\file");

	for (int i = 0; i < PAGES; i++)
	{
		wchar_t writerName[] = L"writer ";
		writerName[6] = '0' + i;
		writerSemaphore[i] = CreateSemaphore(
			&semaphoreSecurityAttributes,
			1,
			1,
			writerName
		);

		wchar_t readerName[] = L"reader ";
		readerName[6] = '0' + i;
		readerSemaphore[i] = CreateSemaphore(
			&semaphoreSecurityAttributes,
			0,
			1,
			readerName
		);
	}
	//информация для запуска нового процесса
	ZeroMemory(&startInfo, sizeof(startInfo));
	startInfo.cb = sizeof(startInfo);

	for (int i = 0; i < PAGES; i++)
	{
		ZeroMemory(&readerInfo[i], sizeof(readerInfo[i]));
		ZeroMemory(&writerInfo[i], sizeof(writerInfo[i]));

		CreateProcess(
			L"D:\\OS_LR4\\path 1\\reader\\x64\\Debug\\reader.exe",		// имя исполняемого файл
			buffer,														// командная строка
			NULL,														// атрибуты защиты процесса
			NULL,														// атрибуты защиты потоков
			TRUE,														// наследование дескрипторов
			CREATE_SUSPENDED,											//поток нового процесса создается в приостановленном состоянии
			NULL,														// блок памяти с переменными окружения
			NULL,														// текущий диск и каталог для процесса
			&startInfo,													// указывает на структуру STARTUPINFO
			&readerInfo[i]												// инициализируемая структура
		);
		CreateProcess(
			L"D:\\OS_LR4\\path 1\\writer\\x64\\Debug\\writer.exe",		// имя исполняемого файл
			buffer,														// командная строка
			NULL,														// атрибуты защиты процесса
			NULL,														// атрибуты защиты потоков
			TRUE,														// наследование дескрипторов
			CREATE_SUSPENDED,											//поток нового процесса создается в приостановленном состоянии
			NULL,														// блок памяти с переменными окружения
			NULL,														// текущий диск и каталог для процесса
			&startInfo,													// указывает на структуру STARTUPINFO
			&readerInfo[i]												// инициализируемая структура
		);

		processes[i] = readerInfo[i].hProcess;
		processes[PAGES * 2 - 1 - i] = writerInfo[i].hProcess;
	}


	for (int i = 0; i < PAGES; i++)
		ResumeThread(writerInfo[i].hThread);
	for (int i = 0; i < PAGES; i++)
		ResumeThread(readerInfo[i].hThread);

	WaitForMultipleObjects(PAGES * 2, processes, TRUE, INFINITE);		// ждем окончание процессов

	for (int i = 0; i < PAGES; i++)
	{
		CloseHandle(writerInfo[i].hProcess);
		CloseHandle(writerInfo[i].hThread);

		CloseHandle(readerInfo[i].hProcess);
		CloseHandle(readerInfo[i].hThread);

		CloseHandle(readerSemaphore[i]);
		CloseHandle(writerSemaphore[i]);
	}
	CloseHandle(hMapping);
	CloseHandle(file);

	return 0;
}
