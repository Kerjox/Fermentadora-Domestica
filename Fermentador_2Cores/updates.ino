bool checkforUpdates(){

  String data_To_Send = "IDplaca=";
  data_To_Send.concat(id_Board);
  data_To_Send.concat("&currentVersion=" + currentVersion);
  String datos = peticion("checkforUpdates.php", data_To_Send);
  //Serial.println(datos_Enviar);
    if (datos == "fallo") {

      #ifdef pantallaLCD
        Serial.println("------------------------------");
        Serial.println("Error al comprobar actualizaciones");
        Serial.println("------------------------------");
      #endif
      #ifdef pantallaLCD
        printLCD(0, 0, " Error checking", 1, 0, "   updates");
      #endif

    }else{

      int n = s.separa(datos, ':', 0).toInt();
      int u = s.separa(datos, ':', 1).toInt();

      Serial.println("---------------------");
      Serial.println(datos);
      Serial.println(n);
      Serial.println(u);

      if (n == 1){

        #ifdef pantallaLCD
          printLCD(0, 0, " Actualizacion", 1, 0, "---Disponible---");
        #endif

        Serial.println("Actualización disponible");
        delay(5000);

        if (u == 1){
          
          #ifdef pantallaLCD
            printLCD(0, 0, " Actualizando", 1, 0, "---No apagar---");
          #endif

          Serial.println("Comenzando proceso de actualización");
          Serial.println("-----------------------------------");
          
          t_httpUpdate_return ret = httpUpdate.update(wifiClient, ipServer, 80, "/firmware/arduino.bin");

          switch (ret) {
            case HTTP_UPDATE_FAILED:

              Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
              break;

            case HTTP_UPDATE_NO_UPDATES:
            
              Serial.println("HTTP_UPDATE_NO_UPDATES");
              break;

            case HTTP_UPDATE_OK:

              Serial.println("HTTP_UPDATE_OK");
              break;
          }
        }
      }else Serial.println("No hay actualizaciones disponibles");
    }
}
