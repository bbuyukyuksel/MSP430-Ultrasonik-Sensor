#include "io430.h"
#include <stdlib.h>
#include "LCD.h"

#define Sensor1TRIG P2OUT_bit.P1        //1. Sensorun trigger pini P2.1 cikis
#define Sensor1ECHO P2IN_bit.P2         //1. Sensorun echo pini P2.2'de giris

#define Sensor2TRIG P2OUT_bit.P5        //2. Sensorun trigger pini P2.5 cikis
#define Sensor2ECHO P2IN_bit.P4         //2. Sensorun echo pini P2.4'te giris

#define FAILED 0
#define minOlcumDegeri 2                //Sensorun olctugu min deger
#define maxOlcumDegeri 400              //Sensorun olctugu max deger

#define SENSOR_1_ID 1                   //Olcum fonksiyonunda 1. sensorun olcum yapabilmesi icin verdigimiz ID degeri
#define SENSOR_2_ID 2                   //Olcum fonksiyonunda 2. sensorun olcum yapabilmesi icin verdigimiz ID degeri

#define Uzunluk		64 //cm                 //Kova uzunlugu

void pin_init(void);                    //Pin giris cikislari ayarlar
void timer_init(void);                  //Timer ayarlari yapar
void dco_init(void);                    //MSP'nin calisma frekansini ayarlar.
int SENSOR_DEGER_OKU(int Sensor_ID);    //Olcumleri yapacak Sensor

int main( void )
{ 
  WDTCTL = WDTPW + WDTHOLD;
  
  dco_init();
  pin_init();   // Sensorler icin pinleri ayarla
  timer_init(); // Timer Register'i için gerekli ayarlari yap
  
  lcd_start();  // LCD ayarlarini yap.
  lcd_clear();  // LCD'yi temizle
  
  lcd_go(1,5);
  lcd_string("AYARLAR");
  lcd_go(2,4);
  lcd_string("YAPILIYOR");
  
  __delay_cycles(500000); //500ms bekle
 
  while(1){
    int Uzaklik_1 = 0;
    int Uzaklik_2 = 0;
    
    while(1){ //Uzaklik Failed Olmadigi Surece Sensorden deger oku, failed degilse donguyu kir sonraki okumaya gec.
      Uzaklik_1 = SENSOR_DEGER_OKU(SENSOR_1_ID); 
      if(Uzaklik_1 != FAILED)
        break;
    }
    while(1){
      Uzaklik_2 = SENSOR_DEGER_OKU(SENSOR_2_ID);  //Uzaklik Failed Olmadigi Surece Sensorden deger oku, failed degilse donguyu kir ve devam et
      if(Uzaklik_2 != FAILED)
        break;
    }
    float oran = (Uzunluk / 100.0);                              //Kova Boyutu / 100 ile oranti degerimizi hesapladik.
    int Ort_Doluluk_Cm = ((Uzaklik_1 + Uzaklik_2) / 2.0);        //Olcum degerlerimizin ortalamasini aldik.
    
    if(Ort_Doluluk_Cm > Uzunluk)
      Ort_Doluluk_Cm = Uzunluk;   // Olculen Degerlerin ortalamasi kovamizin boyutundan buyukse, fark negatif cikmasin diye max uzunlugumuz olan kova boyutumuzu atiyoruz.
    
    int Gercek_Doluluk_Cm = Uzunluk - (Ort_Doluluk_Cm); //Gercek Doluluk Degeri
    int DolulukYuzdesi = (int)(Gercek_Doluluk_Cm / oran);  //Doluluk Yuzdesi Hesaplaniyor.
    
    lcd_clear();
    lcd_go(1,1);
    lcd_string("COP");
    lcd_go(2,1);
    lcd_string("SEVIYESI:    %");
    __delay_cycles(100);
    char String_DolulukYuzdesi[8];
    itoa(DolulukYuzdesi,String_DolulukYuzdesi);      //LCD'ye yazdirilmak icin sayisal deger, char arrayine (string'e) cevriliyor.
    lcd_string(String_DolulukYuzdesi);
   
    __delay_cycles(300000); //300ms bekle
  }
  return 0;
}
void dco_init(void){
  
  // MSP'nin calisma frekansi 1MHZ ayarlandi.
  BCSCTL1= CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
}

void timer_init(void){
  TA0CTL=0x00;                          //TIMERAO KONTROL REG.SIFIRLA
  TA0CTL= TASSEL_2 + MC_0 + ID_0;       //TIMERA0 DCO CLOCK , 1E BÖL, Surekli Sayim, SEKLINDE AYARLANDI.  
}
void pin_init(void){
  P2DIR = BIT1 | BIT5; //Iki Sensorunde Trigger uclari cikis olarak ayarlandi
  P2REN = BIT2 | BIT4; //Giris olarak ayarlanan sensorlerin P2REN ile ic direnclerin kullanilcagi soylendi.
  P2OUT = 0x00;        //Girisler icin Pull-Down direncleri, Cikislar icin cikisa 0V verildi.
}

int SENSOR_DEGER_OKU(int ID){
  int sure, mesafe;
 
  switch(ID){
  case SENSOR_1_ID :
    Sensor1TRIG=1;             //TRIGGER UCUNU BIR YAP 10 MIKROSANIYE BEKLE VE TEKRAR SIFIR YAP
    __delay_cycles(10);
    Sensor1TRIG=0;
    while(!(Sensor1ECHO)){  //Eger ki bir sekilde Echo ucu 1 olmazsa sonsuz donguden kurtarmak icin
      Sensor1TRIG = 1;      //Trig'e tekrar 1, sonra 0 ver.
     __delay_cycles(15);
      Sensor1TRIG = 0;
      __delay_cycles(15);
    }     // Sensor1ECHO PINI 1 OLANA KADAR BEKLE 
    
    TA0CTL |=MC_2;      // COUNT MODU CONTINIOUS OLARAK AYARLA
    while(Sensor1ECHO); // Sensor1ECHO PINI 0 OLANA KADAR BEKLE VE SAY
    TA0CTL ^=MC_2;      // SAYMA MODUNU DEVRE DISI
    sure=TA0R;          // SURE DEGISKENINE TA0R REGISTERINDEKI DEGER YÜKLENDI.    
    TA0R=0;             // REGISTERIN DEGERINI 0'LA
    __delay_cycles(50000); //100ms
    mesafe=sure/56;
    if(mesafe <= 2 || mesafe > 400){
      return FAILED;
    }
    return mesafe;
  
  case SENSOR_2_ID: 
    Sensor2TRIG =1;             //TRIGGER UCUNU BIR YAP 10 MIKROSANIYE BEKLE VE TEKRAR SIFIR YAP
    __delay_cycles(10);
    Sensor2TRIG=0;

    while(!(Sensor2ECHO)){
      Sensor2TRIG = 1;
     __delay_cycles(15);
      Sensor2TRIG = 0;
      __delay_cycles(15);
    }     // ECHO PINI 1 OLANA KADAR BEKLE 
    
    TA0CTL |=MC_2;      // COUNT MODU CONTINIOUS OLARAK AYARLA
    while(Sensor2ECHO);        // ECHO PINI 0 OLANA KADAR BEKLE
    TA0CTL ^=MC_2;      // SAYMA MODUNU DEVRE DISI
    sure=TA0R;          // SURE DEGISKENINE TA0R REGISTERINDEKI DEGER YÜKLENDI.    
    TA0R=0;
    __delay_cycles(50000); //100ms
    mesafe=sure/56;
    if(mesafe <= minOlcumDegeri || mesafe > maxOlcumDegeri){ //Min olcum degerinden kucuk yada max olcum degerinden buyukse hata geri dondur.
      return FAILED;
    }
    return mesafe;
  }
  return 0;
}