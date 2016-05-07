<?php  
  # Necessary at the top of every page for session management
  session_start();
  
  # Includes the RAT configuration file
  include "config/config.php";

  # Establishes a connection to the RAT database
  # Uses variables from "config/config.php"
  # "SET NAMES utf8" is necessary to be Unicode-friendly
  $dbConnection = new PDO("mysql:host=$dbHost;dbname=$dbName", $dbUser, $dbPass, array(PDO::MYSQL_ATTR_INIT_COMMAND => "SET NAMES utf8"));
?>

<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>CrunchRAT</title>
    <link href="bootstrap/css/bootstrap.css" rel="stylesheet">
    <style type="text/css">
      body 
      {
        padding-top: 40px;
        padding-bottom: 40px;
        background-color: #f5f5f5;
      }
      .form-signin 
      {
        max-width: 300px;
        padding: 19px 29px 29px;
        margin: 0 auto 20px;
        background-color: #fff;
        border: 1px solid #e5e5e5;
        -webkit-border-radius: 5px;
           -moz-border-radius: 5px;
                border-radius: 5px;
        -webkit-box-shadow: 0 1px 2px rgba(0,0,0,.05);
           -moz-box-shadow: 0 1px 2px rgba(0,0,0,.05);
                box-shadow: 0 1px 2px rgba(0,0,0,.05);
      }
      .form-signin .form-signin-heading,
      .form-signin .checkbox 
      {
        margin-bottom: 10px;
      }
      .form-signin input[type="text"],
      .form-signin input[type="password"] {
        font-size: 16px;
        height: auto;
        margin-bottom: 15px;
        padding: 7px 9px;
      }
    </style>
    <link href="bootstrap/css/bootstrap-responsive.css" rel="stylesheet">
  </head>

  <body>
    <div class="container">
      <form class="form-signin" method="post">
        <h2 class="text-center">CrunchRAT</h2>
        <input type="text" class="input-block-level" name="username" placeholder="Username">
        <input type="password" class="input-block-level" name="password" placeholder="Password">
        <?php
          # Stores the username and password that were entered into the login form
          $username = $_POST["username"];
          $password = $_POST["password"];

          # This action is performed when the RAT user clicks "Sign In"
          if (isset($username) && !empty($username) && isset($password) && !empty($password))
          {
            # Determines if the username/password entered match a valid set of credentials
            $statement = $dbConnection->prepare("SELECT * FROM users WHERE username = :username AND password = :password");
            $statement->bindValue(":username", $username);
            $statement->bindValue(":password", $password);
            $statement->execute();

            # rowCount() will return 1 if successful authentication
            if ($statement->rowCount() == 1)
            {
              # Successful authentication occurred
              # We now start a session
              $_SESSION["authenticated"] = 1;

              # Redirects the RAT user to command.php
              # "command.php" will allow them to task commands
              header("Location: command.php");
            }
            # Else failed authentication
            else
            {
              # Redirects to login.php with a failed error
              header("Location: login.php?error=1");
            }
      
            # Kills database connection
            $statement->connection = null;
          }

          # Prints "Login Failed" message to the screen if incorrect credentials were entered
          if (isset($_GET["error"]))
          {
            echo "<h4>Login Failed</h4>";
          }
        ?>
        <button class="btn btn-large btn-primary" type="submit">Sign In</button>
      </form>
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