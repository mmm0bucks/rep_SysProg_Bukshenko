// L01.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "L01.h"
#include "SysProg.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CWinApp theApp;

enum MessageTypes
{
	MT_CLOSE,
	MT_DATA,
};

struct MessageHeader
{
	int messageType;
	int size;
};

struct Message
{
	MessageHeader header = {0};
	string data;
	Message() = default;
	Message(MessageTypes messageType, const string& data = "")
		:data(data)
	{
		header = { messageType,  int(data.length()) };
	}
};

class Session
{
	queue<Message> messages;
	CRITICAL_SECTION cs;
	HANDLE hEvent;
public:
	int sessionID;

	Session(int sessionID)
		:sessionID(sessionID)
	{
		InitializeCriticalSection(&cs);
		//hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~Session()
	{
		DeleteCriticalSection(&cs);
		CloseHandle(hEvent);
	}

	void addMessage(Message& m)
	{
		EnterCriticalSection(&cs);
		messages.push(m);
		SetEvent(hEvent);
		LeaveCriticalSection(&cs);
	}

	bool getMessage(Message& m)
	{
		bool res = false;
		WaitForSingleObject(hEvent, INFINITE);
		EnterCriticalSection(&cs);
		if (!messages.empty())
		{
			res = true;
			m = messages.front();
			messages.pop();
		}
		if (messages.empty())
		{
			ResetEvent(hEvent);
		}
		LeaveCriticalSection(&cs);
		return res;
	}

	void addMessage(MessageTypes messageType, const string& data = "")
	{
		Message m(messageType, data);
		addMessage(m);
	}
};

DWORD WINAPI MyThread(LPVOID lpParameter)
{
	auto session = static_cast<Session*>(lpParameter);
	SafeWrite(L"session", session->sessionID, L"created");
	while (true)
	{
		Message m;
		if (session->getMessage(m))
		{
			switch (m.header.messageType)
			{
				case MT_CLOSE:
				{
					SafeWrite(L"session", session->sessionID, L"closed");
					delete session;
					return 0;
				}
				case MT_DATA:
				{
					SafeWrite(L"session", session->sessionID, L"data", m.data);
					Sleep(1000 * session->sessionID);
					break;
				}
			}
		}
	}
	return 0;
}

int main()
{
	HANDLE confirmEvent = ::CreateEvent(NULL, FALSE, FALSE, L"ConfirmEvent");
	HANDLE startEvent = ::CreateEvent(NULL, FALSE, FALSE, L"StartEvent");
	HANDLE stopEvent = ::CreateEvent(NULL, FALSE, FALSE, L"CloseProc");
	HANDLE exitEvent = ::CreateEvent(NULL, FALSE, FALSE, L"ExitProc");

	HANDLE events[3];
	events[0] = startEvent;
	events[1] = stopEvent;
	events[2] = exitEvent;

	::SetEvent(confirmEvent);

	std::vector<HANDLE> hThreads;
	std::vector<Session*> sessions;

	while (true)
	{
		DWORD dwWaitRes = ::WaitForMultipleObjects(sizeof(events) / sizeof(HANDLE), events, FALSE, INFINITE);
		switch (dwWaitRes)
		{
		case WAIT_OBJECT_0 + 2:
			for (int i{ 0 }; i < sessions.size(); ++i)
			{
				sessions[i]->addMessage(MT_CLOSE);
				CloseHandle(hThreads[i]);
			}
			::SetEvent(confirmEvent);
			return 0;
		case WAIT_OBJECT_0 + 1:

			if (!hThreads.size())
			{
				::SetEvent(confirmEvent);
				return 0;
			}
			sessions.back()->addMessage(MT_CLOSE);
			sessions.pop_back();

			::CloseHandle(hThreads.back());
			hThreads.pop_back();

			break;

		case WAIT_OBJECT_0:

			sessions.push_back(new Session(hThreads.size()));
			hThreads.push_back(::CreateThread(NULL, 0, MyThread, (LPVOID)sessions.back(), 0, NULL));
			break;

		default:

			std::cout << "There was an error" << std::endl;
			return 0;
		}
		::SetEvent(confirmEvent);
	}

	WaitForMultipleObjects((DWORD)hThreads.size(), hThreads.data(), TRUE, INFINITE);

	CloseHandle(confirmEvent);
	CloseHandle(startEvent);
	CloseHandle(stopEvent);

	return 0;
}
