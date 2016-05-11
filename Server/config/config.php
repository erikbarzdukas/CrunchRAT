<?php
  # RAT configuration file

  # Change if webroot isn't /var/www/html - Forward slash at the end is necessary and must not be ommitted
  $webrootPath = "/var/www/html/";

  # Uploads directory - Forward slash at the end is necessary and must not be ommitted
  $uploadsPath = $webrootPath . "uploads/";

  # Downloads directory - www-data must have write permission to this directory - Forward slash at the end is necessary and must not be ommitted
  $downloadsPath = "/home/t3ntman/Desktop/downloads/";

  # Don't change
  $dbHost = "127.0.0.1";

  # Don't change
  $dbName = "RAT";

  # Don't change
  $dbUser = "root";

  # Change
  $dbPass = "root";
?>