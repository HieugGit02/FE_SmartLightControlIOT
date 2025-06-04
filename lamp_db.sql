-- Drop existing database if it exists
-- DROP DATABASE IF EXISTS lamp_db;

-- Create a new database
-- CREATE DATABASE lamp_db;

-- -- Use the new database
-- USE railway;

DROP TABLE lamps;
DROP TABLE delay_time;
DROP TABLE source_control;
DROP TABLE lamp_audit;

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

CREATE TABLE lamp_audit (
  audit_id INT AUTO_INCREMENT PRIMARY KEY,
  lamp_id INT,
  old_state INT,
  new_state INT,
  action_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


-- Insert a row into lamps
INSERT INTO lamps (id, name, state) VALUES (1, 'Desk Lamp', 1);

-- Insert a delay time value
INSERT INTO delay_time (dt_value) VALUES (2);

INSERT INTO source_control (src_value) VALUES ('MQTT');

-- Query the lamps table
SELECT * FROM lamps;

-- Audit log for Lamp state
DELIMITER $$

CREATE TRIGGER trg_lamp_state_update
BEFORE UPDATE ON lamps
FOR EACH ROW
BEGIN
  IF OLD.state <> NEW.state THEN
    INSERT INTO lamp_audit (lamp_id, old_state, new_state)
    VALUES (OLD.id, OLD.state, NEW.state);
  END IF;
END$$

DELIMITER ;
