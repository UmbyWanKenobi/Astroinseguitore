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
  1 Motherboard Arduino Mega 2560 R3 ATmega2560-16AU
  1 Pololu DRV-8825 (TI-8825)
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
#define SLEEP_PIN 16 // Accende il Pololu (Booleano, se alto spenge)
#define RESET_PIN 7  // Reset controller motore
// Menù interattivo
char* MENU[3][4] = {
  {"Settaggio", "Controllo", "Reset", "Avvio"},
  {"Tempo d'inseguimento", "Tempo di scatto", "Lunghezza focale"},
  {"minuti", "secondi", "millimetri"}
};
static const float FILETTI_CM = 10;   // Numero di filetti per cm  nella barra
static const float LATO = 11.5;   // Distanza tra cerniera e centro della barra in cm
static const float pi = 3.1415926;   // Angolo di inizio
static const float GIORNO_SOLARE = 86400; // Giorno solare in secondi
static const float GIORNO_SIDERALE = 86164.0419; // Giorno siderale in secondi e decimali
static const float SEC_SIDERALE = 1 / ( GIORNO_SIDERALE / GIORNO_SOLARE );  // Converte un secondo in tempo siderale

static const float CROP = 1.6; // For Canon AFC
unsigned long previousMillis = millis();


boolean ledState = LOW; // Usato per il fototriac  necessario allo scatto remoto

//                                                                                                                                ---------------------------------------------------
//         DATI DEL CHIP TEXAS INSTRUMENTS TI-8825 PER IL POLOLU DRV8825                                                          | MS0  | MS1  | MS2  |        DIVISORE            |
//                                                                                                                                |------|------|------|----------------------------|
#define MS0_PIN 50  //                                                                                                            | LOW  | LOW  | LOW  | passo intero               |
boolean MS0_Stato = LOW; // Usato per definire la moltiplicazione degli step del motore ad opera della scheda pilota              | HIGH | LOW  | LOW  | mezzo passo                |
#define MS1_PIN 14  //                                                                                                            | LOW  | HIGH | LOW  | un quarto di passo         |
boolean MS1_Stato = LOW; // Usato per definire la moltiplicazione degli step del motore ad opera della scheda pilota              | HIGH | HIGH | LOW  | un ottavo di passo         |
#define MS2_PIN 15  //                                                                                                            | LOW  | LOW  | HIGH | un sedicesimo di passo     |
boolean MS2_Stato = LOW; // Usato per definire la moltiplicazione degli step del motore ad opera della scheda pilota              | HIGH | LOW  | HIGH | un trentaduesimo di passo  |
//                                                                                                                                | LOW  | HIGH | HIGH | un trentaduesimo di passo  |
//                                                                                                                                | HIGH | HIGH | HIGH | un trentaduesimo di passo  |
//                                                                                                                                ---------------------------------------------------
boolean STATO_INSEGUIMENTO = false; // Imostato a TRUE se l'iseguimento è siderale
float STEP = 360 / 1.8; // Gradi per step del motore
float uSTEP = 0;  // Micropassi legati alla scheda pilota  (calcolato dinamicamente)
int i = 0; // Variabile Globale
static const int REGOLA_ESP = 500;
unsigned long _time = millis(); // Legato all'orologio interno di Arduino (millisecondi dall'accensione della scheda)
float TEMPO_INSEGUIMENTO = 0;  // Tempo di insegujimento siderale
float TEMPO_SIDERALE = 0; // Tempo siderale (visualizzato durante il periodo di inseguimento)
float TEMPO_SOLARE = 0; //Tempo solare necessario nell'equazioni di calcolo
float THETA = 0; // Angolo  θ in radianti per secondo, che varia nel corso del movimento
float THETA_GRADI = 0; // Angolo θ espresso in gradi
int actime = 0;
int sctime = 0;
int lcount = 0;
float tr;

float PSI = 0;
float COMPLEMENTO_PSI = 0;
float INDICE_DI_CORREZIONE = 0;
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
//      INIZIALIZZAZIONE DELLE PERIFERICHE
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Inizializza il display
static AccelStepper stepper(AccelStepper::DRIVER,
                            STEP_PIN,
                            DIR_PIN);






//-------------------------------------(     SETUP PRINCIPALE     )----------------------------//
void setup() {
#ifdef DEBUG                    // Per Uso  
  Serial.begin(115200);      //    di
#endif                          //   debug

  pinMode(PIU_PIN, INPUT_PULLUP);
  pinMode(MENO_PIN, INPUT_PULLUP);
  pinMode(SELECT_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FOTO_PIN, OUTPUT);
  pinMode(MS0_PIN, OUTPUT);
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode (SLEEP_PIN, OUTPUT);
  pinMode (SLEEP_PIN, OUTPUT);
  digitalWrite (SLEEP_PIN, LOW);
  digitalWrite (RESET_PIN, LOW);
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(100000);

  // SCHERMATA DI AVVIO
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(2, 1);
  lcd.print("ASTROINSEGUITORE");
  lcd.setCursor(0, 2);
  lcd.print("(c) ilpoliedrico.it");

  tone(BUZZER_PIN, i, 200);

  delay( 3000 );
  lcd.clear();
  delay( 1000 );
  row = 2;
  print_menu(pos, row);
  digitalWrite( FOTO_PIN, LOW );


}  //--------------------------------------------------------------------------------------------//



//--------------------------------------(           CICLO DI LOOP       )-------------------------------//
void loop () {
  _F_MENU_();


}//-----------------------------------------------------------------------------------------------------------//

//--------------------------------------(        MENU' PRINCIPALE      )---------------------------------//
void _F_MENU_()
{
  lcd.setCursor(0, 1);
  int p_avanti = digitalRead(PIU_PIN);
  int p_indietro = digitalRead(MENO_PIN);
  int p_invio = digitalRead(SELECT_PIN);
  // LOOP PRIMO MENU'
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
        //  SETTAGGIO
        case 0:
          _F_SET_();
          pos = 0;
          row = 2;
          print_menu(pos, row);
          break;
        //  CONTROLLO
        case 1:
          _F_CONTROLLO_();
          pos = 1;
          row = 2;
          print_menu(pos, row);
          break;
        //  RESET
        case 2:
          _F_RESET_();
          pos = 2;
          row = 2;
          print_menu(pos, row);
          Serial.println(pos);
          break;
        //  AVVIO SEQUENZA PRINCIPALE
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
//------------------(    CICLO MINORE PER CENTRARE IL CURSORE   )------------------//
void print_menu(int pos, int row)
{
  lcd.clear();
  lcd.print("  ASTROINSEGUITORE ");
  lcd.setCursor((20 - strlen( MENU[0][pos])) / 2, row);
  lcd.print( MENU[0][pos] );
}

//-----------------------(   MENU' SETTAGGIO DELLE FUNZIONI PRINCIPALI   )-------------------//
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
      lcd.setCursor((20 - strlen(MENU[1][i])) / 2, 1);
      lcd.print( MENU[1][i] );
      scelta = false;
    }
    //  CICLO DI IMMISSIONE DEI PARAMETRI
    if ( p_avanti ==  HIGH || p_indietro ==  HIGH || p_invio ==  HIGH)
    {
      if ( p_avanti ==  HIGH )
      {
        if ( i == 0 ) {
          minuti++;
          sprintf (_buffer, "%s:", MENU[2][i]);
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
          sprintf (_buffer, "%s:", MENU[2][i]);
          lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
          lcd.print (_buffer);
          row = 3; clear_row(row);
          dtostrf(secondi, 13, 3, _buffer);
          lcd.print( _buffer );
          char _buffer[10] = "";
        }
        if ( i == 2 ) {
          LUNGH_FOCALE++;
          sprintf (_buffer, "%s:", MENU[2][i]);
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
          sprintf (_buffer, "%s:", MENU[2][i]);
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
          sprintf (_buffer, "%s:", MENU[2][i]);
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
          sprintf (_buffer, "%s:", MENU[2][i]);
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
//------------------------------------------------------------------------------------------------------------------//


//----------------------------(CICLO MINORE DI PULIZIA DELLA RIGA DI STAMPA   )-------------------------------------//
void clear_row(int row) {
  lcd.setCursor(0, row);
  lcd.print("                   ");
  lcd.setCursor(0, row);
}


//---------------------------------(   VISUALIZZA I PARAMETRI IN MEMORIA   )-----------------------------//
void _F_CONTROLLO_()
{
  for (int i = 0; i < 3; i++)
  {
    lcd.clear();  lcd.setCursor((20 - strlen("CONTROLLO")) / 2, 0);
    lcd.print("CONTROLLO");
    lcd.setCursor((20 - strlen(MENU[1][i])) / 2, 1);
    lcd.print( MENU[1][i] );


    if ( i == 0 ) {

      sprintf (_buffer, "%s: %s", MENU[2][i], String(minuti).c_str());
      lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
      lcd.print (_buffer);

    }
    if ( i == 1 ) {
      sprintf (_buffer, "%s: %s", MENU[2][i], String(secondi).c_str());
      lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
      lcd.print (_buffer);

    }
    if ( i == 2 ) {
      sprintf (_buffer, "%s: %s", MENU[2][i], String(LUNGH_FOCALE).c_str());
      lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
      lcd.print (_buffer);

    }

    delay( 3000 );
  }
}
//-----------------------------------------------------------------------------------------///

//---------------------------( RESET COMPLETO    )--------------------------//
void _F_RESET_()
{
  lcd.clear();
  lcd.print( "Riporto la tavoletta chiusa" );
  STATO_INSEGUIMENTO = false;
  stepper.runToNewPosition (0);

}//--------------------------------------------------------------------------//





void _F_AVVIO_()
{
  digitalWrite(MS0_PIN, HIGH );
  MS0_Stato = HIGH;
  digitalWrite(MS1_PIN, HIGH );
  MS1_Stato = HIGH;
  digitalWrite(MS2_PIN, HIGH );
  MS2_Stato = HIGH;
  digitalWrite (SLEEP_PIN, HIGH);
  STATO_INSEGUIMENTO = true;

  TEMPO_INSEGUIMENTO = minuti * 60;

  if (MS0_Stato && !MS1_Stato && !MS2_Stato)
  {
    STEP = STEP * 2;
  }
  if (!MS0_Stato && MS1_Stato && !MS2_Stato)
  {
    STEP = STEP * 4;
  }
  if (MS0_Stato && MS1_Stato && !MS2_Stato)
  {
    STEP = STEP * 8;
  } if (!MS0_Stato && !MS1_Stato && MS2_Stato)
  {
    STEP = STEP * 16;
  }
  if (MS0_Stato && MS1_Stato && MS2_Stato)
  {
    STEP = STEP * 32;
  }
  _F_SIDERALE();
}


void _F_SIDERALE () {

  unsigned long solar_cache = millis();
  float ASTA = (LATO * 2) * sin((pi * TEMPO_INSEGUIMENTO ) / GIORNO_SIDERALE);
  float GIRI = ASTA * FILETTI_CM;
  float uSTEP = GIRI * STEP;
  float _speed = uSTEP / TEMPO_INSEGUIMENTO;
  Serial.println(uSTEP);
  Serial.println(_speed);


  while (( STATO_INSEGUIMENTO ) && ( TEMPO_SOLARE < TEMPO_INSEGUIMENTO )) {
    int p_avanti = digitalRead(PIU_PIN);
    long _time = millis();
    // Recupera il tempo solare in secondi
    TEMPO_SOLARE = float(millis() - solar_cache ) / 1000.0;
    // Converte il tempo solare in tempo siderale
    TEMPO_SIDERALE = TEMPO_SOLARE * SEC_SIDERALE;
    tr = TEMPO_INSEGUIMENTO - TEMPO_SOLARE;



    if ( p_avanti ==  HIGH ) {
      F_END_TRACK();
    }


    if (_time > lcount + 4000) {
      statusprint();
      lcount = millis ();
    }

    // Calcolo l'angolo θ iniziale
    THETA = (TEMPO_SIDERALE / GIORNO_SOLARE) * (pi * 2);

    // Converte θ in gradi per la lettura
    THETA_GRADI = THETA * 180.0 / pi;

    // calcolo il complemento ddell'angolo ψ opposto a θ
    COMPLEMENTO_PSI = THETA /2;
    // Calculate the INDICE_DI_CORREZIONE distance
    INDICE_DI_CORREZIONE = 1.25 * tan(COMPLEMENTO_PSI);
    //applico la correzione
    rc = LATO - INDICE_DI_CORREZIONE;
    
    d = (rc * sin(THETA)) / sin(PSI);
   
    long steps = abs(d * FILETTI_CM * STEP);

    while (last_steps < steps)
    {
      
      stepper.runToNewPosition (steps);
     
      last_steps = last_steps + 1;
    

    tstep = millis();

    }
  }


  tone(BUZZER_PIN, 1440, 2000);
  delay (6000);
  F_END_TRACK();
}

void statusprint () {
  
  lcd.setCursor(0, 0);


  
  lcd.print("T. siderale = "); lcd.print(TEMPO_SIDERALE);
  lcd.setCursor(0, 1);
  lcd.print("Time remain= "); lcd.print(tr);
  lcd.setCursor(0, 2);
  
  lcd.print("Steps= "); lcd.print(stepper.currentPosition());
  lcd.print(" - ");
  
  lcd.setCursor(0, 3);
 
  lcd.print("THETA Angle = "); lcd.print(THETA_GRADI);

}


void scatto() {
  
  unsigned long currentMillis = millis();

  if ((ledState == HIGH) && (currentMillis - previousMillis >= (secondi * 1000)))
  {
    ledState = LOW;  
    previousMillis = currentMillis;
    digitalWrite(FOTO_PIN, ledState);
  }
  else if ((ledState == LOW) && (currentMillis - previousMillis >= 3000))
  {
    ledState = HIGH; 
    previousMillis = currentMillis;
    digitalWrite(FOTO_PIN, ledState);   
  }

}


void F_END_TRACK() {
  STATO_INSEGUIMENTO = false;
  digitalWrite (SLEEP_PIN, LOW);
  digitalWrite (RESET_PIN, HIGH);
  delay(1000);
  digitalWrite (RESET_PIN, LOW);
  return;
}
