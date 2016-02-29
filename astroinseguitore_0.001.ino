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
#include <Button.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <Adafruit_HMC5883_U.h>

#define DEBUG   // Non commentare per test
// Input Digitali

#define m_pin 47
// Output digitali
#define STEP_PIN 18    // Pin pilota stepper
#define DIR_PIN 19     // Pin direzione stepper
#define FOTO_PIN 13   // Pin pilota scatto remoto temporizzato (Booleano, se alto scatta per il tempo richiesto)
#define BUZZER_PIN 12 // Pin pilota del buzzer
//#define SLEEP_PIN 16 // Accende il Pololu (Booleano, se alto spenge)
//#define RESET_PIN 7  // Reset controller motore
// Menù interattivo
char* MENU[3][5] = {
  {"Impostazioni", "Controllo", "Reset", "Avvio","Calibrazione"},
  {"Tempo d'inseguimento", "Tempo di scatto", "Decl. magnetica"},
  {"minuti", "secondi", "gradi e primi"}
};
static const float FILETTI_CM = 10;   // Numero di filetti per cm  nella barra
static const float LATO = 15.5;   // Distanza tra cerniera e centro della barra in cm
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
#define MS0_PIN 16  //                                                                                                            | LOW  | LOW  | LOW  | passo intero               |
boolean MS0_Stato = LOW; // Usato per definire la moltiplicazione degli step del motore ad opera della scheda pilota              | HIGH | LOW  | LOW  | mezzo passo                |
#define MS1_PIN 15  //                                                                                                            | LOW  | HIGH | LOW  | un quarto di passo         |
boolean MS1_Stato = LOW; // Usato per definire la moltiplicazione degli step del motore ad opera della scheda pilota              | HIGH | HIGH | LOW  | un ottavo di passo         |
#define MS2_PIN 14  //                                                                                                            | LOW  | LOW  | HIGH | un sedicesimo di passo     |
boolean MS2_Stato = LOW; // Usato per definire la moltiplicazione degli step del motore ad opera della scheda pilota              | HIGH | LOW  | HIGH | un trentaduesimo di passo  |
//                                                                                                                                | LOW  | HIGH | HIGH | un trentaduesimo di passo  |
//                                                                                                                                | HIGH | HIGH | HIGH | un trentaduesimo di passo  |
//                                                                                                                                ---------------------------------------------------
boolean STATO_INSEGUIMENTO = false; // Imostato a TRUE se l'iseguimento è siderale
float STEP = 360 /0.9;
// Gradi per step del motore
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
unsigned long lcount = 0;
unsigned long scount = 0;
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
int minuti = 40;
float secondi = 10.0;
float RAD_DECL_MAGN = 0.048; // Declinazione magnetica per l'Italia Centrale
  float DECL_MAGN = 0;
float UpdateInterval = REGOLA_ESP / 10 / CROP;
int pos = 0;
int row = 0;
//      INIZIALIZZAZIONE DELLE PERIFERICHE
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Inizializza il display
static AccelStepper stepper(AccelStepper::DRIVER,
                            STEP_PIN,
                            DIR_PIN);
//Init HMC5883L sensor
Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345);


Button PIU = Button(4, PULLUP);   // Pulsante PIU'
Button MENO = Button(3, PULLUP);  // Pulsante MENO
Button SELECT = Button(2, PULLUP); // Pulsante SELECT



//-------------------------------------(     SETUP PRINCIPALE     )----------------------------//
void setup() {
#ifdef DEBUG                    // Per Uso  
  Serial.begin(115200);      //    di
#endif                          //   debug

 /* pinMode(PIU_PIN, INPUT_PULLUP);
  pinMode(MENO_PIN, INPUT_PULLUP);
  pinMode(SELECT_PIN, INPUT_PULLUP);
*/
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FOTO_PIN, OUTPUT);
  pinMode(MS0_PIN, OUTPUT);
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
//  pinMode (SLEEP_PIN, OUTPUT);
  
  //digitalWrite (SLEEP_PIN, LOW);
  //digitalWrite (RESET_PIN, LOW);

  stepper.setMaxSpeed(20000);
  stepper.setAcceleration(100000);

  // SCHERMATA DI AVVIO
  compass.begin();
  Wire.begin();

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(2, 1);
  lcd.print("ASTROINSEGUITORE");
  lcd.setCursor(0, 2);
  lcd.print("(c) ilpoliedrico.it");

  tone(BUZZER_PIN, 1200, 500);

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


//---------------------------( RESET COMPLETO    )--------------------------//
void _F_RESET_()
{
  lcd.clear();
  lcd.print( "Riporto la tavoletta chiusa" );
  STATO_INSEGUIMENTO = false;
 // digitalWrite (SLEEP_PIN, HIGH);
  stepper.runToNewPosition (0);
  //digitalWrite (SLEEP_PIN, LOW);
 // digitalWrite (RESET_PIN, HIGH);
  delay(1000);
  //digitalWrite (RESET_PIN, LOW);
}//--------------------------------------------------------------------------//





void _F_AVVIO_()
{
  _F_STEP_();
  TEMPO_INSEGUIMENTO = minuti * 60;
  if (TEMPO_SOLARE > TEMPO_INSEGUIMENTO) TEMPO_SOLARE = 0;;
  STATO_INSEGUIMENTO = true;
  last_steps = 0;
  Serial.println(STEP);
 // digitalWrite (SLEEP_PIN, HIGH);  // /Sveglia il motore
  _F_SIDERALE();
}


void _F_SIDERALE () {

  unsigned long solar_cache = millis();
  /*  float ASTA = (LATO * 2) * sin((pi * TEMPO_INSEGUIMENTO ) / GIORNO_SIDERALE);
    float GIRI = ASTA * FILETTI_CM;
    float uSTEP = GIRI * STEP;
    float _speed = uSTEP / TEMPO_INSEGUIMENTO;
    Serial.println(uSTEP);
    Serial.println(_speed);

  */
  uSTEP = ((LATO * 2) * sin((pi * TEMPO_INSEGUIMENTO ) / GIORNO_SIDERALE)) * FILETTI_CM * STEP;
 
  while (( STATO_INSEGUIMENTO ) && ( TEMPO_SOLARE < TEMPO_INSEGUIMENTO )) {
  
    long _time = millis();
    // Recupera il tempo solare in secondi
    TEMPO_SOLARE = float(millis() - solar_cache ) / 1000.0;
    // Converte il tempo solare in tempo siderale
    TEMPO_SIDERALE = TEMPO_SOLARE * SEC_SIDERALE;
    tr = TEMPO_INSEGUIMENTO - TEMPO_SOLARE;



    if ( PIU.isPressed() ) {
      F_END_TRACK();
    }


    if (millis() > lcount + 4000) {
      statusprint();
      lcount = millis ();

    }
    if (millis() > scount + 5000) {
      scatto();
      scount = millis ();

    }
    // Calcolo l'angolo θ iniziale
    THETA = (TEMPO_SIDERALE / GIORNO_SOLARE) * (pi * 2);

    // Converte θ in gradi per la lettura
    THETA_GRADI = GRADI_DA_RADIANTI (THETA);
    // calcolo il complemento ddell'angolo ψ opposto a θ
    COMPLEMENTO_PSI = THETA / 2;
    // Calculate the INDICE_DI_CORREZIONE distance
    INDICE_DI_CORREZIONE = 1.25 * tan(COMPLEMENTO_PSI);
    //applico la correzione
    rc = LATO - INDICE_DI_CORREZIONE;

    // Calculate d, the distance the threaded rod needs to travel
    d = (rc * sin(THETA)) / sin((pi - THETA) / 2);
    // Calculate the number of steps needed to
    unsigned long steps = (d * FILETTI_CM * STEP);

    while (last_steps < steps)
    {

      stepper.runToNewPosition (steps);

      last_steps = last_steps + 1;


      tstep = millis();

    }
  }

  statusprint();
  tone(BUZZER_PIN, 1440, 2000);
  delay (6000);
  F_END_TRACK();
}
//--------------------------------(   STAMPA LO STATO DI AVAMZAMENTO DELLA FASE DI INSEGUIMENTO SIDERALE   )-----------------------------//
void statusprint () {

  lcd.setCursor(0, 0);



  lcd.print("T. siderale = "); lcd.print(TEMPO_SIDERALE);
  lcd.setCursor(0, 1);
  lcd.print("Time remain= "); lcd.print(tr);
  lcd.setCursor(0, 2);

  lcd.print("Steps= "); lcd.print(stepper.currentPosition());
  lcd.print(" - ");
  lcd.print( uSTEP - stepper.currentPosition());
             lcd.setCursor(0, 3);

             lcd.print("THETA Angle = "); lcd.print(THETA_GRADI);
   

}

//--------------------------------(   ROUTINE DI SCATTO REMOTO   )-----------------------------//
void scatto() {

  unsigned long currentMillis = millis();

  if (millis() - previousMillis >= (secondi * 1000))
  {
    previousMillis = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(FOTO_PIN, ledState);

  }
}

//--------------------------------(   FINE DEL CICLO   )-----------------------------//
void F_END_TRACK() {
  STATO_INSEGUIMENTO = false;
  //digitalWrite (SLEEP_PIN, LOW);
  // digitalWrite (RESET_PIN, HIGH);
  delay(1000);
  // digitalWrite (RESET_PIN, LOW);
  return;
}
//--------------------------------(   STABILISCE IL NUMERO DEI MICROSTEP   )-----------------------------//
void _F_STEP_() {
  STEP = 400;
  digitalWrite(MS0_PIN, HIGH );
  MS0_Stato = HIGH;
  digitalWrite(MS1_PIN, HIGH );
  MS1_Stato = HIGH;
  digitalWrite(MS2_PIN, HIGH );
  MS2_Stato = HIGH;


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
  Serial.println(STEP);
}
float GRADI_DA_RADIANTI(float angolo) {
  float gradi;
  gradi= angolo * 180 / pi;
return gradi;
}

