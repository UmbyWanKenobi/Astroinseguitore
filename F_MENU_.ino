
//--------------------------------------(        MENU' PRINCIPALE      )---------------------------------//
void _F_MENU_()
{
  lcd.setCursor(0, 1);

  // LOOP PRIMO MENU'
  if ( PIU.isPressed () || MENO.isPressed () || SELECT.isPressed () )
  {
    if ( PIU.isPressed () ) {
      pos++;
    }
    if ( MENO.isPressed () ) {
      pos--;
    }
    if ( pos > 4 ) pos = 0;
    if ( pos < 0 )  pos = 4;
    print_menu(pos, row);
    delay(200);
    if ( SELECT.isPressed () ) {
      lcd.clear();
      switch ( pos )
      {
        //  IMPOSTAZIONI
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
        case 4:
          _F_CALIBRAZIONE();
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

//-----------------------(   MENU' IMPOSTAZIONI DELLE FUNZIONI PRINCIPALI   )-------------------//
void _F_SET_()
{
  int i = 0;
  boolean scelta = true;

 
  unsigned int _deg, _D, _min, _M;

  while ( i < 3 )
  {


    if ( scelta )
    {
     
      lcd.clear();  lcd.setCursor((20 - strlen("IMPOSTAZIONI")) / 2, 0);
      lcd.print("IMPOSTAZIONI");
      lcd.setCursor((20 - strlen(MENU[1][i])) / 2, 1);
      lcd.print( MENU[1][i] );
      scelta = false;
    }
    //  CICLO DI IMMISSIONE DEI PARAMETRI
    if ( PIU.isPressed () || MENO.isPressed () || SELECT.isPressed () )
    {
      if ( PIU.isPressed () )
      {
        if ( i == 0 ) {
          minuti += 1;
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
        if ( i == 2 ) {                                                                               //  In questa routine viene impostata la DECLINAZIONE MAGNETICA
          DECL_MAGN += 0.0003;                                                                             //  per la località di osservazione. Essa si può ricavare visitando il sito
          Split_into_dms(DECL_MAGN, &_deg, &_min);                                                         //  http://www.magnetic-declination.com/ cercando la propria località
          _D = _deg;                                                                                  //  anche se la stima di default (2°45') copre abbastanza bene l'Italia Centrale.
          _M = _min;                                                                                  //  L'utente vedrà visualizzato il valore in gradi sessadecimali 
          sprintf (_buffer, "%s:", MENU[2][i]);                                                       //  mentre l'azione di input avviene tutta in radianti, per le ragioni di
          lcd.setCursor((20 - strlen( _buffer )) / 2, 2);                                             //  calcolo di Arduino.
          lcd.print (_buffer);                                                                        //  infatti la routine prevede l'incremento - e il decremento, vedi più sotto -
          row = 3; clear_row(row);                                                                    //  di un valore pari a 0,0003, equivalente all'incirca ad un primo d'arco,
          sprintf(_buffer, "%3d\337%2d'", _D, _M);                                                    //  più che sufficiente per lo  scopo.
          lcd.setCursor((20 - strlen( _buffer )) / 2, row);                                           //  L'uso di due variabili temporanee (_D e _M) si è rivelato necesssario per
          lcd.print( _buffer );                                                                       //  motivi legati al'errata interpretazione dei puntatori (&_deg e &_min) da parte 
        }                                                                                             //  del processo di scrittura a video.
      }
      if ( MENO.isPressed () )
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
          DECL_MAGN -= 0.0003;
          Split_into_dms(DECL_MAGN, &_deg, &_min);
          _D = _deg;
          _M = _min;
          sprintf (_buffer, "%s:", MENU[2][i]);
          lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
          lcd.print (_buffer);
          row = 3; clear_row(row);
          sprintf(_buffer, "%3d\337%2d'", _D, _M);
          lcd.setCursor((20 - strlen( _buffer )) / 2, row);
          lcd.print( _buffer );
        }
      }
      if ( SELECT.isPressed () )
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
void Split_into_dms( float angle, unsigned int* _deg, unsigned int* _min ) {
  float t;
  unsigned int d, m;
  float s;

  Serial.println(angle);
  if (angle < 0.0) {
    angle = - angle;
  }
  angle= GRADI_DA_RADIANTI(angle);
  //Serial.println(angle);
  d = (unsigned int)angle;
  // Serial.println(d);
  t = (angle - (float )d) * 60.0;
  //  Serial.println(t);
  m = (unsigned int)(t);


  if (m == 60) {
    m = 0;
    d++;
  }

  *_deg = d;
  *_min = m;

  //  Serial.println(_deg);
  Serial.println(angle);


}

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
      sprintf (_buffer, "%s: %s", MENU[2][i], String(DECL_MAGN).c_str());
      lcd.setCursor((20 - strlen( _buffer )) / 2, 2);
      lcd.print (_buffer);

    }

    delay( 3000 );
  }
}
//--------------------------------------------------------------------------------------

