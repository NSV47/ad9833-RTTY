#include <Arduino.h>
#include <AD9833.h>     // Include the library
/*RTTY transimitter for AD9833
 Sergey Novikov
 Distribuited under GNU GPL
 Based on:
 Baudot code by Tim Zaman http://www.timzaman.nl/?p=138
 */

#include <stdint.h>

#define FNC_PIN 4       // Can be any digital IO pin

/*----------------------------------------------------------------------*/
/*Transmission parameters*/

//TX frequency (MHZ - e6)
// #define FREQ  14.082e6
#define RF_FREQ 10099

// offset RTTY
#define OFFSET 170
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*AD9850 - Arduino wiring*/
//CLK - D11, FQ - D10, DATA - D9, RST - D8
// #define CLK  8
// #define FQ   9
// #define DATA 10
// #define RST  11
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*Baudot code definitions*/
#define ARRAY_LEN 32
#define LETTERS_SHIFT 31
#define FIGURES_SHIFT 27
#define LINEFEED 2
#define CARRRTN  8

#define is_lowercase(ch)    ((ch) >= 'a' && (ch) <= 'z')
#define is_uppercase(ch)    ((ch) >= 'A' && (ch) <= 'Z')

unsigned long time;

char letters_arr[33] = "\000E\nA SIU\rDRJNFCKTZLWHYPQOBG\000MXV\000";
char figures_arr[33] = "\0003\n- \a87\r$4',!:(5\")2#6019?&\000./;\000";

enum baudot_mode {
  NONE,
  LETTERS,
  FIGURES
};
/*----------------------------------------------------------------------*/

//AD9850 object
// EF_AD9850 AD9850(CLK, FQ, DATA, RST);
AD9833 gen(FNC_PIN);       // Defaults to 25MHz internal reference frequency

void rtty_txbit (int bit);
void rtty_txstring(char *str);
void rtty_txstring(String str);
void rtty_txbyte(uint8_t b);
uint8_t char_to_baudot(char c, char *array);

void setup()
{
  //AD9850 initialize and reset
  // AD9850.init();
  // AD9850.reset();
  pinMode(4,OUTPUT);
  // pinMode(2,OUTPUT);
  // pinMode(6,OUTPUT);
  Serial.begin(9600);
//digitalWrite(2,HIGH);
Serial.println(RF_FREQ);
  gen.Begin();
//  i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  gen.ApplySignal(SQUARE_WAVE,REG0,RF_FREQ*1000ul); // SINE_WAVE // SQUARE_WAVE // HALF_SQUARE_WAVE
  // gen.EnableOutput(true);   // Turn ON the output - it defaults to OFF
}


void loop()
{

      rtty_txstring("RYRYRYRY THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG"); 
      // AD9850.reset();
      gen.EnableOutput(false);   // Turn ON the output - it defaults to OFF
      delay(1000);
}




//RTTY functions
/*----------------------------------------------------------------------*/
uint8_t char_to_baudot(char c, char *array)
{
  int i;
  for (i = 0; i < ARRAY_LEN; i++)
  {
    if (array[i] == c)
      return i;
  }

  return 0;
}

void rtty_txbyte(uint8_t b)
{
  int8_t i;

  rtty_txbit(0);

  /* TODO: I don't know if baudot is MSB first or LSB first */
  /* for (i = 4; i >= 0; i--) */
  for (i = 0; i < 5; i++)
  {
    if (b & (1 << i))
      rtty_txbit(1);
    else
      rtty_txbit(0);
  }

  rtty_txbit(1);
}

void rtty_txstring(String str)
{
  int len = str.length();
  char buf[len];
  str.toCharArray(buf, len);

  rtty_txstring(buf);
  
}

void rtty_txstring(char *str)
{
  enum baudot_mode current_mode = NONE;
  char c;
  uint8_t b;

  while (*str != '\0')
  {
    c = *str;
    /* some characters are available in both sets */
    if (c == '\n')
    {
      rtty_txbyte(LINEFEED);
    }
    else if (c == '\r')
    {
      rtty_txbyte(CARRRTN);
    }
    else if (is_lowercase(*str) || is_uppercase(*str))
    {
      if (is_lowercase(*str))
      {
        c -= 32;
      }

      if (current_mode != LETTERS)
      {
        rtty_txbyte(LETTERS_SHIFT);
        current_mode = LETTERS;
      }

      rtty_txbyte(char_to_baudot(c, letters_arr));
    }
    else
    {
      b = char_to_baudot(c, figures_arr);

      if (b != 0 && current_mode != FIGURES)
      {
        rtty_txbyte(FIGURES_SHIFT);
        current_mode = FIGURES;
      }

      rtty_txbyte(b);
    }

    str++;
  }
}



// Transmit a bit as a mark or space
void rtty_txbit (int bit) {
  if (bit) {
    // High - mark
    //digitalWrite(2, HIGH);
    //digitalWrite(3, LOW);

    // AD9850.wr_serial(0x00, FREQ+OFFSET);
    gen.ApplySignal(SQUARE_WAVE,REG0,(RF_FREQ*1000ul)+OFFSET); // SINE_WAVE // SQUARE_WAVE // HALF_SQUARE_WAVE 
    gen.EnableOutput(true);   // Turn ON the output - it defaults to OFF
  } 
  else {
    // Low - space
    //digitalWrite(3, HIGH);
    //digitalWrite(2, LOW);

    // AD9850.wr_serial(0x00, FREQ);
    gen.ApplySignal(SQUARE_WAVE,REG0,(RF_FREQ*1000ul)); // SINE_WAVE // SQUARE_WAVE // HALF_SQUARE_WAVE 
    gen.EnableOutput(true);   // Turn ON the output - it defaults to OFF
  }

  // Delay appropriately - tuned to 45.45 baud.

  delay(22); //sets the baud rate
  //delayMicroseconds(250);
}

