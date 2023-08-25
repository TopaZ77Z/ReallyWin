#pragma once
#include "IAppSystem.h"

class ILocalize : public IAppSystem
{
public:
	// adds the contents of a file to the localization table
	virtual bool AddFile(const char* fileName, const char* pPathID = 0, bool bIncludeFallbackSearchPaths = false) = 0;

	// Remove all strings from the table
	virtual void RemoveAll() = 0;

	// Finds the localized text for tokenName. Returns NULL if none is found.
	virtual wchar_t* Find(const char* tokenName) = 0;

	// Like Find(), but as a failsafe, returns an error message instead of NULL if the string isn't found.  
	virtual const wchar_t* FindSafe(const char* tokenName) = 0;

	// converts an english string to unicode
	// returns the number of wchar_t in resulting string, including null terminator
	virtual int ConvertANSIToUnicode(const char* ansi, wchar_t* unicode, int unicodeBufferSizeInBytes) = 0;

	// converts an unicode string to an english string
	// unrepresentable characters are converted to system default
	// returns the number of characters in resulting string, including null terminator
	virtual int ConvertUnicodeToANSI(const wchar_t* unicode, char* ansi, int ansiBufferSize) = 0;
};