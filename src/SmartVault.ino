#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo kilitServosu;

const byte SATIR = 4;
const byte SUTUN = 4;
char tuslar[SATIR][SUTUN] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte satirPinleri[SATIR] = {A0, A1, A2, A3};
byte sutunPinleri[SUTUN] = {10, 9, 8, 7};
Keypad keypad = Keypad(makeKeymap(tuslar), satirPinleri, sutunPinleri, SATIR, SUTUN);

const int YESIL_LED = 13;
const int KIRMIZI_LED = A5;

String mevcutSifre = "";
String girilenSifre = "";
const String MASTER_KOD = "0000";
const String PANIK_KODU = "9999";
int hataliGirisSayisi = 0;
bool sistemKilitli = false;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  kilitServosu.attach(6);
  kilitServosu.write(0);
  
  pinMode(YESIL_LED, OUTPUT);
  pinMode(KIRMIZI_LED, OUTPUT);

  int ilkByte = EEPROM.read(0);
  if (ilkByte == 255 || ilkByte == 0) {
     sifreyiHafizayaKaydet("1234");
  }
  
  mevcutSifre = sifreyiHafizadanOku();
  ekraniTemizle("Sifre Giriniz:");
}

void loop() {
  if (sistemKilitli) {
    digitalWrite(KIRMIZI_LED, HIGH);
    return;
  }

  char tus = keypad.getKey();

  if (tus) {
    if (tus == '#') {
       sifreyiKontrolEt();
    }
    else if (tus == '*') {
       girilenSifre = "";
       ekraniTemizle("Sifre Giriniz:");
    }
    else {
       girilenSifre += tus;
       lcd.print("*");
    }
  }
}

void sifreyiHafizayaKaydet(String sifre) {
  for (int i = 0; i < 4; i++) {
    EEPROM.write(i, sifre.charAt(i));
  }
}

String sifreyiHafizadanOku() {
  String okunan = "";
  for (int i = 0; i < 4; i++) {
    char k = EEPROM.read(i);
    okunan += k;
  }
  return okunan;
}

void ekraniTemizle(String mesaj) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(mesaj);
  lcd.setCursor(0, 1);
}

void lcd_yaz(String satir1, String satir2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(satir1);
  lcd.setCursor(0, 1);
  lcd.print(satir2);
}

void kapiyiAc(bool panikModu) {
  kilitServosu.write(90);
  digitalWrite(YESIL_LED, HIGH);
  
  if (panikModu) {
    Serial.println("!!! ALARM: POLIS CAGIRIN !!!");
    for(int i=0; i<5; i++) {
        digitalWrite(KIRMIZI_LED, HIGH); delay(100);
        digitalWrite(KIRMIZI_LED, LOW); delay(100);
    }
  }
  delay(3000);
  kilitServosu.write(0);
  digitalWrite(YESIL_LED, LOW);
  ekraniTemizle("Sifre Giriniz:");
}

void guvenlikKontrolu() {
  if (hataliGirisSayisi >= 3) {
    sistemKilitli = true;
    lcd_yaz("SISTEM KILITLENDI", "10 Sn Bekle...");
    delay(10000);
    sistemKilitli = false;
    hataliGirisSayisi = 0;
    ekraniTemizle("Sifre Giriniz:");
  }
}

void yeniSifreBelirle() {
  String yeni = "";
  while(true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') break;
      yeni += key;
      lcd.print(key);
    }
  }
  
  if (yeni.length() == 4) {
    sifreyiHafizayaKaydet(yeni);
    mevcutSifre = yeni;
    lcd_yaz("Sifre Degisti!", "");
    delay(2000);
  } else {
    lcd_yaz("HATA: 4 Hane", "Olmali");
    delay(2000);
  }
  ekraniTemizle("Sifre Giriniz:");
}

void sifreyiKontrolEt() {
  if (girilenSifre == mevcutSifre) {
    lcd_yaz("Giris Basarili", "");
    kapiyiAc(false);
    hataliGirisSayisi = 0;
  }
  else if (girilenSifre == PANIK_KODU) {
    lcd_yaz("Giris Basarili", "");
    kapiyiAc(true);
  }
  else if (girilenSifre == MASTER_KOD) {
    lcd_yaz("Yonetici Modu", "Yeni Sifre Yaz:");
    yeniSifreBelirle();
  }
  else {
    lcd_yaz("HATALI SIFRE!", "Tekrar Deneyin");
    digitalWrite(KIRMIZI_LED, HIGH);
    delay(1000);
    digitalWrite(KIRMIZI_LED, LOW);
    
    hataliGirisSayisi++;
    guvenlikKontrolu();
    
    ekraniTemizle("Sifre Giriniz:");
  }
  girilenSifre = "";
}