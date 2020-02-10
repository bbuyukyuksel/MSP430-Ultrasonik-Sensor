#include "io430.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RS		P1OUT_bit.P4	//0 ise komut, 1 ise text.
#define E		P1OUT_bit.P5	//Clock sinyali, dusen kenarda islem yapilir.
#define HOME	0x02
#define CLEAR	0x01
#define CURSOR_BLINK    BIT0
#define CURSOR_NOBLINK  0
#define CURSOR_ON       BIT1
#define CURSOR_OFF      0
#define DISPLAY_ON      BIT2
#define DISPLAY_OFF     0
#define DYNAMIC_DISP    BIT0
#define SYTATIC_DISP    0
unsigned int control = 0;
void lcd_delay(int ms);
void lcdEDegistir(void);
void lcdNeYazilacak(char ne);
void lcd_send(char ne, int veri);
void lcd_start(void);
void lcd_go(unsigned int satir, unsigned int sutun);
void lcd_string (char* yazi);
void lcd_char (char harf);
void lcd_clear(void);
void lcd_shift(char x);
void cursor_shift(char x);
void lcd_set(unsigned int x);
void entry_set(char ch);
char inttoch(unsigned int num);
unsigned int len_int(unsigned int num);
char* itoa_ (unsigned int num);
char* itoa(int i, char b[]);
void lcd_int(unsigned int num);
void __delay_mil(unsigned int millis);
void __delay_mic(unsigned int micros);
void lcd_blink(unsigned int delayMS);

/*****************************************************************************
P1.0 - D4 Pin11
P1.1 - D5 Pin12
P1.2 - D6 Pin13
P1.3 - D7 Pin14
P1.4 - RS Pin4
//P1.5 - R/W Pin5
P1.5 - E Pin6
Anot - VCC
Katot- VSS
*******************************************************************************/

void lcd_delay(int ms){			 // Bekleme fonksiyonu
  unsigned int i, j;
  i = ms;
  for(j = 0 ; j < 250 ; j++)
  {
     while(i--);
     i = ms;
  }
}

void lcdEDegistir(void)           // E pini düþen kenar tetiklemeli.
{                                 // Bu yüzden gönderdiðimiz bilginin LCD üzerinde
  E = 1;                          // iþlem görmesi için E ucuna düþen kenar uyguluyoruz.
  E = 0;                          // Bunu da E'yi önce "1", sonra "0" yaparak hallediyoruz.
}
void lcdNeYazilacak(char ne)      // LCD'ye giden bilgimiz veri ya da komut olabilir.
{
  if(ne == 'k')                   // Komut ise;
    RS = 0;                       // RS pini "0" yapýlýr.
  else if(ne == 'v')              // Veri ise;
    RS = 1;                       // RS pini "1" yapýlýr.
}

void lcd_send(char ne, int veri) // Gidecek bilginin türünü ve bilgiyi aldýk.
{
	P1OUT = (P1OUT & 0xF0)|((veri>>4) & 0x0F);	// Bilgiyi P1OUT'a bastýk. Böylece LCD ile baðlantýlý kýsým en deðerlikli dört bit oluyor.
	lcdNeYazilacak(ne);             			// Fonksiyona gelen bilgi türünü diðer fonksiyona aktarýp RS pinini ayarladýk.
	lcdEDegistir();                 			// Bilginin aktif olmasý için E pinine düþen kenar uyguladýk.
 
	P1OUT = (P1OUT & 0xF0)|(veri & 0x0F); // send lower nibble
	lcdNeYazilacak(ne);             // RS pinini ayarladýk.
	lcdEDegistir();                 // E pinine düþen kenar uyguladýk.
}
void lcd_start(void)
{
  P1DIR |= 0xFF;
  P1OUT &= 0x00;
  lcd_delay(15);                 // Bir süre beklendi.
  lcd_send('k', 0x33);           // "00000010" yani imleç en baþa gidecek.
  lcd_send('k', 0x32);           // "00101000" yani 4 pin, 2 satýr, 5x7 karakter kullanýlacak.
  lcd_send('k', 0x28);           // "00001000" yani imleç gizlenecek.
  lcd_send('k', 0x0E);           // "00000110" yani imleç yazýlan her karakterden sonra saða doðru kayacak.
  lcd_send('k', 0x01);           // "00000001" yani LCD temizlenecek.
  lcd_send('k', 0x06);
  lcd_send('k', 0x80);
  lcd_delay(10);                   // Bir süre beklendi.
  
}

void lcd_go(unsigned int satir, unsigned int sutun)
{
  if(satir == 1)
  {
    sutun--;                      // 1-16 arasýnda giriþ için 0-15 arasý deðer gelecek.
    lcd_send('k', 0x80 + sutun); // Komut tablosuna göre 80-8F arasýnda deðer gönderiliyor.
  }
 
  else if(satir == 2)
  {
    sutun--;                      // 0-15 arasýnda bir deðer elde etmek için.
    lcd_send('k', 0xC0 + sutun); // C0 - CF arasý deðer gönderiliyor.
  }
}
void lcd_string (char* yazi)      // Bir string bastýrmak için fonksiyon.
{
  lcd_delay(1);                    // Bir süre beklendi.
  int i = 0;
  while(yazi[i])                  // Yazýnýn sonuna gelene kadar,
  lcd_send('v', yazi[i++]);      // Veriyi gönder ve bir sonraki veriye geç.
}
void lcd_char (char harf)       // Tek harf bastýrmak için fonksiyon.
{
  lcd_delay(1);                    // Bir süre beklendi.
  lcd_send('v', harf);           // Veriyi gönder.
}
void lcd_clear(void)
{
  lcd_send('k', 0x01);           // LCD temizlendi.
  lcd_go(1, 1);            // 1. satýr, 1. sütuna gidildi.
  lcd_delay(10);                   // Bir süre beklendi.
}
void lcd_shift(char x){
   if(x == 'r')
     lcd_send('k',0x10 | BIT3 | BIT2);
  else if(x == 'l')
    lcd_send('k',0x10 | BIT3);
}
void cursor_shift(char x){
  if(x == 'r')
     lcd_send('k',0x10 | BIT2);
  else if(x == 'l')
    lcd_send('k',0x10);
}
void lcd_set(unsigned int x){
    lcd_send('k',x | BIT3);
}
void entry_set(char ch){
  unsigned int x = BIT2;
  if(ch == 'r')
    x |= BIT1;
  else if(ch == 'l')
    x &= ~BIT1;
  
  lcd_send('k',x);
}
char inttoch(unsigned int num){
    switch(num){
    case 0: return '0'; break;
    case 1: return '1'; break;
    case 2: return '2'; break;
    case 3: return '3'; break;
    case 4: return '4'; break;
    case 5: return '5'; break;
    case 6: return '6'; break;
    case 7: return '7'; break;
    case 8: return '8'; break;
    case 9: return '9'; break;
    default : return NULL;
    }
}
unsigned int len_int(unsigned int num){
    unsigned short i = 0;
    if(num == 0)
      return 1;
    while(num > 0){
        num /= 10;
        i++;
    }
    return i;
}
char* itoa_ (unsigned int num){
    int i;
    unsigned int size = len_int(num);
    char* buffer = (char*) malloc(sizeof(char) * (size+2));
    for(i = size -1;i >= 0; i--){
        buffer[i] = inttoch(num % 10);
        num = num / 10;
    }
	buffer[size] = '\0';
    return buffer;
}
void lcd_int(unsigned int num){
    lcd_string(itoa_(num));
}

void __delay_mil(unsigned int millis){
  __enable_interrupt();
  control = 0;
  TACCR0 = 125 * millis; //1ms'de bir kesme üretilir.//maks. 500 milisaniye.
  TACCTL0 = CCIE;
  TACTL = MC_1 | ID_3 | TASSEL_2 | TACLR;
  while(!control);
  TACCTL0 &= ~CCIE;
}
void __delay_mic(unsigned int micros){
  __enable_interrupt();
  control = 0;
  TACCR0 = micros; //1ms'de bir kesme üretilir.//maks. 500 milisaniye.
  TACCTL0 = CCIE;
  TACTL = MC_1 | ID_0 | TASSEL_2 | TACLR;
  while(!control);
  TACCTL0 &= ~CCIE;
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void){
  control = 1;
}
void lcd_blink(unsigned int delayMS){
      lcd_set(DISPLAY_ON);
      __delay_mil(delayMS/2);
      __delay_mil(delayMS/2);
      lcd_set(DISPLAY_OFF);
      __delay_mil(delayMS/2);
      __delay_mil(delayMS/2);
}
char* itoa(int i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}











	
