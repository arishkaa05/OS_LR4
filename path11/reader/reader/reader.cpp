
#include <Windows.h>
#include <fstream>
#include <time.h>
#include <string>

#define PAGES 11

using namespace std;

int main()
{
	DWORD processId, start, pageSize;
	HANDLE hFileMappingObject;
	HANDLE readerSemaphore[PAGES];
	HANDLE writerSemaphore[PAGES];
	SYSTEM_INFO siSysInfo;
	fstream log;
	string logName;
	void* viewOfFile;

	GetSystemInfo(&siSysInfo);
	pageSize = siSysInfo.dwPageSize;

	processId = GetCurrentProcessId();				//извлекает идентификатор процесса вызывающего процесса
	start = GetTickCount64();

	srand(processId);								//используется для установки начала последовательности

	logName = "D:\\OS_LR4\\path1\\logs\\reader_log_" + to_string(processId) + ".txt";
	log.open(logName.c_str(), fstream::app);


	hFileMappingObject = OpenFileMapping(			//открывает объект "проецируемый файл"
		FILE_MAP_READ,								// режим доступа
		FALSE,										// наследование
		L"Local\\file"								// имя объекта
	);

	viewOfFile = MapViewOfFile(						//отображает представление проецируемого файла в адресное пространство вызывающего процесса
		hFileMappingObject,							// дескр. объекта проецируемый файл
		FILE_MAP_READ,								// режим доступа
		0, 0,										// смещение
		PAGES * pageSize);							// число отображаемых байтов

	VirtualLock(viewOfFile, PAGES * pageSize);		// Блокирует указанную область ВАП процесса в физической памяти

	for (int i = 0; i < PAGES; i++)
	{
		wchar_t writerName[] = L"writer ";
		writerName[6] = '0' + i;
		writerSemaphore[i] = OpenSemaphore(			//oткрывает существующий именованный объект семафора
			SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,	//доступ
			FALSE,									//процессы не наследуют дескриптор
			writerName);							//имя открываемого семафора.

		wchar_t readerName[] = L"reader ";
		readerName[6] = '0' + i;
		readerSemaphore[i] = OpenSemaphore(			//oткрывает существующий именованный объект семафора
			SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,	//доступ
			FALSE,									//процессы не наследуют дескриптор
			readerName);							//имя открываемого семафора.
	}
	// Чтение
	for (int i = 0; i < PAGES; i++)
	{
		int page = WaitForMultipleObjects(			//ждет, когда массив объектов перейдет в сигнальное состояние
			PAGES,									// количество объектов в массиве
			readerSemaphore,						// массив объектов
			FALSE,									// ждем любого объекта
			INFINITE);

		log << processId << "\t" << page << "\t" << "READ_START" << "\t" << GetTickCount64() - start << "\t" << GetTickCount64() << "\t" << start << "\n";
		char* buf = (char*)((char*)viewOfFile + page * pageSize);
		char buffer[2];
		buffer[0] = buf[0];
		buffer[1] = buf[1];
		Sleep(500 + rand() % 1000);
		log << processId << "\t" << page << "\t" << "READ_END" << "\t" << GetTickCount64() - start << "\t" << GetTickCount64() << "\t" << start << "\n";

		ReleaseSemaphore(							//увеличивает значение счетчика семафора
			writerSemaphore[page],					//идентификатор семафора 
			1,										//значение инкремента 
			NULL);									//адрес переменной для записи 
		log << processId << "\t" << page << "\t" << "WSEM_RELEASED" << "\t" << GetTickCount64() - start << "\t" << GetTickCount64() << "\t" << start << "\n";
	}

	for (int i = 0; i < PAGES; i++)					// Закрытие дескрипторов
	{
		CloseHandle(readerSemaphore[i]);
		CloseHandle(writerSemaphore[i]);
	}
	VirtualUnlock(viewOfFile, pageSize);			//разблокирует указанный диапазон страниц в ВАП процесса
	UnmapViewOfFile(viewOfFile);					//отменяет сопоставление представления файла из адресного пространства вызывающего процесса
	log.close();
}
