/* The easy circuit:
 *
 *                  Analog pin 0
 *                        |
 *    5V |-----/\/\/\-----+-----/\/\/\-----| GND
 *
 *               ^                ^ 
 *        10K thermistor     10K resistor
 *
 * The advanced circuit:
 *
 *          AREF      Analog pin 0
 *           |              |
 *    3.3V |-+---/\/\/\-----+-----/\/\/\-----| GND
 *
 *                 ^                ^ 
 *          10K thermistor     10K resistor
 */



#include <LiquidCrystal.h>// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h> 
 
Servo motor;

#define SS_PIN 10
#define RST_PIN 9
// Definicoes pino modulo RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); 

char st[20];

#define gas_pin   A0 // choose the input pin (for GAS sensor)  
#define flame_pin A1 // choose the input pin (for Flame sensor) 
#define temp_pin  A2 // choose the input pin (for Temp sensor) 
 
#define e_s1 A3 //echo pin
#define t_s1 A4 //Trigger pin
#define USONIC_DIV 0.034
#define buzzer A5 // choose the pin for the Buzzer


int cm, flame, temp, gas; // variable for reading the gaspin status

int Vo;
float R1 = 10000; // value of R1 on board
float logR2, R2, T;
//steinhart-hart coeficients for thermistor
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;  

int set1 = 900; // we start, assuming Smoke detected
int set2 = 45;  // we start, assuming Temp detected
int set3 = 15;  // we start, assuming CM detected

void setup() {
Serial.begin(9600); // Inicia a serial

pinMode(temp_pin,INPUT); // declare sensor as input
pinMode(gas_pin, INPUT);  // declare sensor as input
pinMode(flame_pin,  INPUT);   // declare sensor as input

pinMode(buzzer,OUTPUT); // declare Buzzer as output 

pinMode(e_s1,INPUT); 
pinMode(t_s1,OUTPUT);
///////

//////
motor.attach(8); // Define que o servo esta ligado a porta digital 3
motor.write(90); // Move o servo para a posicao inicial (cancela fechada)

SPI.begin(); // Inicia  SPI bus
mfrc522.PCD_Init(); // Inicia MFRC522

lcd.begin(16, 4);
lcd.clear();
lcd.setCursor(0,0);
lcd.print("   WELCOME To   ");
lcd.setCursor(0,1);
lcd.print("");
delay(2000);
lcd.clear();
}

void loop() {
cm = ultra_read(t_s1,e_s1);  
gas = analogRead(gas_pin);
flame = digitalRead(flame_pin);
////////////////////////
long duration;
float distance;
int percentage;
duration=pulseIn(e_s1,HIGH);
distance= (float)duration*USONIC_DIV/2;
cm=map(distance, 14, 4, 0, 100);

if(cm < 0)
{
  cm = 0;
  }
  else if(cm>100)
  {
    cm =100;
    }

Vo = analogRead(temp_pin);
R2 = R1 * (1023.0 / (float)Vo - 1.0); //calculate resistance on thermistor
logR2 = log(R2);
temp = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // temperature in Kelvin
temp = temp - 273.15; //convert Kelvin to Celcius
// T = (T * 9.0)/ 5.0 + 32.0; //convert Celcius to Fahrenheit

lcd.setCursor(0, 0);
lcd.print("Smoke:");
lcd.print(gas);
lcd.print("  ");

lcd.setCursor(0, 1);
lcd.print("Temp:");
lcd.print(temp);
lcd.write(223);
lcd.print("C  ");

lcd.setCursor(0, 2);
lcd.print("Flame:");
lcd.print(flame);

lcd.setCursor(10, 2);
lcd.print("CM:");
lcd.print("Stock 100%");
lcd.print(cm);

lcd.print("  ");


/////////////////////////****************/////////////////


////////////////////////*****************/////////////////
if(flame==0){ // check if the Fire variable is LOW
lcd.setCursor(0, 3);
lcd.print("Flame Alert..!!!");  
digitalWrite(buzzer, HIGH); // Turn LED on.
}

if(gas>set1){ // check if the Smoke variable is High
lcd.setCursor(0, 3);
lcd.print("Smoke Alert..!!!");    
digitalWrite(buzzer, HIGH); // Turn LED on. 
}

if(temp>set2){ // check if the Temp variable is High
lcd.setCursor(0, 3);
lcd.print("Temp Alert...!!!");    
digitalWrite(buzzer, HIGH); // Turn LED on.
}

if(cm>set3){ // check if the Fire variable is LOW
lcd.setCursor(0, 3);
lcd.print("Theft Alert..!!!");  
digitalWrite(buzzer, HIGH); // Turn LED on.
}
if(gas<set1 && temp<set2 && flame==1 && cm<set3){ // check if the Smoke variable is Low
lcd.setCursor(0, 3);
lcd.print(".....Normal.....");
digitalWrite(buzzer, LOW); // Turn LED on.
}  

Serial.print("Smoke: ");Serial.println(gas);
Serial.print("Temp: "); Serial.println(temp);
Serial.print("Flame: ");Serial.println(flame);
Serial.print("Stock: "); Serial.println(cm);
Serial.print("Distance:");Serial.print(distance);
Serial.println("cm");

Serial.println();

delay(500);  
  // Aguarda a aproximacao do cartao
  if (!mfrc522.PICC_IsNewCardPresent()){return;}
  // Seleciona um dos cartoes
  if (!mfrc522.PICC_ReadCardSerial()){return;}
  // Mostra UID na serial
  Serial.print("UID da tag :");
  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase();
  
  // Testa se o cartao1 foi lido
  if (conteudo.substring(1) == "83 05 28 17"){// Levanta a cancela e acende o led verde
    motor.write(0);
    delay(3000);
    motor.write(90);
    }
    
  delay(100);
}

//**********************ultra_read****************************
int ultra_read(int pin_t,int pin_e){
digitalWrite(pin_t,LOW);
delayMicroseconds(2);
digitalWrite(pin_t,HIGH);
delayMicroseconds(10);
long time=pulseIn (pin_e,HIGH);
int ultra_time =  time / 29 / 2; 
delay(10);
return ultra_time;
}
