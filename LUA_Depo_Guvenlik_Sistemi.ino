#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Servo.h>

#define SDA_PIN 10
#define SCL_PIN 13
#define RST_PIN 5
#define SERVO_PIN 9
#define BUZZER_PIN A3
#define NOTE_DO 2000
#define NOTE_ME 8000

Servo servo;
MFRC522 rfid(SDA_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
char* correctPin = "1234";
bool doorIsOpen = false;
String allowedCardID1 = "Ilk kartinizin UID'si buraya";
String allowedCardID2 = "Ikinci kartinizin UID'si buraya";   //<===========  Kartların UID'si buradan girilmeli
String allowedCardID3 = "Ucuncu kartinizin UID'si buraya";
String allowedCardID4 = "Dorduncu kartinizin UID'si buraya";
boolean scrollTextActive = true;
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {12, 2, 3, 4};
byte colPins[COLS] = {6, 7, 8};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
int failedAttempts = 0;
bool alarmEnabled = false;
//----------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  servo.attach(9);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  HOSGELDINIZ");
  lcd.setCursor(0, 1);
  lcd.print("  KART OKUTUN");
  pinMode(BUZZER_PIN, OUTPUT);
  servo.write(0);
  // Seri iletişimi başlatın
  Serial.begin(9600);
}
//----------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  // RFID kartı okuma
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String cardID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      cardID += String(rfid.uid.uidByte[i], HEX);
    }
    cardID.toUpperCase();
    Serial.println("Kart ID: " + cardID);
    rfid.PICC_HaltA();
   if (cardID == allowedCardID1 || cardID == allowedCardID2 || cardID == allowedCardID3 || cardID == allowedCardID4){  
      tone(BUZZER_PIN, NOTE_DO);
      delay(250);
      tone(BUZZER_PIN, NOTE_ME);
      delay(260);
      noTone(BUZZER_PIN);
      toggleDoor();
      failedAttempts=0;
    } 
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("     ERISIM");
      lcd.setCursor(0, 1);
      lcd.print("   ENGELLENDI");
      tone(BUZZER_PIN, NOTE_DO);
      delay(133);
      noTone(BUZZER_PIN);
      tone(BUZZER_PIN, NOTE_DO);
      delay(133);
      noTone(BUZZER_PIN);
      tone(BUZZER_PIN, NOTE_DO);
      delay(133);
      noTone(BUZZER_PIN);
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("LUTFEN KARTINIZI");
      lcd.setCursor(0, 1);
      lcd.print("    OKUTUNUZ");  
      if (!alarmEnabled) {
        failedAttempts++;
        checkAlarm();
        scrollTextActive = true;
      }
    }
  }
  // Keypad ile kapıyı kontrol etme
  char customKey = keypad.getKey();
  if (customKey == '#') {
    tone(A3, 2550, 50);
    delay(100);
    tone(A3, 2550, 50);
    // Keypad'den "#" tuşuna basıldığında kapıyı kontrol et
    if (!alarmEnabled) {
      checkDoorCode();
    }
  } else if (customKey == '*') {
    // "*" tuşu işleme alınabilir
  }
  checkAlarm();
}
//----------------------------------------------------------------------------------------------------------------------------------------------
void toggleDoor() {
  if (doorIsOpen) {
    closeDoor();
  } else {
    openDoor();
  }
}
//----------------------------------------------------------------------------------------------------------------------------------------------
void openDoor() {
  servo.write(90);
  doorIsOpen = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  KAPI ACILDI");
  lcd.setCursor(0, 1);
  lcd.print("  HOSGELDINIZ");
  delay(5000);
  scrollTextActive = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  const char* longText = "KAPIYI KILITLEMEK ICIN LUTFEN SIFRE GIRIN YA DA KARTINIZI OKUTUN ";
  scrollText(longText, 150); // Yazının kayma hızını ayarlayabilirsiniz.
}
//----------------------------------------------------------------------------------------------------------------------------------------------
void closeDoor() {
  servo.write(0);
  doorIsOpen = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  KAPI KAPANDI");
  lcd.setCursor(0, 1);
  lcd.print("   IYI GUNLER");
  delay(5000);
  scrollTextActive = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  const char* longText = "KAPI ACMAK ICIN LUTFEN SIFRE GIRIN YA DA KARTINIZI OKUTUN ";
  scrollText(longText, 150); // Yazının kayma hızını ayarlayabilirsiniz.
}
//----------------------------------------------------------------------------------------------------------------------------------------------
void checkDoorCode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SIFREYI GIRIN:");
  char enteredPin[5];
  int pinIndex = 0;
  while (pinIndex < 4) {
    char key = keypad.getKey();
    if (key != NO_KEY) {
      tone(A3, 2050, 100);
      lcd.setCursor(pinIndex, 1);
      lcd.print('*');
      enteredPin[pinIndex] = key;
      pinIndex++;
      delay(200);
    }
  }
  enteredPin[4] = '\0';
  if (strcmp(enteredPin, correctPin) == 0) {
    lcd.clear();
    lcd.setCursor(0, 0 );
    lcd.print("  SIFRE DOGRU!");
    tone(BUZZER_PIN, NOTE_DO);
  delay(250);
  tone(BUZZER_PIN, NOTE_ME);
  delay(260);
  noTone(BUZZER_PIN);
    delay(2000);
    toggleDoor();
    failedAttempts=0;
  } 
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  SIFRE YANLIS");
    tone(BUZZER_PIN, NOTE_ME);
    delay(250);
    tone(BUZZER_PIN, NOTE_DO);
    delay(260);
    noTone(BUZZER_PIN);
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LUTFEN KARTINIZI");
    lcd.setCursor(0, 1);
    lcd.print("    OKUTUNUZ");
    if (!alarmEnabled) {
      failedAttempts++;
      checkAlarm();
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------------------------------
void checkAlarm() {
  if (failedAttempts >= 200 && !alarmEnabled) {
    alarmEnabled = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!  COK SAYIDA  !");  
    lcd.setCursor(0, 1);
    lcd.print(" HATALI DENEME"); 
    for (int i = 0; i < 6; i++) { // Yaklaşık 20 kez dön
      // Alarm tonunu çal
      tone(BUZZER_PIN, 1000, 500); // Alarm tonu çal
      delay(550); // 0.5 saniye boyunca çal
      tone(BUZZER_PIN, 1000, 500); // Alarm tonu çal
      delay(550); // 0.5 saniye boyunca sessiz kal
    }
    alarmEnabled = false;
    failedAttempts = 0;
    const char* longText2 = "10 DAKIKA SONRA TEKRAR DENEYIN! ";
    scrollText(longText2, 150); // Yazının kayma hızını ayarlayabilirsiniz.
    delay(10000);
    return setup();
  }
}
//----------------------------------------------------------------------------------------------------------------------------------------------
void scrollText(String text, int speed) {
  int textLength = text.length();
  while (scrollTextActive) {
    for (int i = 0; i < textLength + 16; i++) {
      if (!scrollTextActive) {
        break; // Eğer kayan yazı devre dışı bırakıldıysa döngüyü kır
      }
      // Kart okunduğunda ya da '#' tuşuna basıldığında döngüyü sonlandır
      if (rfid.PICC_IsNewCardPresent() || keypad.getKey() == '#') {
        scrollTextActive = false;
        return;
      }
      lcd.clear(); // Ekranı temizle
      int endIndex = min(i, textLength);
      int startIndex = max(0, i - 16);
      String displayText = text.substring(startIndex, endIndex);
      lcd.setCursor(0, 0);
      lcd.print(displayText);
      delay(speed); // Kayan hızı
    }
    // Kayan yazı sona ulaştığında başa dönmesini sağlayan bir bekleme süresi ekleyin
    delay(100); // Örneğin, 2 saniye bekleme
  }
}
