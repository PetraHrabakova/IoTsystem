DROP TABLE Velocity_x;
DROP TABLE Velocity_y;
DROP TABLE Velocity_z;
DROP TABLE Freq_peaks_x;
DROP TABLE Freq_peaks_y;
DROP TABLE Freq_peaks_z;
DROP TABLE Sensor;



CREATE TABLE Sensor (
	Sensor_ID INT,
	Location CHAR(30), 
	PRIMARY KEY (Sensor_ID) );
	
CREATE TABLE Velocity_x (
	Sensor_ID INT, 
	Velocity FLOAT,
	Category CHAR(5),
	Time_stamp TIMESTAMP,
	FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID) );
	
CREATE TABLE Velocity_y (
	Sensor_ID INT, 
	Velocity FLOAT,
	Category CHAR(5),
	Time_stamp TIMESTAMP,
	FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID) );

CREATE TABLE Velocity_z (
	Sensor_ID INT, 
	Velocity FLOAT,
	Category CHAR(5),
	Time_stamp TIMESTAMP,
	FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID) );

CREATE TABLE Freq_peaks_x (
	Sensor_ID INT, 
	Amplitude FLOAT,
	Freq 	FLOAT,	
	Time_stamp TIMESTAMP,
	FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID) );
	
CREATE TABLE Freq_peaks_y (
	Sensor_ID INT, 
	Amplitude FLOAT,
	Freq 	FLOAT,	
	Time_stamp TIMESTAMP,
	FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID) );
	
CREATE TABLE Freq_peaks_z (
	Sensor_ID INT, 
	Amplitude FLOAT,
	Freq 	FLOAT,	
	Time_stamp TIMESTAMP,
	FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID) );
	
insert into sensor(sensor_id, location) values(1, 'warehouse1');

