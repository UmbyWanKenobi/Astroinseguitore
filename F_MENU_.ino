
//--------------------------------------(        MENU' PRINCIPALE      )---------------------------------//
void _F_MENU_()
{
  lcd.setCursor(0, 1);
  int p_avanti = digitalRead(PIU_PIN);
  int p_indietro = digitalRead(MENO_PIN);
  int p_invio = digitalRead(SELECT_PIN);
  // LOOP PRIMO MENU'
  if ( p_avanti == LOW || p_indietro == LOW || p_invio == LOW)
  {
    if ( p_avanti == LOW ) {
      pos++;
    }
    if ( p_indietro == LOW ) {
      pos--;
    }
    if ( pos > 3 ) pos = 0;
    if ( pos < 0 )  pos = 3;
    print_menu(pos, row);
    delay(200);
    if ( p_invio == LOW ) {
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
    if ( p_avanti ==  LOW || p_indietro ==  LOW || p_invio ==  LOW)
    {
      if ( p_avanti ==  LOW )
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
      if ( p_indietro ==  LOW )
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
      if ( p_invio ==  LOW )
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
