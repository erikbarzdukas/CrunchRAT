# Must be run as root

# Updates
apt-get update

# Upgrades
apt-get -y upgrade

# Installs Apache
apt-get install -y apache2

# Installs MySQL
apt-get install -y mysql-server

# Installs PHP
apt-get install -y php libapache2-mod-php php-mcrypt php-mysql

# Removes default index.html file
rm -rf /var/www/html/index.html

# Restarts Apache service
service apache2 restart

# Creates uploads directory
mkdir /var/www/html/uploads

# Changes permissions so we can write to the uploads directory
chown www-data:www-data /var/www/html/uploads
