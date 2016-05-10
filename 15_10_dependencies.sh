# Must be run as root

apt-get update
apt-get upgrade -y

# Installs Apache
apt-get install -y apache2

# Installs MySQL
apt-get install -y mysql-server php5-mysql

# Installs PHP
apt-get install php5
