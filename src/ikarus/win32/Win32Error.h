#ifndef WIN32_ERROR_H
#define WIN32_ERROR_H

class Win32Error : public std::exception
{
public:
	explicit Win32Error();
	explicit Win32Error(const char *baseMessage);
	explicit Win32Error(const char *baseMessage, unsigned int errorCode);
	explicit Win32Error(unsigned int errorCode);
	virtual const char *what() const
	{ return mErrorMessage.c_str(); }
	virtual unsigned int getErrorCode() const
	{ return mErrorCode; }
private:
	void initMessage(const char *baseMessage);
	unsigned int mErrorCode;
	std::string mErrorMessage;
};

#endif
