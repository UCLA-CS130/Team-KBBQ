-- Create Database
CREATE DATABASE CS130;

-- Use new database
USE CS130;

-- Create Actors table
CREATE TABLE Actors (
    Name VARCHAR(40),
    Movie VARCHAR(80),
    Year INT,
    Role VARCHAR(40)
);

-- Load sample data from 'actors.csv' located in same directory
LOAD DATA LOCAL INFILE 'actors.csv' INTO TABLE Actors
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

-- Create Persons table
CREATE TABLE Persons (
    ID int NOT NULL AUTO_INCREMENT,
    LastName varchar(255) NOT NULL,
    FirstName varchar(255),
    Age int,
    PRIMARY KEY (ID)
);

--  Create Persons table with manual data
INSERT INTO Persons (LastName, FirstName, Age)
VALUES ("Doe", "John", 30), ("Doe", "Jane", 31);

-- Create new user CS130user
CREATE USER 'CS130user'@'localhost' IDENTIFIED BY 'password';

--  Give CS130user root privileges
GRANT ALL PRIVILEGES ON *.* TO 'CS130user'@'localhost'
WITH GRANT OPTION;

-- Refresh privileges
FLUSH PRIVILEGES;
