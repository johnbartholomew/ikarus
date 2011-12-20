#include "Global.h"
#include "Win32Error.h"

Win32Error::Win32Error()
:	mErrorCode(GetLastError())
{
	initMessage(0);
}

Win32Error::Win32Error(const char *baseMessage)
:	mErrorCode(GetLastError())
{
	initMessage(baseMessage);
}

Win32Error::Win32Error(const char *baseMessage, unsigned int errorCode)
:	mErrorCode(errorCode)
{
	initMessage(baseMessage);
}

Win32Error::Win32Error(unsigned int errorCode)
:	mErrorCode(errorCode)
{
	initMessage(0);
}

void Win32Error::initMessage(const char *baseMessage)
{
	LPSTR bufPtr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, mErrorCode, 0, reinterpret_cast<LPSTR>(&bufPtr), 0, 0);
	if (baseMessage)
	{
		mErrorMessage = baseMessage;
		mErrorMessage += "\r\n";
	}
	mErrorMessage += bufPtr;
	LocalFree(bufPtr);
}
