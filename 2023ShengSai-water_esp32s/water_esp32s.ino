#define e 12
#define d 14
#define c 27
#define h 26
#define b 22
#define a 23
#define g 19
#define f 18
#define buttonPin_1 15
#define buttonPin_2 2
#define buttonPin_system 32  //开闭环切换按钮引脚，另一端接GND
#define water_pin 35         //水位检测器模拟信号接受引脚
#define relay_pin 21         //继电器控制引脚
#define led_R 13
#define led_G 25
#define led_B 33

#define Trig 5 //引脚Tring 发射
#define Echo 34 //引脚Echo 接受
#define buttonPin_hcsr04 4   //超声波启用按钮，一端接GND
float cm; //距离变量
float temp; //
int counter = 0;
int water_val = 0;            //水位值
bool buttonState = HIGH;      // 当前按键状态
bool lastButtonState = HIGH;  // 上一次按键状态

//bool system_state; //开环还是闭环，开环-没按-HIGH, 闭环-按下-LOW

unsigned long lastDebounceTime = 0;  // 上一次按键变化的时间
unsigned long debounceDelay = 50;    // 按键防抖动的延迟时间（毫秒
void setup() {
  // put your setup code here, to run once:
  pinMode(e, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(h, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(a, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(buttonPin_2, INPUT_PULLUP);
  pinMode(buttonPin_1, INPUT_PULLUP);
  pinMode(buttonPin_system, INPUT_PULLUP);
  pinMode(buttonPin_hcsr04, INPUT_PULLUP);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(relay_pin, OUTPUT);
  pinMode(led_R, OUTPUT);
  pinMode(led_G, OUTPUT);
  pinMode(led_B, OUTPUT);
  Serial.begin(9600);
  color(255, 255, 255);
  //数码管自检0~9
  Self_test();
}

void loop() {

  int system_state = digitalRead(buttonPin_system);
  //int system_state = 0;
  //Serial.println(system_state);
  if (system_state == 1) {
    //如果system_state == 1 ,则表示当前处于开环控制状态

    color(0, 255, 255); //红色
    digitalWrite(relay_pin, LOW);
    Serial.println("open-loop");
    int reading = digitalRead(buttonPin_2);
    int reading_buttonPin_1 = digitalRead(buttonPin_1);
    int sr04_state = digitalRead(buttonPin_hcsr04);
    //int sr04_state = 0;
    if (sr04_state == LOW) { //监测到超声波按钮被按下
      number_None();
      color(255, 255, 0);
      float cm_result = hc_sr04();
      Serial.print("Distance = ");
      Serial.print(cm_result);//串口输出距离换算成cm的结果
      Serial.println("cm");
      if (cm_result <= 6) { //物体靠近，启动抽水
        digitalWrite(relay_pin, HIGH);
      } else {
        digitalWrite(relay_pin, LOW);
      }
    } else {
      // 检测按键状态是否发生变化
      if (reading != lastButtonState) {
        // 记录按键状态变化的时间
        lastDebounceTime = millis();
      }
      // 检测是否超过防抖动延迟时间
      if ((millis() - lastDebounceTime) > debounceDelay) {
        // 如果当前状态和上次状态不同，说明按键状态稳定
        if (reading != buttonState) {
          buttonState = reading;

          // 按键按下，数字+1
          if (buttonState == LOW) {
            counter = (counter + 1) % 10;
            Serial.print("当前数字："); Serial.println(counter);
            updateDisplay(counter);
          }
        }

        //如果检测到按钮1，则开始倒计时，抽水
        if (reading_buttonPin_1 == LOW) {
          Serial.println("开始抽水");
          digitalWrite(relay_pin, HIGH);
          for (int i = counter; i >= 1; i--) {
            updateDisplay(i);
            Serial.print("倒计时："); Serial.println(i);
            delay(1000);
          }
          updateDisplay(0);
          digitalWrite(relay_pin, LOW);
          counter = 0;
        }
      }

      // 更新上一次按键状态
      lastButtonState = reading;
    }
  } else {
    //如果system_state == 0 ,则表示当前处于闭环控制状态
    Serial.println("闭环控制");
    color(255, 0, 255); //绿色
    number_None();
    counter = 0;
    water_val = analogRead(water_pin);
    //float water_val = 43;
    Serial.print("当前水位值:"); Serial.println(water_val);
    if (water_val < 40) {  // 如果 val 小于 10
      digitalWrite(relay_pin, HIGH);
      Serial.println("开始抽水");
    } else {
      Serial.println("水位以达到指定位置。");
      digitalWrite(relay_pin, LOW);
    }
  }
  delay(100);
}

void updateDisplay(int counter) {
  switch (counter) {
    case 0:
      number_0();
      break;
    case 1:
      number_1();
      break;
    case 2:
      number_2();
      break;
    case 3:
      number_3();
      break;
    case 4:
      number_4();
      break;
    case 5:
      number_5();
      break;
    case 6:
      number_6();
      break;
    case 7:
      number_7();
      break;
    case 8:
      number_8();
      break;
    case 9:
      number_9();
      break;
    default:
      number_None();
      break;
  }
}
void Self_test() {
  for (int i = 0; i < 10; i++) {
    updateDisplay(i);
    delay(400);
  }
  number_dot();
  delay(400);
  number_None();
}
void number_0() {
  digitalWrite(e, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(h, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(a, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, LOW);
}

void number_1() {
  digitalWrite(e, LOW);
  digitalWrite(d, LOW);
  digitalWrite(c, HIGH);
  digitalWrite(h, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(a, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
}

void number_2() {
  digitalWrite(e, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(c, LOW);
  digitalWrite(h, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(a, HIGH);
  digitalWrite(f, LOW);
  digitalWrite(g, HIGH);
}

void number_3() {
  digitalWrite(e, LOW);
  digitalWrite(d, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(h, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(a, HIGH);
  digitalWrite(f, LOW);
  digitalWrite(g, HIGH);
}

void number_4() {
  digitalWrite(e, LOW);
  digitalWrite(d, LOW);
  digitalWrite(c, HIGH);
  digitalWrite(h, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(a, LOW);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}
void number_5() {
  digitalWrite(e, LOW);
  digitalWrite(d, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(h, LOW);
  digitalWrite(b, LOW);
  digitalWrite(a, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}

void number_6() {
  digitalWrite(e, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(h, LOW);
  digitalWrite(b, LOW);
  digitalWrite(a, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}
void number_7() {
  digitalWrite(e, LOW);
  digitalWrite(d, LOW);
  digitalWrite(c, HIGH);
  digitalWrite(h, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(a, HIGH);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
}
void number_8() {
  digitalWrite(e, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(h, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(a, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}
void number_9() {
  digitalWrite(e, LOW);
  digitalWrite(d, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(h, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(a, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}
void number_dot() {
  digitalWrite(e, LOW);
  digitalWrite(d, LOW);
  digitalWrite(c, LOW);
  digitalWrite(h, HIGH);
  digitalWrite(b, LOW);
  digitalWrite(a, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
}

void number_None() {
  digitalWrite(e, LOW);
  digitalWrite(d, LOW);
  digitalWrite(c, LOW);
  digitalWrite(h, LOW);
  digitalWrite(b, LOW);
  digitalWrite(a, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
}
void color(unsigned char red, unsigned char green, unsigned char blue) //函数功能声明
{
  analogWrite(led_R, red);
  analogWrite(led_G, green);
  analogWrite(led_B, blue);
}

float hc_sr04() {
  digitalWrite(Trig, LOW); //给Trig发送一个低电平
  delayMicroseconds(2);    //等待 2微妙
  digitalWrite(Trig, HIGH); //给Trig发送一个高电平
  delayMicroseconds(10);    //等待 10微妙
  digitalWrite(Trig, LOW); //给Trig发送一个低电平

  temp = float(pulseIn(Echo, HIGH)); //存储回波等待时间,
  //pulseIn函数会等待引脚变为HIGH,开始计算时间,再等待变为LOW并停止计时
  //返回脉冲的长度

  //声速是:340m/1s 换算成 34000cm / 1000000μs => 34 / 1000
  //因为发送到接收,实际是相同距离走了2回,所以要除以2
  //距离(厘米)  =  (回波时间 * (34 / 1000)) / 2
  //简化后的计算公式为 (回波时间 * 17)/ 1000

  cm = (temp * 17 ) / 1000; //把回波时间换算成cm
  return cm;
}
