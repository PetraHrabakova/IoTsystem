#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "GFX4d.h"                                            // include library for the screen

// Objects
GFX4d gfx = GFX4d();                                          // Create object for the screen
WiFiClient espClient;
PubSubClient client(espClient);

// Wifi setup variables
#define wifi_ssid "Petra"
#define wifi_password "Hrabka1998"

// MQTT setup variables
// #define mqtt_server "broker.hivemq.com"
#define mqtt_server "84.238.36.3"
#define mqtt_user "IoTproject"
#define mqtt_password "Thursday123"
#define velocity_x "accel_data/sensor1/velocity_x"
#define velocity_y "accel_data/sensor1/velocity_y"
#define velocity_z "accel_data/sensor1/velocity_z"
#define vel_categ_x "accel_data/sensor1/vel_categ_x"
#define vel_categ_y "accel_data/sensor1/vel_categ_y"
#define vel_categ_z "accel_data/sensor1/vel_categ_z"
#define peaks_fx_topic "accel_data/sensor1/peaks_f_x"
#define peaks_fy_topic "accel_data/sensor1/peaks_f_y"
#define peaks_fz_topic "accel_data/sensor1/peaks_f_z"
#define peaks_ax_topic "accel_data/sensor1/peaks_a_x"
#define peaks_ay_topic "accel_data/sensor1/peaks_a_y"
#define peaks_az_topic "accel_data/sensor1/peaks_a_z"
#define location_topic "accel_data/sensor1/location"


class button {                    // Creating class for button
  public:                         // Access modifier to make all members public
    int hndl;                     // Attributes that the class has (only blue print, not the data)
    int x;                        // All the variables needed to create a button
    int y;
    int w;
    int h;
    int buttonColour;
    String text;
    int fontID;
    int txtColour;

    // Constructor "button" that passes all the needed variables for creation as parameters
    button(int hndl, int x, int y, int w, int h, int buttonColour, String text, int fontID, int txtColour)
    {
      this-> hndl = hndl;
      this-> x = x;
      this-> y = y;
      this-> w = w;
      this-> h = h;
      this-> buttonColour = buttonColour;
      this-> text = text;
      this-> fontID = fontID;
      this-> txtColour = txtColour;
    }

    // Class method init() to create the button
    void init() {
      gfx.Buttonx(hndl, x, y, w, h, buttonColour, text, fontID, txtColour);
    }
};

// Button objects
// Bellow I create all the button objects used in the program using "button" constructor created above in the class
// Main screen
button button1 = button(1, 95, 70, 140, 40, BLUE, "VELOCITY", 2, BLACK);              // VELOCITY
button button2 = button(2, 95, 120, 140, 40, GREEN, "FREQUENCY PEAKS", 2, BLACK);      // FREQUENCY PEAKS
button button3 = button(3, 95, 170, 140, 40, ORANGE, "LOCATION", 2, BLACK);             // SENSOR LOCATION


// Variables
// Variables for checking if button is pressed
int reading;
int last_reading;
int but;

// Array for MQTT data 
char veloc_x[10];
char veloc_y[10];
char veloc_z[10];
char veloc_categ_x[2];
char veloc_categ_y[2];
char veloc_categ_z[2];
char freq_x[10];
char freq_y[10];
char freq_z[10];
char amp_x[10];
char amp_y[10];
char amp_z[10];
char local[15];

// Callback function which sorts out data that comes through via MQTT 
// And puts it to an array
// Each measurement has a letter at the beginning which the Python program sends
// And that's how the callback function knows which data comes through
void callback(char* topic, byte* payload, unsigned int length) {
  for (int i=1;i<length;i++) {
    if ((char)payload[0] == 'x') {
      veloc_x[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new velocity data");
    }
    if ((char)payload[0] == 'y') {
      veloc_y[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new velocity data");
    }
    if ((char)payload[0] == 'z') {
      veloc_z[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new velocity data");
    }
    if ((char)payload[0] == 'b') {
      veloc_categ_x[0] = (char)payload[1];
    }  
    if ((char)payload[0] == 'c') {
      veloc_categ_y[0] = (char)payload[1];
    } 
    if ((char)payload[0] == 'd') {
      veloc_categ_z[0] = (char)payload[1];
    } 
    
    if ((char)payload[0] == 'e') {
      freq_x[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new peaks data");
    }
    if ((char)payload[0] == 'f') {
      freq_y[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new peaks data");
    }
    if ((char)payload[0] == 'g') {
      freq_z[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new peaks data");
    }
    
    if ((char)payload[0] == 'h') {
      amp_x[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new peaks data");
    }
    if ((char)payload[0] == 'i') {
      amp_y[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new peaks data");
    }
    if ((char)payload[0] == 'j') {
      amp_z[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new peaks data");
    }
    
    if ((char)payload[0] == 'l') {
      local[i] = (char)payload[i];
      // Notify user about new data
      gfx.TextSize(1);
      gfx.MoveTo(200, 20);
      gfx.println("new location data");
    }  
  }
}

// Connect to WiFi
void connectWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.softAP(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.print(WiFi.channel());
  Serial.println(WiFi.localIP());
}

// Connnects to MQTT client and subscribes to all topics 
void connectMQTT()
{
  while (!client.connected())
  {
    String clientId = ".";
    clientId += String(random(0xf), HEX);
    Serial.printf("MQTT connecting as client %s...\n", clientId.c_str());
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_password))
    // if (client.connect(clientId.c_str()))
    {
      Serial.println("MQTT connected");
      client.subscribe(velocity_x);
      client.subscribe(velocity_y);
      client.subscribe(velocity_z);
      client.subscribe(vel_categ_x);
      client.subscribe(vel_categ_y);
      client.subscribe(vel_categ_z);
      client.subscribe(peaks_ax_topic);
      client.subscribe(peaks_ay_topic);
      client.subscribe(peaks_az_topic);
      client.subscribe(peaks_fx_topic);
      client.subscribe(peaks_fy_topic);
      client.subscribe(peaks_fz_topic);
      client.subscribe(location_topic);
      
      Serial.println("Subscribed to all topics");
    } else
    {
      Serial.printf("MQTT failed, state %s, retrying...\n", client.state());
      delay(2500);
    }
  }
}

// Deletes menu
void delete_menu() {             
  gfx.DeleteButton(1, BLACK);
  gfx.DeleteButton(2, BLACK);
  gfx.DeleteButton(3, BLACK);
  gfx.Cls();
}

// Initializes menu buttons
void menu() {
  gfx.Cls();
  gfx.Rectangle(0,0,318,238, CYAN);
  gfx.Rectangle(1,1,317,237, CYAN);
  gfx.Rectangle(2,2,316,236, CYAN);
  gfx.TextSize(2);
  gfx.MoveTo(20, 20);
  gfx.println("SensorID: 1");               
  button1.init();
  button2.init();
  button3.init();
}

// Show location data 
char show_location() {
  gfx.Cls();
  gfx.Rectangle(0,0,318,238, PURPLE);
  gfx.Rectangle(1,1,317,237, PURPLE);
  gfx.Rectangle(2,2,316,236, PURPLE);
  gfx.TextSize(2);
  gfx.MoveTo(20, 20);
  gfx.print("Location of sensor: ");
  gfx.MoveTo(40, 60);
  for (int i=1;i<16;i++){    gfx.print(local[i]);  }
  delay(3000);
}

char show_peaks() {
  // Create table + frame around screen
  gfx.Cls();
  gfx.Line(107, 80, 107, 240, ORANGE);
  gfx.Line(214, 80, 214, 240, ORANGE);
  gfx.Line(53, 85, 53, 240, WHITE);
  gfx.Line(160, 85, 160, 240, WHITE);
  gfx.Line(267, 85, 267, 240, WHITE);
  gfx.Rectangle(0, 0, 318, 238, ORANGE);
  gfx.Rectangle(1, 1, 317, 237, ORANGE);
  gfx.Rectangle(2, 2, 316, 236, ORANGE);
  gfx.TextSize(2);
  gfx.MoveTo(20, 20);
  gfx.print("Last 3 frequency peaks: ");

  // Row with axis
  gfx.TextColor(GREEN);
  gfx.MoveTo(53, 60);
  gfx.print("x");
  gfx.MoveTo(160, 60);
  gfx.print("y");
  gfx.MoveTo(267, 60);
  gfx.print("z");

  // Row with f[Hz] and a[m/s2]
  gfx.TextColor(WHITE);
  gfx.TextSize(1);
  gfx.MoveTo(13, 85);
  gfx.print("f[Hz]");
  gfx.MoveTo(59, 85);
  gfx.print("a[m/s2]");
  gfx.MoveTo(118, 85);
  gfx.print("f[Hz]");
  gfx.MoveTo(168, 85);
  gfx.print("a[m/s2]");
  gfx.MoveTo(226, 85);
  gfx.print("f[Hz]");
  gfx.MoveTo(273, 85);
  gfx.print("a[m/s2]");

  // Print data
  // Peak 1 - x, y, z
  int r1 = 96;
  gfx.TextColor(WHITE);
  gfx.TextSize(1);
  gfx.MoveTo(17, r1);
  for (int i=1;i<6;i++){    gfx.print(freq_x[i]);  }
  gfx.MoveTo(67, r1);
  for (int i=1;i<6;i++){    gfx.print(amp_x[i]);   }
  gfx.MoveTo(125, r1);
  for (int i=1;i<6;i++){    gfx.print(freq_y[i]);  }
  gfx.MoveTo(174, r1);
  for (int i=1;i<6;i++){    gfx.print(amp_y[i]);   }
  gfx.MoveTo(233, r1);
  for (int i=1;i<6;i++){    gfx.print(freq_z[i]);  }
  gfx.MoveTo(278, r1);
  for (int i=1;i<6;i++){    gfx.print(amp_z[i]);   }

  // Peak 2 - x, y, z
    int r2 = 106;
  gfx.TextColor(WHITE);
  gfx.TextSize(1);
  gfx.MoveTo(17, r2);
  gfx.print("123");
  for (int i=6;i<11;i++){    gfx.print(freq_x[i]);  }
  gfx.MoveTo(67, r2);
  for (int i=6;i<11;i++){    gfx.print(amp_x[i]);   }
  gfx.MoveTo(125, r2);
  for (int i=6;i<11;i++){    gfx.print(freq_y[i]);  }
  gfx.MoveTo(174, r2);
  for (int i=6;i<11;i++){    gfx.print(amp_y[i]);   }
  gfx.MoveTo(233, r2);
  for (int i=6;i<11;i++){    gfx.print(freq_z[i]);  }
  gfx.MoveTo(278, r2);
  for (int i=6;i<11;i++){    gfx.print(amp_z[i]);   }

  // Peak 3 - x, y, z
    int r3 = 116;
  gfx.TextColor(WHITE);
  gfx.TextSize(1);
  gfx.MoveTo(17, r3);
  for (int i=11;i<16;i++){    gfx.print(freq_x[i]);  }
  gfx.MoveTo(67, r3);
  for (int i=11;i<16;i++){    gfx.print(amp_x[i]);   }
  gfx.MoveTo(125, r3);
  for (int i=11;i<16;i++){    gfx.print(freq_y[i]);  }
  gfx.MoveTo(174, r3);
  for (int i=11;i<16;i++){    gfx.print(amp_y[i]);   }
  gfx.MoveTo(233, r3);
  for (int i=11;i<16;i++){    gfx.print(freq_z[i]);  }
  gfx.MoveTo(278, r3);
  for (int i=11;i<16;i++){    gfx.print(amp_z[i]);   }
  
  delay(3000);
}

// Display x, y, z velocity 
char show_velocity() {
  // Make a frame around the screen
  gfx.Cls();
  gfx.Rectangle(0,0,318,238, LIME);
  gfx.Rectangle(1,1,317,237, LIME);
  gfx.Rectangle(2,2,316,236, LIME);
  gfx.TextSize(2);
  gfx.MoveTo(20, 20);
  gfx.print("Last velocity readings: ");
  gfx.MoveTo(55, 60);
  gfx.print("x");
  gfx.MoveTo(45, 100);
  for (int i=1;i<10;i++){    gfx.print(veloc_x[i]);  }  
  gfx.MoveTo(140, 60);
  gfx.print("y");
  gfx.MoveTo(125, 100);
  for (int i=1;i<10;i++){    gfx.print(veloc_y[i]);  }
  gfx.MoveTo(220,60);
  gfx.print("z");
  gfx.MoveTo(205, 100);
  for (int i=1;i<10;i++){    gfx.print(veloc_z[i]);  }
}

// Based on data from MQTT, choose colour that should be displayed on the velocity screen
void veloc_category(int a, int b, int c, int d, char* data) {
  if (data[0] == 'A') {
    gfx.Rectangle(a, b, c, d, GREEN);
  }
  if (data[0] == 'B') {
    gfx.Rectangle(a, b, c, d, YELLOW);
  }
  if (data[0] == 'C') {
    gfx.Rectangle(a, b, c, d, ORANGE);
  }
  if (data[0] == 'D') {
    gfx.Rectangle(a, b, c, d, RED);
  }
}


void setup() {
  Serial.begin(115200);
  // WiFi + MQTT setup
  connectWifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  // Screen setup
  gfx.begin();                            // Initial settings for the screen
  gfx.Cls();                              // Initial settings for the screen
  gfx.ScrollEnable(false);                // Initial settings for the screen
  gfx.BacklightOn(true);                  // Initial settings for the screen
  gfx.Orientation(LANDSCAPE);             // Initial settings for the screen
  gfx.touch_Set(TOUCH_ENABLE);            // Initial settings for the screen

  // Initial screen setup
  // Create frame around screen
  gfx.Rectangle(0,0,318,238, CYAN);
  gfx.Rectangle(1,1,317,237, CYAN);
  gfx.Rectangle(2,2,316,236, CYAN);

  gfx.TextSize(2);
  gfx.MoveTo(20, 20);
  gfx.println("SensorID: 1");             // SENSOR 1
  button1.init();                         // Initialize main menu buttons
  button2.init();                         // Initialize main menu buttons
  button3.init();                         // Initialize main menu buttons
}


void loop() {
  // Connect to Rpi broker
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Check buttons state
  reading = gfx.CheckButtons();                     
  if (reading != last_reading && reading > 0) {     // A massive if statement that checks if any of the buttons are being pressed
    but = reading;

    // The numbers are button IDs assigned in the class objects
    if (but == 1) {               // VELOCITY
      delete_menu();              // Deletes menu buttons
      show_velocity();
      veloc_category(40, 50, 85, 85, veloc_categ_x);
      veloc_category(120, 50, 165, 85, veloc_categ_y);
      veloc_category(200, 50, 245, 85, veloc_categ_z);
      delay(2000);
      menu();                       // Initializes menu buttons again
    }

    if (but == 2) {               // FREQUENCY PEAKS
      delete_menu();              // Deletes menu buttons
      show_peaks();
      menu();                     // Initializes menu buttons again
    }

    if (but == 3) {               // SEE LOCATION
      delete_menu();              // Deletes menu buttons
      show_location();
      menu();                     // Initializes menu buttons again
    }
  }
  else {
    but = 0;
  }

  last_reading = reading;
  yield();                         // This is requiered for the ESP8266
}
