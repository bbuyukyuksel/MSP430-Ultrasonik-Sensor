#include "io430.h"
#include <stdlib.h>
#include "LCD.h"

#define TRIG P2OUT_bit.P1    //1.Sensorun Trigger pini P2.1'de cikis olarak ayarlandi    
#define ECHO P2IN_bit.P2	 //1.Sensorun Echo pini P2.2'de giris olarak ayarlandi    

#define TRIG_2 P2OUT_bit.P3	//2.Sensorun Echo pini P2.3'de cikis olarak ayarlandi 
#define ECHO_2 P2IN_bit.P4	//2.Sensorun Echo pini P2.4'de giris olarak ayarlandi 

#define OLCUM_YAPILAMADI -1 //Sensor olcum fonksiyonundan olcum yapilmazsa geri dondurulecek deger

#define CopKutusuBoyutu 65 //cm

int ortalama(int* dizi,int length){   //Dizinin ortalamasini alan fonksiyon
  int sum = 0;
  int i;
  for(i = 0; i< length ; i++){
      sum += dizi[i]; 
  }
  return (sum / length);
}

int OLCUM_YAP(int SENSOR_NO){	//Olcum Yapan fonksiyon
  int sure, uzaklik;
  char buff[8];
  
  if(SENSOR_NO == 1){
    TRIG=1;             //TRIGGER UCUNU BIR YAP 10 MIKROSANIYE BEKLE VE TEKRAR SIFIR YAP
    __delay_cycles(10);
    TRIG=0;
 
    while(!(ECHO)){		//Eger sensorun echo ucu bir sekilde 1 olmazsa sonsuz donguden kurtarmak icin echo 1 olana kadar trigger ucunu 1'den 0'a cekiyoruz. 
      TRIG = 1;
     __delay_cycles(10);
      TRIG = 0;
      __delay_cycles(10);
    }     // ECHO PINI 1 OLANA KADAR BEKLE 
    
    TA0CTL |=MC_2;      // COUNT MODU CONTINIOUS OLARAK AYARLA
    while(ECHO);        // ECHO PINI 0 OLANA KADAR BEKLE
    TA0CTL ^=MC_2;      // SAYMA MODUNU DEVRE DISI
    sure=TA0R;          // SURE DEGISKENINE TA0R REGISTERINDEKI DEGER YÜKLENDI.    
    TA0R=0;
    __delay_cycles(100000); //100ms
    uzaklik=sure/56;		//Uzaklik hesaplama icin denklem kullanildi
    if(uzaklik <= 2 || uzaklik > 450){		//uzaklik degeri 2'den kucuk yada 450'den buyukse fail ver.
      lcd_go(2,15);
      lcd_char('1');
      return OLCUM_YAPILAMADI;
    }
     lcd_go(2,15);
      lcd_char(' ');
    return uzaklik;	//olcum basarili olmussa uzaklik degerini dondur.
  }
  else if(SENSOR_NO == 2){
    TRIG_2 =1;             //TRIGGER UCUNU BIR YAP 10 MIKROSANIYE BEKLE VE TEKRAR SIFIR YAP
    __delay_cycles(10);
    TRIG_2=0;

    while(!(ECHO_2)){
      TRIG_2 = 1;
     __delay_cycles(10);
      TRIG_2 = 0;
      __delay_cycles(10);
    }     // ECHO PINI 1 OLANA KADAR BEKLE 
    
    TA0CTL |=MC_2;      // COUNT MODU CONTINIOUS OLARAK AYARLA
    while(ECHO_2);        // ECHO PINI 0 OLANA KADAR BEKLE
    TA0CTL ^=MC_2;      // SAYMA MODUNU DEVRE DISI
    sure=TA0R;          // SURE DEGISKENINE TA0R REGISTERINDEKI DEGER YÜKLENDI.    
    TA0R=0;
    __delay_cycles(100000); //100ms
    uzaklik=sure/56;
    if(uzaklik <= 2 || uzaklik > 450){
      lcd_go(2,16);
      lcd_char('2');
      return OLCUM_YAPILAMADI;
    }
      lcd_go(2,16);
      lcd_char(' ');
    return uzaklik;
  }
  return -1;
}

int main( void )
{ 
  WDTCTL = WDTPW + WDTHOLD;
  
  BCSCTL1= CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  
  float katSayi = 100.0F / CopKutusuBoyutu;  //Doluluk oranini hesaplamak icin gerekli katsayi.
  
  __delay_cycles(100000); //100ms
  
  P2DIR = BIT1 | BIT3;		//1 ve 3 cikis olarak ayarlandi.
  P2REN = BIT2 | BIT4;		//pull down olarak P2 internal resistorler etkinlestirildi.
  P2OUT = 0x00;				
    
  TA0CTL=0x00;  //TIMERAO KONTROL REG.SIFIRLA
  TA0CTL= TASSEL_2 + MC_0 + ID_0; //TIMERA0 DCO CLOCK , 1E BÖL SEKLINDE AYARLANDI
  
  lcd_start();
  lcd_set(CURSOR_OFF | DISPLAY_ON); //LCD'deki ekran acilarak, cursor kapatildi.
  lcd_clear();
  
  lcd_string("Olcum icin");
  lcd_go(2,1);
  lcd_string("hazirlaniyor..");
  
  while(1){
    int Sensor_1_Uzaklik = -1;
    int Sensor_2_Uzaklik = -1;
    int SensorOlcumleri[6] = {0}; //Her sensor 3'er kez olcum yaptigindan degerleri saklayacagimiz dizi
    int Index = 0;
    int i = 0;
    for(i = 0; i < 3; i++){
      do{
        Sensor_1_Uzaklik = OLCUM_YAP(1);
      }while(Sensor_1_Uzaklik == OLCUM_YAPILAMADI); //Hata mesaji almayana kadar olcum yap
      SensorOlcumleri[Index++] = Sensor_1_Uzaklik; //Diziye at.
      __delay_cycles(100000);
      do{
        Sensor_2_Uzaklik = OLCUM_YAP(2);
      }while(Sensor_2_Uzaklik == OLCUM_YAPILAMADI);
      SensorOlcumleri[Index++] = Sensor_2_Uzaklik; 
    }
    
    int ortalamaDolulukCm = ortalama(SensorOlcumleri,6); //Dizinin ortalamasini al
    
	if(ortalamaDolulukCm > CopKutusuBoyutu)		//Islemler negatif olmasin diye, mesafe bir sekilde cop kutusundan yuksek cikarsa, max uzunluk olan cop kutusu uzunlugunu ver.
		ortalamaDolulukCm = CopKutusuBoyutu;
    
	int DolulukCm = CopKutusuBoyutu - (ortalamaDolulukCm); //Cople olan mesafe'den kutunun boyutu cikartilarak copun doluluk orani hesaplaniyor
    float DolulukYuzdesi_ = DolulukCm * katSayi;		//100'de seklinde oran ifade ediliyor.
    int DolulukYuzdesi = (int)DolulukYuzdesi_;			//Float olan sayi, lcd'de yazdirilabilmek icin int'e cevrildi.
    lcd_clear();
    lcd_string("Doluluk : %");
    __delay_cycles(100);
    char DolulukYuzdesi_str[8];
    itoa(DolulukYuzdesi,DolulukYuzdesi_str);
    lcd_string(DolulukYuzdesi_str);
   
    __delay_cycles(500000); //500ms
    
  }
  return 0;
}