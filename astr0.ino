/*
   ASTROINSEGUITORE versione 0.01
   (c) 2016 Lafayette Software Development for Il Poliedrico
   written by Umberto Genovese

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  //
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  //
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
  //



  Hardware richiesto:
  1 motherboard Arduino Mega 2560 R3 ATmega2560-16AU
  1 EasyDriver v4.4 LDTR-161537 (IC A3967)
  1 Display LCD 2004A (20 colonne X 4 righe)con modulo di comunicazione seriale IIC / I2C

*/

// Librerie esterne
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>

#define DEBUG   // Non commentare per test
// Input Digitali
#define  PIU_PIN  4   // Pulsante PIU'
#define  MENO_PIN  3  // Pulsante MENO
#define  SELECT_PIN 2 // Pulsante SELECT

// Output digitali
#define STEP_PIN 18    // Pin pilota stepper
#define DIR_PIN 19     // Pin direzione stepper
#define FOTO_PIN 13   // Pin pilota scatto remoto temporizzato (Booleano, se alto scatta per il tempo richiesto)
#define BUZZER_PIN 12 // Pin pilota del buzzer
#define MS1_PIN 14    // Divisore per 2 degli step (Booleano, se alto divide)
#define MS2_PIN 15    // Divisore per 4 degli step (Booleano, se alto divide)
#define SLEEP_PIN 16 // Accende l'EasyStepper (Booleano, se alto spenge)
#define RESET_PIN 7  // Reset controller motore
// Menù interattivo
char* MENU_1[4]       =  {"Settaggio", "Controllo", "Reset", "Avvio"};
char* MENU_2[3]       = {"Tempo d'inseguimento", "Tempo di scatto", "Lunghezza focale"};
char* MENU_2_unita[3] = {"minuti", "secondi", "millimetri"};


static const float FILETTI_CM = 10;   // Numero di filetti per cm  nella barra
static const float LATO = 11.5;   // Distanza tra cerniera e centro della barra in cm
static const float pi = 3.1415926;   // Angolo di inizio
static const float GIORNO_SOLARE = 86400; // Giorno solare in secondi
static const float GIORNO_SIDERALE = 86164.0419; // Giorno siderale in secondi e decimali
static const float SEC_SIDERALE = 1 / ( GIORNO_SIDERALE / GIORNO_SOLARE );  // Converte un secondo in tempo siderale

static const float CROP = 1.6; // For Canon AFC
static const float THETA = ( GIORNO_SIDERALE / GIORNO_SOLARE ) * ( pi * 2 ); // Angolo θ in radianti per secondo, che varia nel corso del movimento
unsigned long previousMillis = millis();
boolean ledState = LOW;          // ledState used to set the LED
boolean MS1_Stato = LOW;
boolean MS2_Stato = LOW;
boolean STATO_INESEGUIMENTO = false;
float STEP = 360 / 1.8; // Gradi per step
float uSTEP = 0;
int i = 0;
static const int REGOLA_ESP = 500;
unsigned long _time = millis();
float TEMPO_INSEGUIMENTO = 0;
float sidereal = 0;
float solar_time = 0;
float theta = 0;
float degangle = 0;
int actime = 0;
int sctime = 0;
int lcount = 0;
float tr;

float psi = 0;
float comp_psi = 0;
float correction = 0;
float rc = 0;
float d = 0;
float last_steps = 0;
float steps = 0;
float tstep  = 0;

char _buffer[13];
// Init level
int minuti = 1;
float secondi = 10.01;
int LUNGH_FOCALE = 10;

float UpdateInterval = REGOLA_ESP / LUNGH_FOCALE / CROP;
int pos = 0;
int row = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Inizializza il display
static AccelStepper stepper(AccelStepper::DRIVER,
                            STEP_PIN,
                            DIR_PIN);







void setup() {
#ifdef DEBUG                    // Per Uso  
  Serial.begin(115200);      //    di
#endif                          //   debug

  pinMode(PIU_PIN, INPUT_PULLUP);
  pinMode(MENO_PIN, INPUT_PULLUP);
  pinMode(SELECT_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FOTO_PIN, OUTPUT);
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode (SLEEP_PIN, OUTPUT);
  pinMode (SLEEP_PIN, OUTPUT);
  digitalWrite (SLEEP_PIN, LOW);
  digitalWrite (RESET_PIN, LOW);
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(100000);


  lcd.begin();
  lcd.backlight();
  lcd.setCursor(2, 1);
  lcd.print("ASTROINSEGUITORE");
  lcd.setCursor(0, 2);
  lcd.print("(c) ilpoliedrico.it");

  for (int i = 800; i < 1600; i = i + 200) {
    tone(BUZZER_PIN, i, 200);
    // Serial.println (i);
  }
  delay( 3000 );
  lcd.clear();
  delay( 1000 );
  row = 2;
  print_menu(pos, row);
  digitalWrite( FOTO_PIN, LOW );


}   /****************************************     Fine setup principale     ****************************************/

void loop () {
  _F_MENU_();


}
// Menù principale
void _F_MENU_() {
  lcd.setCursor(0, 1);
  int p_avanti = digitalRead(PIU_PIN);
  int p_indietro = digitalRead(MENO_PIN);
  int p_invio = digitalRead(SELECT_PIN);

  if ( p_avanti == HIGH || p_indietro == HIGH || p_invio == HIGH)
  {
    if ( p_avanti == HIGH ) {
      pos++;

    }
    if ( p_indietro == HIGH ) {
      pos--;

    }

    if ( pos > 3 ) pos = 0;
    if ( pos < 0 )  pos = 3;

    print_menu(pos, row);
    delay(200);

    if ( p_invio == HIGH ) {
      lcd.clear();
      switch ( pos )
      {
        case 0:
          _F_SET_();
          pos = 0;
          row = 2;
          print_menu(pos, row);

          break;

        case 1:
          _F_CONTROLLO_();
          pos = 1;
          row = 2;
          print_menu(pos, row);

          break;
        case 2:
          _F_RESET_();
          pos = 2;
          row = 2;
          print_menu(pos, row);
          Serial.println(pos);
          break;

        case 3:
          _F_AVVIO_();
          pos = 3;
          row = 2;
          print_menu(pos, row);

          break;
      }
    }
  }
}
void print_menu(int pos, int row)
{
  lcd.clear();
  lcd.print("  ASTROINSEGUITORE ");
  lcd.setCursor((20 - strlen( MENU_1[pos])) / 2, row);
  lcd.print( MENU_1[pos] );
}


void _F_SET_()

{
  int i = 0;
  boolean scelta = true;
  char _buffer [10];
  while ( i < 3 )
  {


    int p_avanti = digitalRead(PIU_PIN);
    int p_indietro = digitalRead(MENO_PIN);
    int p_invio = digitalRead(SELECT_PIN);

    if ( scelta )
    {
      lcd.clear();  lcd.setCursor((20 - strlen("SETTAGGIO")) / 2, 0);
      lcd.print("SETTAGGIO");
      lcd.setCursor((20 - strlen(MENU_2[i])) / 2, 1);
      lcd.print( MENU_2[i] );
      scelta = false;
    }

    if ( p_avanti ==  HIGH || p_indietro ==  HIGH || p_invio ==  HIGH)
    {
      if ( p_avanti ==  HIGH )
      {
        if ( i == 0 ) {
          minuti++;
          sprintf (_buffer, "%s:", MENU_2_unita[i]);
          lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
          lcd.print (_buffer);
          row = 3; clear_row(row);
          sprintf(_buffer, "%10d", minuti);
          lcd.print( _buffer );
          char _buffer[10] = "";
        }
        if ( i == 1 ) {
          secondi += 0.01;
          if (secondi > 1) {
            secondi += 1.00;
            secondi = int(secondi);
          }
          sprintf (_buffer, "%s:", MENU_2_unita[i]);
          lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
          lcd.print (_buffer);
          row = 3; clear_row(row);
          dtostrf(secondi, 13, 3, _buffer);
          lcd.print( _buffer );
          char _buffer[10] = "";
        }
        if ( i == 2 ) {
          LUNGH_FOCALE++;
          sprintf (_buffer, "%s:", MENU_2_unita[i]);
          lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
          lcd.print (_buffer);
          row = 3; clear_row(row);
          sprintf(_buffer, "%10d", LUNGH_FOCALE);
          lcd.print( _buffer );
          char _buffer[10] = "";
        }
      }
      if ( p_indietro ==  HIGH )
      {
        if ( i == 0 ) {
          minuti--;
          if (minuti <= 0) {
            minuti = 0;
          }
          sprintf (_buffer, "%s:", MENU_2_unita[i]);
          lcd.setCursor((20 - strlen(_buffer )) / 2, 2);
          lcd.print (_buffer);
          row = 3; clear_row(row);
          sprintf(_buffer, "%10d", minuti);
          lcd.print( _buffer );

        }
        if ( i == 1 ) {
          if (secondi > 1) {
            secondi -= 1.00;
            secondi = int(secondi);
          }            else {
            secondi -= 0.01;
          }

          if (secondi <= 0) {
            secondi = 0.01;
          }
          sprintf (_buffer, "%s:", MENU_2_unita[i]);
          lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
          lcd.print (_buffer);
          row = 3; clear_row(row);
          dtostrf(secondi, 13, 3, _buffer);
          lcd.print( _buffer );

        }
        if ( i == 2 ) {
          LUNGH_FOCALE--;
          if (LUNGH_FOCALE <= 0) {
            LUNGH_FOCALE = 0;
          }
          sprintf (_buffer, "%s:", MENU_2_unita[i]);
          lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
          lcd.print (_buffer);
          row = 3; clear_row(row);
          sprintf(_buffer, "%10d", LUNGH_FOCALE);
          lcd.print( _buffer );

        }
      }
      if ( p_invio ==  HIGH )
      {
        lcd.clear();
        i++;
        scelta = true;
      }
    }
    delay( 200 );
  }
}


void clear_row(int row) {
  lcd.setCursor(0, row);
  lcd.print("                   ");
  lcd.setCursor(0, row);
}
void _F_CONTROLLO_()
{

  for (int i = 0; i < 3; i++)
  {
    lcd.clear();  lcd.setCursor((20 - strlen("CONTROLLO")) / 2, 0);
    lcd.print("CONTROLLO");
    lcd.setCursor((20 - strlen(MENU_2[i])) / 2, 1);
    lcd.print( MENU_2[i] );


    if ( i == 0 ) {

      sprintf (_buffer, "%s: %s", MENU_2_unita[i], String(minuti).c_str());
      lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
      lcd.print (_buffer);

    }
    if ( i == 1 ) {
      sprintf (_buffer, "%s: %s", MENU_2_unita[i], String(secondi).c_str());
      lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
      lcd.print (_buffer);

    }
    if ( i == 2 ) {
      sprintf (_buffer, "%s: %s", MENU_2_unita[i], String(LUNGH_FOCALE).c_str());
      lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
      lcd.print (_buffer);

    }

    delay( 3000 );
  }
}

void _F_RESET_()
{
  lcd.clear();
  lcd.print( "Riporto la tavoletta chiusa" );
  STATO_INESEGUIMENTO = false;
  stepper.runToNewPosition (0);

}





void _F_AVVIO_()
{
  digitalWrite(MS1_PIN, HIGH );
  MS1_Stato = HIGH;
  digitalWrite(MS2_PIN, HIGH );
  MS2_Stato = HIGH;
  digitalWrite (SLEEP_PIN, HIGH);
  STATO_INESEGUIMENTO = true;

  TEMPO_INSEGUIMENTO = minuti * 60;

  if (MS1_Stato)
  {
    STEP = STEP * 2;
  }
  if (MS2_Stato)
  {
    STEP = STEP * 4;
  }

  start_tracking();
}


void start_tracking () {

  unsigned long solar_cache = millis();
  float ASTA = (LATO * 2) * sin((pi * TEMPO_INSEGUIMENTO ) / GIORNO_SIDERALE);
  float GIRI = ASTA * FILETTI_CM;
  float uSTEP = GIRI * STEP;
  float _speed = uSTEP / TEMPO_INSEGUIMENTO;
  Serial.println(uSTEP);
  Serial.println(_speed);


  

tone(BUZZER_PIN, 1440, 2000);
delay (6000);
F_END_TRACK();
}

void statusprint () {

}


void scatto() {

}


void F_END_TRACK() {
  STATO_INESEGUIMENTO = false;
  digitalWrite (SLEEP_PIN, LOW);
  digitalWrite (RESET_PIN, HIGH);
  delay(1000);
  digitalWrite (RESET_PIN, LOW);
  return;
}
