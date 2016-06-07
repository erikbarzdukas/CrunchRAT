#include <iostream>
#include <string>
#include "stdafx.h"				    // WinHttpClient
#include "WinHttpClient.h"		// WinHttpClient

wstring getHostname(); // Function prototype


int main()
{
	wcout << getHostname() << endl;

	return 0;
}


// Gets the system hostname - Working as of 06/06/16
// Returns "hostname" wstring
wstring getHostname()
{
	wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1] = L""; // wchar_t array initialization - MAX_COMPUTERNAME_LENGTH + 1 per MSDN
	DWORD size;
	GetComputerName(buffer, &size); // Gets the NetBIOS name of the local computer

	wstring hostname = buffer; // Creates new "hostname" wstring and sets value to the wchar_t array buffer above
	return hostname; // Returns the "hostname" wstring to the calling function
}
// End of getHostname() function
