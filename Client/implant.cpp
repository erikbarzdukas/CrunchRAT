#include <iostream>
#include <string>

// Function prototypes
wstring getHostname();
wstring getOS();


// Start of main() function
int main()
{
	wcout << getHostname() << endl;
	wcout << getOS() << endl;
	
	return 0;
}
// End of main() function


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


// Gets the system OS - Working as of 06/06/16
// Returns "os" wstring
wstring getOS()
{
	HKEY hRegistry = NULL;
	RegOpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion", &hRegistry); // Opens a handle to the Registry
	
	DWORD size;
	RegQueryValueEx(hRegistry, L"ProductName", NULL, NULL, NULL, &size); // Queries the size (in bytes) for the "ProductName" Registry key - Will save size (in bytes) including null-terminator in "size" DWORD
	
	wchar_t buffer[256]; // EWWWW ...HARD-CODED ARRAY SIZE
	RegQueryValueEx(hRegistry, L"ProductName", NULL, NULL, (LPBYTE) &buffer, &size); // Queries the "ProductName" Registry key and stores output in buffer

	wstring os = buffer; // Creates new "os" wstring and sets value to the wchar_t array buffer above
	return os; // Returns the "os" wstring to the calling function
}
// End of getOS() function
