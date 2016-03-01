
void _F_CALIBRAZIONE() {
  boolean calibrazione = true;
  unsigned int _deg, _D, _min, _M;

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
    // Convert radians to degrees for readability.
    //float gradi_ago = GRADI_DA_RADIANTI(ago);
    //Convert float to int
    // int _D=(int)gradi_ago;
    lcd.clear();
    lcd.setCursor(0, 0);

    lcd.print("X");
    lcd.print(Asse_x);
    lcd.print(" Y ");
    lcd.print(Asse_y);
    lcd.print(" ");
    lcd.print(Asse_z);


    lcd.setCursor(0, 1);
    lcd.print("ANGOLO: ");
    Split_into_dms(ago, &_deg, &_min);
    _D = _deg;
    _M = _min;

    sprintf(_buffer, "%3d\337%2d'", _D, _M);

lcd.setCursor(0, 2);
    lcd.print("Radianti: ");
    lcd.print(ago);
    lcd.setCursor(5, 3);
    if (_D >= 0 && _D <= 45) {   // 0-45 degrees
      lcd.print("NORD");
    }
    else if (_D > 45 && _D <= 90) { // 46-90 degrees
      lcd.print("NORD-OVEST");
    }
    else if (_D > 90 && _D <= 135) { // 91-135 degrees
      lcd.print("OVEST");
    }
    else if (_D > 135 && _D <= 180) { // 136-180 degrees
      lcd.print("SUD-OVEST");
    }
    else if (_D > 180 && _D <= 225) { // 181-225 degrees
      lcd.print("SUD");
    }
    else if (_D > 225 && _D <= 270) { // 226-270 degrees
      lcd.print("SUD-EST");
    }
    else if (_D > 270 && _D <= 315) { // 271-315 degrees
      lcd.print("EST");
    }
    else if (_D > 315 && _D <= 360) { // 316-360 degrees
      lcd.print("NORD-EST");
    }
    delay(500);
  }
  //////////////////////////////////////////////////////////////////////////
}

