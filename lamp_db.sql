-- Drop existing database if it exists
-- DROP DATABASE IF EXISTS lamp_db;

-- Create a new database
-- CREATE DATABASE lamp_db;

-- -- Use the new database
-- USE railway;

DROP TABLE lamps;
DROP TABLE delay_time;
DROP TABLE source_control;

-- Create a simple table for lamps
CREATE TABLE lamps (
  id INT PRIMARY KEY,
  name VARCHAR(100),
  state INT
);

-- Create a table for delay_time
CREATE TABLE delay_time (
  dt_value FLOAT
);

CREATE TABLE source_control (
  src_value VARCHAR(20)
);

-- Insert a row into lamps
INSERT INTO lamps (id, name, state) VALUES (1, 'Desk Lamp', 1);

-- Insert a delay time value
INSERT INTO delay_time (dt_value) VALUES (2);

INSERT INTO source_control (src_value) VALUES ('MQTT');

-- Query the lamps table
SELECT * FROM lamps;
