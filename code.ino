#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x20, 16, 2);

#define RLED 12
#define BLED 13
#define BUZZ 3
#define LOCK 6
#define UNLOCK 5
#define K1 A1
#define K2 A2
#define K3 A3
#define SERVO 4

char* read_code;
char* master_code;
char *tag_code[50] = {"AB123456789A"};
short k;
short pin;
int code[] = {1, 2, 3, 4};
int typedcode[4];
int newcode[4];
int count = 1;
int index = 0;
Servo myservo;
int pos = 0;
int keypress = 0;


void setup() {
  Serial.begin(9600);
  pinMode(BLED, OUTPUT);
  pinMode(RLED, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(LOCK, INPUT);
  pinMode(UNLOCK, INPUT);
  pinMode(K1, INPUT);
  pinMode(K2, INPUT);
  pinMode(K3, INPUT);
  noTone(BUZZ);
  myservo.attach(SERVO);
  lcd.init();
  lcd.backlight();
  lcd.noBacklight();
  for (int i = 0 ; i < 4; i++) {
    EEPROM.put(i, code[i]);
  }
  for (pos = 180; pos >= 0; pos -= 1) {
    myservo.write(pos);     
    delay(5);            
  }
  idle();
}


void loop() {
  keypress = 0;
  if (Serial.available() > 0 or digitalRead(UNLOCK) == HIGH or (getkey() > 0 and keypress > 0))
  {
    if (digitalRead(UNLOCK) == HIGH) {
      unlockandlock();
      return;
    }
    if (keypress == 12) {
      Serial.println("Reset Mode");
      keypress = 0;
      reset();
      return;
    }
    if (keypress == 10) {
      Serial.println("Change pin mode");
      keypress = 0;
      changecode();
      return;
    }
    if (keypress != 0) {
      Serial.println("Code mode");
      keypress = 0;
      codemode();
      return;
    }
    else {
      read_code = readCard();
      Serial.print("read code : ");
      Serial.println(read_code);
      for (int i = 0; i < count; i++) {
        Serial.println(tag_code[i]);
        if (!(strcmp(read_code, tag_code[i])))
        {
          index = i;
          unlockandlock();
          return;
        }
      }
      locked();
      for (int i = 0; i < 10; i++) {
        delay(200);
        if (getkey() == 12) {
          Serial.println("Card registration mode");
          card_reg(read_code);
          idle();
          keypress = 0;
          return;
        }
      }
      idle();
    }
  }
  delay(100);
}

/*----------------------------------------------------------------------------------------*/
void idle() {
  digitalWrite(BLED, LOW);
  digitalWrite(RLED, LOW);
  Serial.println("");
  Serial.println("");
  Serial.println("Please scan your TAG");
  lcd.clear();
  lcd.println("  Scan your ID  ");
  lcd.setCursor(0, 1);
  lcd.print("   **Locked**   ");
}

/*----------------------------------------------------------------------------------------*/
char* readCard() {
  char* id = new char[13];
  id[12] = '\0';
  k = 0;
  while (true) {
    if (Serial.available()) {
      id[k] = Serial.read();
      k++;
      if (k == 12) {
        tone(BUZZ, 2000);
        delay(30);
        noTone(BUZZ);
        return id;
      }
    }
  }
}

/*----------------------------------------------------------------------------------------*/
void unlocked() {
  Serial.println("Access gained!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Access gained! ");
  lcd.setCursor(0, 1);
  lcd.print("  **Unlocked**  ");
  digitalWrite(BLED, HIGH);
  tone(BUZZ, 2000);
  delay(100);
  digitalWrite(BLED, LOW);
  noTone(BUZZ);
  delay(100);
  digitalWrite(BLED, HIGH);
  tone(BUZZ, 2000);
  delay(100);
  noTone(BUZZ);
  for (pos = 0; pos <= 180; pos += 1) {
    myservo.write(pos);
    delay(5);
  }
}

/*----------------------------------------------------------------------------------------*/
void locked() {
  digitalWrite(RLED, HIGH);
  Serial.println("Access denied!");
  lcd.setCursor(0, 0);
  lcd.print(" Access denied! ");
  digitalWrite(RLED, HIGH);
  tone(BUZZ, 1000);
  delay(300);
  digitalWrite(RLED, LOW);
  noTone(BUZZ);
  delay(100);
  digitalWrite(RLED, HIGH);
  tone(BUZZ, 1000);
  delay(300);
  digitalWrite(RLED, LOW);
  noTone(BUZZ);
  delay(100);
  digitalWrite(RLED, HIGH);
  tone(BUZZ, 1000);
  delay(300);
  digitalWrite(RLED, LOW);
  noTone(BUZZ);
}

/*----------------------------------------------------------------------------------------*/
void unlockandlock() {
  unlocked();
  while (digitalRead(LOCK) == LOW) {
    if (getkey() == 12 && index > 0) {
      Serial.println(index);
      Serial.println("Card remove mode");
      if (card_remove() > 0) {
        keypress = 0;
        index = 0;
      }
    }
    delay(100);
  }
  tone(BUZZ, 2000);
  delay(100);
  noTone(BUZZ);
  for (pos = 180; pos >= 0; pos -= 1) { 
    myservo.write(pos);              
    delay(5);                      
  }
  idle();
}

/*----------------------------------------------------------------------------------------*/
void card_reg(char* id) {
  lcd.setCursor(0, 0);
  lcd.print(" -Card reg mode-");
  for (int i = 0; i < 35; i++) {
    delay(200);
    if (Serial.available() > 0) {
      master_code = readCard();
      if (!(strcmp(master_code, tag_code[0]))) {
        master_code = NULL;
        tag_code[count] = id;
        count++;
        Serial.println("Successfully registered!");
        print1("Card Registered!");
        return;
      }
      Serial.println("Card not matched!");
      print1("Card not matched");
      return;
    }
  }
  Serial.println("Time Out!");
  print1("   Time Out!!   ");
  return;
}

/*----------------------------------------------------------------------------------------*/
int card_remove() {
  lcd.setCursor(0, 0);
  lcd.print("Card Remove Mode");
  Serial.println("Scan master  card to remove scanned card");
  for (int i = 0; i < 35; i++) {
    delay(200);
    if (Serial.available() > 0) {
      master_code = readCard();
      if (!(strcmp(master_code, tag_code[0]))) {
        tag_code[index] = NULL;
        master_code = NULL;
        Serial.println("Successfully removed!");
        print1(" Card Removed!! ");
        return true;
      }
      Serial.println("Card not matched!");
      print1("Card not matched");
      return true;
    }
  }
  Serial.println("Time Out!");
  print1("   Time Out!!   ");
  return true;
}

/*----------------------------------------------------------------------------------------*/
void print1(String msg) {
  lcd.setCursor(0, 0);
  lcd.print(msg);
  tone(BUZZ, 2000);
  delay(150);
  noTone(BUZZ);
  tone(BUZZ, 2000);
  delay(150);
  noTone(BUZZ);
  delay(500);
}

/*----------------------------------------------------------------------------------------*/
void reset() {
  Serial.println("Do you want to reset all?");
  Serial.println("Press * to confirm");
  lcd.setCursor(0, 0);
  lcd.print(" Are you sure?? ");
  for (int i = 0; i < 20; i++) {
    delay(200);
    if (getkey() == 10) {
      keypress = 0;
      lcd.setCursor(0, 0);
      lcd.print("Scan master card");
      Serial.println("Scan master card to reset");
      for (int i = 0; i < 20; i++) {
        delay(200);
        if (Serial.available() > 0) {
          master_code = readCard();
          if (!(strcmp(master_code, tag_code[0]))) {
            count = 1;
            for (int i = 0; i < 4; i++) {
              code[i] = i + 1;
              EEPROM.put(i, code[i]);
            }
            Serial.println("Successfully Reset!");
            print1(" Reset Success! ");
            master_code = NULL;
            idle();
            keypress = 0;
            return;
          }
          Serial.println("Card not matched!");
          print1("Card not matched");
          idle();
          keypress = 0;
          return;
        }
      }
      Serial.println("Time Out!");
      print1("   Time Out!!   ");
      idle();
      keypress = 0;
      return;
    }
  }
  Serial.println("Time Out!");
  print1("   Time Out!!   ");
  idle();
  return;
}
/*----------------------------------------------------------------------------------------*/
void codemode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Enter Password ");
  Serial.println("Enter password");
  if (ReadPin() > 0) {
    return;
  }
  for (int i = 0; i < 4; i++) {
    if (typedcode[i] != code[i]) {
      Serial.println("Wrong pin!!");
      print1(" Wrong pincode! ");
      idle();
      return;
    }
  }
  Serial.println("Pin matched!");
  unlockandlock();
  return;
}
/*----------------------------------------------------------------------------------------*/
void changecode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Old Password?? ");
  Serial.println("Enter old password");
  if (ReadPin() > 0) {
    return;
  }
  for (int i = 0; i < 4; i++) {
    if (typedcode[i] != code[i]) {
      Serial.println("Wrong pin!!");
      print1(" Wrong pincode! ");
      idle();
      return;
    }
  }
  Serial.println("Pin matched!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" New Password?? ");
  Serial.println("Enter new password");
  if (ReadPin() > 0) {
    return;
  }
  for (int i = 0; i < 4; i++) {
    newcode[i] = typedcode[i];
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Enter Again  ");
  Serial.println("Enter new password again");
  if (ReadPin() > 0) {
    return;
  }
  for (int i = 0; i < 4; i++) {
    if (typedcode[i] != newcode[i]) {
      Serial.println("Paswords are not matched!!");
      print1("  Not matched!  ");
      idle();
      return;
    }
  }
  for (int i = 0; i < 4; i++) {
    code[i] = typedcode[i];
    EEPROM.put(i, code[i]);
  }
  Serial.println("Pin changed succesfully!");
  print1(" Pin Changed!!! ");
  idle();
  return;
}
/*----------------------------------------------------------------------------------------*/
int ReadPin() {
  pin = 0;
  k = 5;
  while (pin != 4) {
    getkey();
    if (keypress == 10 or keypress == 12) {
      lcd.clear();
      Serial.println("Cancelled!!");
      print1("  Cancelled!!!  ");
      idle();
      keypress = 0;
      return 1;
    }
    else if (keypress > 0) {
      lcd.setCursor(k, 1);
      lcd.print("*");
      if (keypress == 11) {
        typedcode[pin] = 0;
        Serial.print("0");
      }
      else {
        typedcode[pin] = keypress;
        Serial.print(keypress);
      }
      keypress = 0;
      pin++;
      k = k + 2;
    }
    delay(50);
  }
  keypress = 0;
  Serial.println("");
  return 0;
}

/*----------------------------------------------------------------------------------------*/
int getkey() {
  int keyval = 0;
  int k1 = analogRead(K1);
  int k2 = analogRead(K2);
  int k3 = analogRead(K3);
  while (analogRead(K1) > 0 or analogRead(K2) > 0 or analogRead(K3) > 0) {
    delay(100);
  }
  if (k1 > 0 && k2 == 0 && k3 == 0) {
    if (k1 > 100) {
      keyval = 10;
      if (k1 > 350) {
        keyval = 7;
        if (k1 > 550) {
          keyval = 4;
          if (k1 > 900) {
            keyval = 1;
          }
        }
      }
    }
  }
  if (k2 > 0 && k1 == 0 && k3 == 0) {
    if (k2 > 100) {
      keyval = 11;
      if (k2 > 350) {
        keyval = 8;
        if (k2 > 550) {
          keyval = 5;
          if (k2 > 900) {
            keyval = 2;
          }
        }
      }
    }
  }
  if (k3 > 0 && k2 == 0 && k1 == 0) {
    if (k3 > 100) {
      keyval = 12;
      if (k3 > 350) {
        keyval = 9;
        if (k3 > 550) {
          keyval = 6;
          if (k3 > 900) {
            keyval = 3;
          }
        }
      }
    }
  }
  if (keyval != 0) {
    keypress = keyval;
    tone(BUZZ, 2000);
    delay(20);
    noTone(BUZZ);
  }
  return keyval;
}
