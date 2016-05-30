CREATE DATABASE RAT;
USE RAT;
CREATE TABLE users (username VARCHAR(50), password VARCHAR(50));
INSERT INTO users (username, password) values ("admin", "changeme");
CREATE TABLE hosts (hostname VARCHAR(100), date VARCHAR(100), os VARCHAR(200), architecture VARCHAR(3));
CREATE TABLE tasks (id INT(16) AUTO_INCREMENT, user VARCHAR(50), hostname VARCHAR(100), action VARCHAR(20), secondary TEXT(65535), PRIMARY KEY (id));
CREATE TABLE output (id INT(16) AUTO_INCREMENT, user VARCHAR(50), hostname VARCHAR(100), action VARCHAR(20), secondary TEXT(65535), stdout TEXT(65535), stderr TEXT(65535), status VARCHAR(1), PRIMARY KEY (id));
CREATE TABLE chat (user VARCHAR(50), date VARCHAR(100), message TEXT(65535));
