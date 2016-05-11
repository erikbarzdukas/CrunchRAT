# CrunchRAT
CrunchRAT currently supports the following features:
* File upload
* File download
* Command execution

It is currently single-threaded (only one task at a time), but multi-threading (or multi-tasking) is currently in the works. Additional features will be included at a later date.

## Server
The server-side of the RAT uses PHP and MySQL. The server-side of the RAT has been tested and works on the following:
* Ubuntu 15.10 (Desktop or Server edition)
* Ubuntu 16.04 (Desktop or Server edition)

Once the latest RAT code has been downloaded, there will be three directories:
* Client - Contains implant code (ignore for this section)
* Server - Contains server code
* Setup - Contains setup files

### Dependencies Setup
1. Within the `Setup` directory, there are two dependencies setup shell scripts. If you are using Ubuntu 15.10 run `15_10_dependencies.sh`, and if you're using Ubuntu 16.04 run `16_04_dependencies.sh`. **Note: This needs to be run as root**.
2. When asked for a new MySQL root password, please choose one that is complex. This information is needed at a later step.

### HTTPS Setup
1. CrunchRAT uses a self-signed certificate to securely communicate between the server and implant. Run the `https_setup.sh` shell script to automate the HTTPS setup. **Note: This needs to be run as root**. When asked to fill out the certificate information (Country Name, etc), please fill out all information. Snort rules already exist to alert on the dummy OpenSSL certificates. Don't be that guy that gets flagged by not filling out this information.
2. After running that setup file, you will need to edit the `/etc/apache2/ports.conf` file and comment out the `Listen 80` line. This will prevent the use of HTTP. Restart the Apache2 service after editing that file by running `service apache2 restart`.

### Database Setup
1. CrunchRAT has the database_setup.sql file to automate the generation of the RAT database and associated tables. **Note: The database_setup.sql file will create a RAT user account (defaults to admin/changeme). Please edit this file and change the default username and password**.
2. Launch a MySQL shell by running the following command `mysql -u root -p`. When asked for the root password, enter what you chose during the dependencies setup step.
3. Once in the MySQL shell, you will want to use the following command `source <path to database_setup.sql`.

### Miscellaneous Setup
1. Copy all files from the `Server` directory to the webroot (typically /var/www/html).
2. Remove the default Apache2 `index.html` file in the webroot.
3. You will want to create a `uploads` directory in the webroot. You will want to make sure that www-data can access this directory with the following command `sudo chown www-data:www-data uploads`. This directory will temporarily store the file(s) to be uploaded to the infected system(s).
4. You will want to create a `downloads` directory as well. **Note: It is absolutely critical that you don't put this folder in the webroot**. I typically create this directory in the /home/USERNAME directory. You will want to make sure that www-data can access this directory with the following command `sudo chown www-data:www-data downloads`. This directory will store all of the files downloaded from the infected system(s).
5. In the webroot, open the `config/config.php` file. This is the main RAT configuration file. Make sure that you update all of the variables (webrootPath, uploadsPath, downloadsPath, and dbPass) to match your environment.
6. Lastly, edit the `/etc/php/7.0/apache2/php.ini` (for Ubuntu 16.04) or `/etc/php5/apache2/php.ini` (for Ubuntu 15.10). Make sure that `file_uploads = On` and change the size on `upload_max_filename` and `post_max_size` entries. Restart the Apache2 service using `service apache2 restart` afterwards. These two entries control the max file size for remote file downloads.

## Client
CrunchRAT is written in C# for simplicity. The C# binary does not have a persistence mechanism in place, but plans to write a C++ stager are currently in the works.

Targeted Framework: .NET Framework 3.5 (enabled by default on Windows 7 systems)

1. Create a new console project in Visual Studio
2. Copy implant.cs code from `Client` directory and add it to the project.
3. Change `Output Type` to `Windows Application` (this will hide the command window) (`Project` --> `Properties` --> `Output Type`).
4. Make sure `Target Framework` is `.NET Framework 3.5`.
5. In the actual code, there will be a variable called `c2` - Change this variable to the IP address or domain name of the C2 server
6. Compile and your implant executable is ready to run.

## Thanks
Special thanks to the following people for helping me along the way:
* Michael Bailey
* Peter Kacherginsky
* Andy Rector
* Cole Hoven
* Erik Barzdukas
* Nick Gordon