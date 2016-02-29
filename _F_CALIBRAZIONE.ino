
void _F_CALIBRAZIONE() {
  boolean calibrazione = true;
  
  while (calibrazione){ 
   sensors_event_t event; 
  compass.getEvent(&event);
  //Variable heading stores value in radians
  float Asse_x = event.magnetic.x;
  float Asse_y = event.magnetic.y;
  float Asse_z = event.magnetic.z;
  
  float heading = atan2(Asse_y, Asse_x);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the compassnetic field in your location.
  // Find yours here: http://www.compassnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  float declinationAngle = RAD_DECL_MAGN;  //<--Change 0.22 with yours. If you can't find your declination juct delete those lines ;)
  heading += declinationAngle;
  // angle = atan2((double)event.magnetic.y,(double)event.magnetic.x)* (180 / 3.141592654) + 180; 
  // Correct for when signs are reversed.
  if(heading < 0)   heading += 2*pi;
  // Check for wrap due to addition of declination.
  if(heading > 2*pi)heading -= 2*pi;
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/pi; 
  //Convert float to int
  int angle=(int)headingDegrees;
  lcd.clear();
  lcd.setCursor(0,0);

  lcd.print("X");
  lcd.print(Asse_x);
  lcd.print(" Y "); 
  lcd.print(Asse_y);
  lcd.print(" "); 
  lcd.print(Asse_z);

  
  lcd.setCursor(1,1);
  lcd.print("ANGOLO: ");
    lcd.print(angle);
    lcd.setCursor(1,2);
    lcd.print("Radianti: ");
    lcd.print(heading);
    lcd.setCursor(5,3);
     if (angle >= 0 && angle <=45) {    // 0-45 degrees
  lcd.print("NORD");
  } 
  else if (angle >45 && angle <=90) {// 46-90 degrees
    lcd.print("NORD-OVEST");
  } 
  else if (angle >90 && angle <=135) {// 91-135 degrees
    lcd.print("OVEST");
  } 
  else if (angle >135 && angle <=180){// 136-180 degrees
   lcd.print("SUD-OVEST");
  } 
  else if (angle >180 && angle <=225){// 181-225 degrees
   lcd.print("SUD");
  } 
  else if (angle >225 && angle <=270){// 226-270 degrees
   lcd.print("SUD-EST");
  } 
  else if (angle >270 && angle <=315){// 271-315 degrees
    lcd.print("EST");
  } 
  else if (angle >315 && angle <=360){// 316-360 degrees
   lcd.print("NORD-EST");
  }
    delay(500);
  }
  //////////////////////////////////////////////////////////////////////////
}

