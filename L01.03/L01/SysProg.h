#pragma once

#include <afxmt.h>

inline void DoWrite()
{
	cout << endl;
}

template <class T, typename... Args> inline void DoWrite(T& value, Args... args)
{
	cout << value << " ";
	DoWrite(args...);
}

static CCriticalSection cs;
template <typename... Args> inline void SafeWrite(Args... args)
{
	cs.Lock();
	DoWrite(args...);
	cs.Unlock();
}

