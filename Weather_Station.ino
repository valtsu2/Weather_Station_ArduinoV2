#include <LiquidCrystal.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN A1
#define DHTTYPE DHT11
#define se Serial
#define SAMPLES 10
#define SENDMINUTES 10.0f


float currentTime = 0;
float preTime = 0, preTime2 = 0;
float sendInterval = SENDMINUTES * 60 * 1000 ;
float calcInterval = sendInterval / SAMPLES;

float humidity, temperature, mh, mt;
float t[SAMPLES], h[SAMPLES];

LiquidCrystal lcd(13, 12, 7, 6, 5, 4);
DHT dht(DHTPIN, DHTTYPE); // create dht object
SoftwareSerial esp(2, 3);

void setup() {

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  dht.begin();
  lcd.print("waiting for WIFI");
  se.begin(115200);
  esp.begin(115200);
  se.println("SERIAL BEGIN");
  connectWifi();
  lcd.clear();
  //pin 8 is led
  pinMode(8, OUTPUT);
  //makes arrays all members 0
  emptyArrays();

}


void loop() {

  digitalWrite(8, false);
  readS();
  printLCD();
  delay(1000);
  currentTime = millis();
  float timer = currentTime - preTime;
  float timer2 = currentTime - preTime2;
  //lisää keskiarvoon numerot
  if (timer2 > calcInterval) {
    addValue();
  }
  if ((timer > sendInterval) && (t[SAMPLES - 1] != 0)) {
    digitalWrite(8, true);
    meanCalc();
    sendData();
    emptyArrays();
    preTime = currentTime;
  }
}

void connectWifi()
{
  int times = 0;
  //insert your wifi and pword here
  String wifiName = "";
  String pWord = "";
  String cmd = "AT+CWJAP=\"";
  cmd += wifiName;
  cmd += "\",\"";
  cmd += pWord;
  cmd += "\"";
  se.println(cmd);
  esp.println("AT+RST");
  delay(2000);
  esp.println("AT+CWMODE=3");
  delay(2000);
  se.println("Connecting to Wifi");
  while (true)
  {
    se.print(".");
    esp.println(cmd);
    esp.setTimeout(5000);
    if ((esp.find("WIFI CONNECTED\r\n") == 1) || (esp.find("WIFI CONNECTED" == 1)))
    {
      se.println("WIFI CONNECTED");
      break;
    }
    times++;
    if (times > 3)
    {
      times = 0;
      se.println("failed to connect, trying again");
    }
  }
}


void meanCalc() {
  mt = 0;
  mh = 0;
  for (int i = 0; i < SAMPLES; i++) {
    mt += t[i];
    mh += h[i];
  }
  mt = mt / SAMPLES;
  mh = mh / SAMPLES;
  se.print("mean temp is :");
  se.println(mt);
  se.print("mean hum is :");
  se.println(mh);
}


void readS() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

void printLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.println(" C     ");
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(humidity);
  lcd.print(" %");
}

void emptyArrays() {
  for (int i = 0; i < SAMPLES; i++) {
    t[i] = 0;
    h[i] = 0;
  }
}

void addValue() {
  static int index = 0;
  t[index] = temperature;
  h[index] = humidity;
  index++;
  if (index == SAMPLES) {
    index = 0;
  }
  se.println("values");
  for (int i = 0; i < SAMPLES; i++) {
    se.print(t[i]);
    se.print("    ");
    se.println(h[i]);
  }
  preTime2 = currentTime;
}

void sendData() {
  //Thingspeak write API key
  String key = "";
  // TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  esp.println(cmd);
  if (esp.find("Error")) {
    se.println("AT+CIPSTART error");
    return;
  }

  // prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += key;
  getStr += "&field1=";
  getStr += String(mt);
  getStr += "&field2=";
  getStr += String(mh);
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  esp.println(cmd);
  se.println(cmd);
  delay(1000);
  esp.print(getStr);
  se.print(getStr);
}
