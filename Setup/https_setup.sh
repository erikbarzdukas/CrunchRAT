#!/bin/bash
# Run this as root

# Enables SSL
a2enmod ssl

# Restarts Apache2 service
service apache2 restart

# Creates certificate directory
mkdir /etc/apache2/ssl

# Generates self-signed certificate
openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout /etc/apache2/ssl/CrunchRAT.key -out /etc/apache2/ssl/CrunchRAT.crt

# Copy Crunch RAT Apache configuration file to /etc/apache2/sites-available
cp CrunchRAT.conf /etc/apache2/sites-available

# Enable Crunch RAT Apache configuration
a2ensite CrunchRAT.conf

# Uses sed to comment out "Listen 80" line in /etc/apache2/ports.conf
sed -i -e 's/Listen 80/#Listen 80/g' /etc/apache2/ports.conf

# Restart Apache2 service
service apache2 restart
