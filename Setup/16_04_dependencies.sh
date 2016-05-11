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

# Restarts Apache service
service apache2 restart
