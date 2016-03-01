
void _F_CALIBRAZIONE() {
  boolean calibrazione = true;
  unsigned int _deg, _D, _min, _M;
  lcd.clear();

  
  while (calibrazione) {
    sensors_event_t event;
    compass.getEvent(&event);
    //
    float Asse_x = event.magnetic.x;
    float Asse_y = event.magnetic.y;
    float Asse_z = event.magnetic.z;

    float ago = atan2(Asse_y, Asse_x) + DECL_MAGN;   // Valore espresso in radianti

    // Corregge un valore negativo
    if (ago < 0)   ago += 2 * pi;
    // Controlla la scala.
    if (ago > 2 * pi)ago -= 2 * pi;
    clear_row(0);
    lcd.print("X");
    lcd.print(Asse_x);
    lcd.print(" Y");
    lcd.print(Asse_y);
    lcd.print(" Z");
    lcd.print(Asse_z);


    lcd.setCursor(0, 1);

    Split_into_dms(ago, &_deg, &_min);
    _D = _deg;
    _M = _min;
    sprintf(_buffer, "ANGOLO: %3d\337%2d'", _D, _M);

    clear_row(1);
    lcd.print( _buffer );
    clear_row(2);
    lcd.print("Radianti: ");
    lcd.print(ago);

    if (( _D >= 338 && _D <= 360 ) or (_D >= 0 && _D <=  22 )) {      // NORD
      clear_row(3);
      lcd.setCursor((20 - strlen(MENU[3][0])) / 2, 3);
      lcd.print( MENU[3][0] );
    }
    else if ( _D >=  23 &&  _D <=  67 ) {                       // NORD-OVEST
      clear_row(3);
      lcd.setCursor((20 - strlen(MENU[3][1])) / 2, 3);
      lcd.print( MENU[3][1] );
    }
    else if ( _D >=  68 && _D <=  112 ) {                            // OVEST
      clear_row(3);
      lcd.setCursor((20 - strlen(MENU[3][2])) / 2, 3);
      lcd.print( MENU[3][2] );
    }
    else if ( _D >=  113 && _D <=  157 ) {                       // SUD-OVEST
      clear_row(3);
      lcd.setCursor((20 - strlen(MENU[3][3])) / 2, 3);
      lcd.print( MENU[3][3] );
    }
    else if ( _D >=  158 && _D <=  202 ) {                            // SUD
      clear_row(3);
      lcd.setCursor((20 - strlen(MENU[3][4])) / 2, 3);
      lcd.print( MENU[3][4] );
    }
    else if ( _D >=  203 && _D <=  247 ) {                         // SUD-EST
      clear_row(3);
      lcd.setCursor((20 - strlen(MENU[3][5])) / 2, 3);
      lcd.print( MENU[3][5] );
    }
    else if ( _D >=  248 && _D <=  292 ) {                             // EST
      clear_row(3);
      lcd.setCursor((20 - strlen(MENU[3][6])) / 2, 3);
      lcd.print( MENU[3][6] );
    }
    else if ( _D >=  293 && _D <=  337 ) {                        // NORD-EST
      clear_row(3);
      lcd.setCursor((20 - strlen(MENU[3][7])) / 2, 3);
      lcd.print( MENU[3][7] );
    }
    if  ( SELECT.isPressed () ){
      calibrazione = false;
    }
    delay(1000);
  }

}

