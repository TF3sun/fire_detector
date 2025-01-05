#include <Servo.h>
#include <TM1637TinyDisplay.h>
#include "DHT.h"
#include <Adafruit_MLX90614.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define LED_RED 5
#define LED_GREEN 6
#define LED_BLUE  11
#define SERVO_PIN 8
#define PIEZO_BUZZER  3
#define CLK 9
#define DIO 10
#define BUTTON_1 4
#define BUTTON_2 7
#define VR_PIN  A0
#define BRIGHT_PIN  A1
#define DHTPIN 2

Servo myservo;
TM1637TinyDisplay display(CLK, DIO);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x3F, 16, 2);

int is_button1_clicked = 0;
int is_detected = 0;

void setup() {
  Serial.begin(9600);
  myservo.attach(SERVO_PIN);
  display.setBrightness(BRIGHT_7);
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  mlx.begin();
  dht.begin();
  lcd.init();
  lcd.backlight();
}

void loop() {
  if (Serial.available() > 0)
  {
    // 모델에서 감지 여부를 입력받음
    String strRead = Serial.readStringUntil('\n');
    // 감지된 경우 감지값을 true로 설정
    if (strRead.indexOf("Detect=True") != -1)
    {
      is_detected = 1;
    }
    // 감지가 끝난경우 감지값을 false로 설정
    else if (strRead.indexOf("Detect=False") != -1)
    {
      is_detected = 0;
    }
  }

  // 온도, 습도, 조도 밝기 값 변수
  int temp_data = dht.readTemperature();
  int humi_data = dht.readHumidity();
  float bright_data = analogRead(BRIGHT_PIN);
  float bright_per = bright_data/1000;
  
  // 센서값 읽기 테스트를 위한 코드
  Serial.print("temp : ");
  Serial.println(temp_data);
  Serial.print("humi : ");
  Serial.println(humi_data);
  Serial.print("bright : ");
  Serial.println(bright_data);
  Serial.print("bright per : ");
  Serial.println(bright_per);
  

  // 온도에 따른 제어문
  // 50도 이상인 경우 감지가 꺼져있지 않으면 동작
  if((temp_data > 50 || is_detected == 1) & is_button1_clicked == 0){
    int red_bright = 255*bright_per;
    redLedSet(red_bright, 0,0);

    lcd.setCursor(0, 0);
    lcd.print("Fire Detection");
    tone(PIEZO_BUZZER, 392);
    redLedSet(255,0,0);
  // 50도 이하인 경우 일반적인 LED, LCD 출력 제어
  }else if(50 > temp_data & temp_data >= 40){
    int red_bright = 255*bright_per;
    int green_bright = 255*bright_per;
    
    redLedSet(red_bright, green_bright,0);

    int temp100 = temp_data*100;
    int fnd_data = temp100+humi_data;
    display.showNumber(fnd_data);
   }else if(40 > temp_data & temp_data >= 30){
    int red_bright = 255*bright_per;;
    int green_bright = 10*bright_per;
    
    redLedSet(red_bright, green_bright ,0);

    int temp100 = temp_data*100;
    int fnd_data = temp100+humi_data;
    display.showNumber(fnd_data);
  }else{
    int green_bright = 255*bright_per;
    redLedSet(0,green_bright,0);

    int temp100 = temp_data*100;
    int fnd_data = temp100+humi_data;
    display.showNumber(fnd_data);
  }
  
  // 버튼의 클릭 여부에 따른 감지 On, Off 제어 및 감지 모델에 전송
  btn1();
  btn2();
  if (is_button1_clicked == 1) Serial.println("BUTTON1=CLICK");
  else Serial.println("BUTTON2=CLICK");

  delay(1000);
}

void redLedSet(int red, int green, int blue){
  analogWrite(LED_RED, red);
  analogWrite(LED_GREEN, green);
  analogWrite(LED_BLUE, blue);
}

void setBuzzer(int freq){
  if (freq > 20)
  {
    tone(PIEZO_BUZZER, freq);
    Serial.println("OKBUZZER");
  }
  else
  {
    noTone(PIEZO_BUZZER);
    Serial.println("OKBUZZER");
  }
}

void btn1(){
  int now_button = 0;
  int before_button = 0;
  
  now_button = digitalRead(BUTTON_1);

  if(now_button == 1){
    is_button1_clicked = 1;
    noTone(PIEZO_BUZZER);
    lcd.setCursor(0, 0);
    lcd.print("Fire Detection");
    lcd.setCursor(0,1);
    lcd.print("Is OFF");
  }
  
  delay(50);
}

void btn2(){
  int now_button = 0;
  int before_button = 0;
  
  now_button = digitalRead(BUTTON_2);

  if(now_button == 1){
    is_button1_clicked = 0;
    lcd.setCursor(0,0);
    lcd.print("Fire Detection");
    lcd.setCursor(0,1);
    lcd.print("Is ON ");
  }
  Serial.println(is_button1_clicked);
  delay(50);
}

void sendTemperature(){
  float temperature = dht.readTemperature();
  if (!isnan(temperature))
  {
    Serial.print("TEMPERATURE=");
    Serial.println(temperature);
  }
  else
  {
    Serial.print("TEMPERATURE=");
    Serial.println(0);
  }
}

void sendHumidity()
{
  float humidity = dht.readHumidity();
  if (!isnan(humidity))
  {
    Serial.print("HUMIDITY=");
    Serial.println(humidity);
  }
  else
  {
    Serial.print("HUMIDITY=");
    Serial.println(0);
  }
}
