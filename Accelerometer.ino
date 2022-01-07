// Collect accelerometer data + send data via MQTT

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Wire.h"

#define MAX_MSG_LEN (3500)

// Wifi configuration - this will be whatever WiFi is available
const char *ssid = "Petra";
const char *password = "Hrabka1998";

// MQTT Configuration - comment this out once you set up Raspberry Pi
// if you have a hostname set for the MQTT server, you can use it here
// const char *serverHostname = "broker.hivemq.com";
const char *serverHostname = "84.238.36.3";
const char *mqttUser = "IoTproject";
const char *mqttPswd = "Thursday123";


// the topics we want to use
const char *topic_x = "warehouse1/sensor1/x";         // probably something like accel_data
const char *topic_y = "warehouse1/sensor1/y";         // probably something like accel_data
const char *topic_z = "warehouse1/sensor1/z";         // probably something like accel_data

// Acelerometer setup
const uint8_t MPU_addr = 0x68; // I2C address of the MPU-6050

// Scales depending on accelerometer range - full-scale, 2g, 4g, 8g, 16g
const float MPU_ACCL_2_SCALE = 16384.0;   // 2^14
const float MPU_ACCL_4_SCALE = 8192.0;    // 2^13
const float MPU_ACCL_8_SCALE = 4096.0;    // 2^12
const float MPU_ACCL_16_SCALE = 2048.0;   // 2^11

// struct for raw 16-bit data from the sensor 
struct rawdata {
  int16_t AcX;
  int16_t AcY;
  int16_t AcZ;
  int16_t Tmp;
};

// struct for scaled data
struct scaleddata {
  float AcX;
  float AcY;
  float AcZ;
  float Tmp;
};

bool checkI2c(byte addr);
void mpu6050Begin(byte addr);
rawdata mpu6050Read(byte addr, bool Debug);
void setMPU6050scales(byte addr, uint8_t Accl);
void getMPU6050scales(byte addr, uint8_t &Accl);
scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug);

WiFiClient espClient;
PubSubClient client(espClient);

// Connect to WiFi
void connectWifi()
{
  delay(10);
  // Connecting to a WiFi network
  Serial.printf("\nConnecting to %s\n", ssid);
  WiFi.softAP(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected on IP address ");
  Serial.println(WiFi.localIP());
}

// Callback function
// Prototype constructor defined by creator of PubSubClient
// as (char *msgTopic, byte *msgPayload, unsigned int msgLength)
void callback(char *msgTopic, byte *msgPayload, unsigned int msgLength)
{
  // Copy payload to a static string
  static char message[MAX_MSG_LEN + 1];
  if (msgLength > MAX_MSG_LEN)
  {
    msgLength = MAX_MSG_LEN;
  }
  strncpy(message, (char *)msgPayload, msgLength);
  message[msgLength] = '\0';
}


// Connect to MQTT server - checks for a connection,
// If none is active it attempts to connect to MQTT server
void connectMQTT()
{
  // Wait until we're connected
  while (!client.connected())
  {
    // Create a random client ID
    String clientId = "ESP8266-";
    clientId += String(random(0xffff), HEX);
    Serial.printf("MQTT connecting as client %s...\n", clientId.c_str());
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPswd))
//    if (client.connect(clientId.c_str()))
    {
      Serial.println("MQTT connected");
    } else
    {
      Serial.printf("MQTT failed, state %s, retrying...\n", client.state());
      // Wait before retrying
      delay(2500);
    }
  }
}

// Function to check I2C communication 
bool checkI2c(byte addr) {
  // We are using the return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address
  Serial.println(" ");
  Wire.beginTransmission(addr);

  if (Wire.endTransmission() == 0)
  {
    Serial.print(" Device Found at 0x");
    Serial.println(addr, HEX);
    return true;
  }
  else
  {
    Serial.print(" No Device Found at 0x");
    Serial.println(addr, HEX);
    return false;
  }
}

// Wake up MPU6050
void mpu6050Begin(byte addr) {
  // This function initializes the MPU-6050 IMU Sensor
  // It verifies the address is correct and wakes up the MPU
  if (checkI2c(addr)) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // Set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    delay(30); // Ensure sensor has enough time to power up
  }
}

// Function to scale data based on chosen resolution
scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug) {
  scaleddata values;
  float scale_value = 0.0;
  byte Accl;
  getMPU6050scales(MPU_addr, Accl);

  scale_value = 0.0;
  switch (Accl) {
    case 0:
      scale_value = MPU_ACCL_2_SCALE;   // ±2 g resolution 
      break;
    case 1:
      scale_value = MPU_ACCL_4_SCALE;   // ±4 g resolution 
      break;
    case 2:
      scale_value = MPU_ACCL_8_SCALE;   // ±8 g resolution
      break;
    case 3:
      scale_value = MPU_ACCL_16_SCALE;    // ±16 g resolution 
      break;
    default:
      break;
  }
  values.AcX = (float) data_in.AcX / scale_value;
  values.AcY = (float) data_in.AcY / scale_value;
  values.AcZ = (float) data_in.AcZ / scale_value;

  if (Debug) {
    
    Serial.print("AcX = "); Serial.print(values.AcX);
    Serial.print(" g| AcY = "); Serial.print(values.AcY);
    Serial.print(" g| AcZ = "); Serial.print(values.AcZ); Serial.println(" g");
    client.publish(topic_x, String(values.AcX).c_str());
    client.publish(topic_y, String(values.AcY).c_str());
    client.publish(topic_z, String(values.AcZ).c_str());
  }
  return values;
}

// Get scales to scale raw 16-bit data
void getMPU6050scales(byte addr, uint8_t &Accl) {
  Wire.beginTransmission(addr);
  Wire.write(0x1B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 2, true); // request a total of 14 registers
  
  // Right shift of bit 3 and 4 by 3 spots
  Accl = (Wire.read() & (bit(3) | bit(4))) >> 3;
}

// Read raw 16-bit data from MPU6050
rawdata mpu6050Read(byte addr, bool Debug) {

  rawdata values;

  Wire.beginTransmission(addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 14, true); // request a total of 14 registers
  values.AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  values.AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  values.AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  
  return values;
}

// Set scales 
void setMPU6050scales(byte addr, uint8_t Accl) {
  Wire.beginTransmission(addr);
  Wire.write(0x1B); // write to register starting at 0x1B
  Wire.write(Accl); // Self Tests Off and set Accl FS to 8g
  Wire.endTransmission(true);
}

void setup()
{
  Wire.begin();
  // Configure serial port for debugging
  Serial.begin(115200);
  // Initialise wifi connection - this will wait until connected
  connectWifi();
  // connect to MQTT server
  client.setServer(serverHostname, 1883);
  //  client.setCallback(callback);
  mpu6050Begin(MPU_addr);

}

void loop()
{
  if (!client.connected())
  {
    connectMQTT();
  }
  // Start MQTT
  client.loop();
  // Send 1024 samples of each acceleration axis every 10 seconds
  for (int i = 0; i <= 1023; i++) {
    rawdata next_sample;
    setMPU6050scales(MPU_addr, 0b00010000);
    next_sample = mpu6050Read(MPU_addr, true);
    convertRawToScaled(MPU_addr, next_sample, true);
  }
  delay(10000);
}
