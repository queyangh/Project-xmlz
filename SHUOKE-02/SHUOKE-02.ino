#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <dht11.h>

const int trigPin = D6;  
const int echoPin = D7;  

const char* ssid = "CX";
const char* password = "8888899999";

const char* mqttServer = "mqtt.sszx.tech";
const int mqttPort = 1883;
const char* mqttUser = "xm6z";
const char* mqttPassword = "05678537";

WiFiClient espClient;
PubSubClient client(espClient);

dht11 DHT11;
#define DHT11PIN 2
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long previousMillis = 0;
const long interval = 800; // 发送数据的间隔
unsigned long mistStartTime = 0;
bool misting = false;
const unsigned long mistDuration = 5000; // 雾化器工作时间（毫秒）

#define buttonPin D0
// const int buttonPin = D0; // 定义按钮引脚
bool loopControl = true; // 定义工作状态变量，true为闭环控制，false为开环控制

void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(8);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  
  Serial.begin(115200);
  pinMode(D3, OUTPUT);
  digitalWrite(D3, LOW);
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP); // 初始化按钮引脚，启用内部上拉电阻

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  int button_state = digitalRead(buttonPin);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // 读取超声波距离
    int distance = measureDistance(trigPin, echoPin);
    
    // 读取DHT11传感器温湿度
    DHT11.read(DHT11PIN);
    float h = DHT11.humidity;
    float t = DHT11.temperature;

    Serial.println("\n");
    Serial.print("当前湿度 (%): ");
    Serial.println(h);

    Serial.print("当前温度 (℃): ");
    Serial.println(t);

    // 在OLED上显示数据
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);
    display.print("Temp: ");
    display.print(int(t));
    display.print("C");
    display.println();
    display.print("Hum: ");
    display.print(int(h));
    display.print("%");
    display.println();
    display.println();
    display.print("Dis: ");
    display.print(distance);
    display.print("cm");
    display.display();

    // 发布数据到MQTT
    char payload[50];
    sprintf(payload, "{\"temperature\": %.2f, \"humidity\": %.2f, \"distance\": %d}", t, h, distance);
    client.publish("温湿度数据", payload);
    Serial.println("Data published");

    // 检查按钮状态，切换工作模式
    // if (digitalRead(buttonPin) == LOW) { // 再次检查确保按钮确实被按下
    //   loopControl = false; // 切换工作状态
    //   Serial.println("工作模式切换");
    // }else if(digitalRead(buttonPin) == HIGH){
    //   loopControl = true;
    // }

    // 根据工作状态执行不同的逻辑
    if (button_state == 1) {
      // 闭环控制逻辑
      if (h < 60) {
        digitalWrite(D5, HIGH); // 启动雾化器
        digitalWrite(D3, LOW);  // 关闭风扇
        Serial.println("湿度低于60%，启动加湿器");
      } else {
        digitalWrite(D5, LOW);  // 关闭雾化器
        digitalWrite(D3, HIGH); // 启动风扇
        Serial.println("湿度高于等于60%，关闭加湿器");
      }
    } else {
      // 开环控制逻辑
      if (distance < 7) {
        if (!misting) {
          misting = true;
          mistStartTime = currentMillis;
          digitalWrite(D5, HIGH); // 启动雾化器
          Serial.println("距离小于7cm，启动加湿器");
        }
      }
      if (misting && (currentMillis - mistStartTime >= mistDuration)) {
        misting = false;
        digitalWrite(D5, LOW); // 停止雾化器
        Serial.println("加湿器停止");
      }
    }
  }
}

// 超声波测距函数
int measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2; // 计算距离并取整
  return distance;
}

// 连接WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT回调函数
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// 重连MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.subscribe("温湿度/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
