#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)


#define SEALEVELPRESSURE_HPA (1013.25)
const char* ssid = "<ssid>";  // Enter SSID here
const char* password = "<password>";  //Enter Password here
Adafruit_BMP280 bmp;

IPAddress local_IP(*.*.*.*);
// Set your Gateway IP address
IPAddress gateway(*.*.*.*);

IPAddress subnet(*.*.*.*);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

ESP8266WebServer server(80);
float temperature;
float maxTemp;
float minTemp;
char tempTrend;

void setup() {
  Serial.begin(115200);
  delay(500);
  startbmp280();
  setupNetwork();
  setupWebServer();
  getTemp();
  
  }

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
}


void startbmp280() {
    Serial.println("BMP280 Test");
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor");
    while (1);
  }
 
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
              Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
              Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
              Adafruit_BMP280::FILTER_X16,      /* Filtering. */
              Adafruit_BMP280::STANDBY_MS_500);
  
  
}

void setupNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupWebServer(){
  server.on("/", handle_OnConnect);
  server.on("/resetMaxMin", handle_resetMaxMin);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void getTemp(){
  Serial.print(F("Temperature = "));
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");
  maxTemp = bmp.readTemperature();
  minTemp = bmp.readTemperature();
  //tempTrend "Steady";
}
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void handle_OnConnect() {
  temperature = bmp.readTemperature();
  if (maxTemp < temperature){
    maxTemp = temperature;
  }
  if (minTemp > temperature){
    minTemp = temperature;
  }
  
  server.send(200, "text/html", sendHTML(temperature, maxTemp, minTemp));
}

void handle_resetMaxMin() {
  minTemp = temperature;
  maxTemp = temperature;
  handle_OnConnect();
}

String sendHTML(float temperature, float maxTemp, float minTemp){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  //ptr +="<meta http-equiv=\"refresh\" content=\"5\">\n";
  ptr +="<title>ESP8266 Weather Station</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;border: solid;border-radius: 10px;margin-left: auto;margin-right: auto;width: 200px}\n";
  ptr +="button {font-size: 24px;color: #444444;margin-bottom: 10px;border: solid;border-radius: 25px;margin-left: auto;margin-right: auto;width: 200px}\n";
  ptr +="</style>\n";
  ptr +="<script>\n";
  ptr +="setInterval(loadDoc,5000);\n";
  ptr +="function loadDoc() {\n";
  ptr +="var xhttp = new XMLHttpRequest();\n";
  ptr +="xhttp.onreadystatechange = function() {\n";
  ptr +="if (this.readyState == 4 && this.status == 200) {\n";
  ptr +="document.body.innerHTML =this.responseText}\n";
  ptr +="};\n";
  ptr +="xhttp.open(\"GET\", \"/\", true);\n";
  ptr +="xhttp.send();\n";
  ptr +="}\n";
  ptr +="function resetmaxmin() {\n";
  ptr +="var xhttp = new XMLHttpRequest();\n";
  ptr +="xhttp.onreadystatechange = function() {\n";
  ptr +="if (this.readyState == 4 && this.status == 200) {\n";
  ptr +="document.body.innerHTML =this.responseText}\n";
  ptr +="};\n";
  ptr +="xhttp.open(\"GET\", \"/resetMaxMin\", true);\n";
  ptr +="xhttp.send();\n";
  ptr +="}\n";
  ptr +="</script>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>ESP8266 Temperature Station</h1>\n";
  ptr +="<p>Temperature: ";
  ptr +=temperature;
  ptr +="&degC  </p>\n";
  ptr +="<div id=\"maxmin\">\n";
  ptr +="<p>Max Temperature: \n";
  ptr +=maxTemp;
  ptr +="&degC  </p>\n";
  ptr +="<p>Min Temperature: \n";
  ptr +=minTemp;
  ptr +="&degC  </p>\n";
  ptr +="</div>\n";
  ptr +="<button onmousedown=\"resetmaxmin()\">Reset the max and min records</button>\n";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
  }
