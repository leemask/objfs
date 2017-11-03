#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <stdlib.h>
#include <stdio.h>
#include <SoftwareSerial.h>

#define DEBUG

#define WIFI_NAME "EFTech"
#define WIFI_PASS "20140924"

/* Use WiFiClient class to create TCP connections */
ESP8266WiFiMulti WiFiMulti;
WiFiClient _client;

unsigned int _okCount = 0;
unsigned int _failCount = 0;

/* SERVER INFORMATION */
const uint16_t port = 3300;
const char * SERVER_IP = "192.168.0.36"; // ip or dns
const char* SERVER_PAGE = "/testJm";
int connection_check = 0;

SoftwareSerial nucleo(D1, D2); //RX, TX
char ch;
String recv_data;
int send_data;
char temp[64];
String cmd;

int blink = LOW;
int test = 5;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("\nProgram Starts");
  nucleo.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  WiFiMulti.addAP(WIFI_NAME, WIFI_PASS);

  Serial.println();
  Serial.print("Wait for WiFi... ");
  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, blink);
    blink = !blink;
    delay(100);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(100);
  
  Serial.print("Connecting Server...");
  if (_client.connect(SERVER_IP, port)) {
    Serial.println("OK");
    connection_check = 1;
  }
  else {
    Serial.println("FAIL");
    connection_check = 0;
  }
  
  if (connection_check == 0) {
    while(1) {
      digitalWrite(LED_BUILTIN, blink);
      blink = !blink;
      delay(100);
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {  
  recv_data = "";
  cmd = "";
  ch = 0;
  Serial.print("Waiting Data > ");
  do {
    if(nucleo.available()) {
      ch = nucleo.read();
      recv_data.concat(ch);
    }
#ifdef DEBUG
    while(Serial.available()) {
      ch = Serial.read();
      cmd.concat(ch);
    }
    if (cmd != "") {
      cmd.concat('\n');
      ch = '\n';
    }
#endif
    delay(10);
  }while (ch != '\n');
  
  if (recv_data != "") {
    Serial.print(recv_data);
    memset(temp, 0x00, sizeof(temp));
    recv_data.substring(0, recv_data.length()).toCharArray(temp, recv_data.length());
    send_data = atoi(temp);
    Serial.print("Send Data (");
    Serial.print(send_data, DEC);
    Serial.print(")...");
    sendDataToServer(send_data);
    Serial.println("OK");
  }

#ifdef DEBUG
  if (cmd != "") {
    Serial.print(cmd);
    memset(temp, 0x00, sizeof(temp));
    cmd.substring(0, cmd.length()).toCharArray(temp, cmd.length());
    send_data = atoi(temp);
    Serial.print("Send Data (");
    Serial.print(send_data, DEC);
    Serial.print(")...");
    sendDataToServer(send_data);
    Serial.println("OK");
  }
#endif
}

void sendDataToServer(int n)
{
  char sending_string[128];    
  char out_buffer[256];
  
  sprintf(sending_string, "{\"data\":%d}", n);
  sprintf(out_buffer, "POST %s HTTP/1.1", SERVER_PAGE);
  _client.println(out_buffer);
  _client.println("Host: XXX");
  _client.println("User-Agent: Arduino/1.0");
  _client.println("Content-Type:application/json");
    
  sprintf(out_buffer, "Content-Length: %u", strlen(sending_string));
  _client.println(out_buffer);
  _client.println();
  _client.println(sending_string);
}
