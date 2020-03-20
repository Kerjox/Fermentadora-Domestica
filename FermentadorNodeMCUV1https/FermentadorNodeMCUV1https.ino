/*
 * PROGRAMA PARA EL MANEJO DE UN FERMENTADOR
 * 
 * Notas de la version 2:
 *  - Estructura del programa.
 *  - Funcion triaje() para diferenciar procesos.
 *  - Programacion de la maceracion.
 *  - Establecimiento de la consigna de comunicacion.
 * 
 * Notas de la version 3:
 *  - Cambio en la consigna de comunicacion.
 *  - Modificar los pines del arduino y las variables configurables a const 
 *    (constantes durante todo el programa) para dar mas robustez al codigo.
 *  - Añadir comentarios en el setup.
 *  - Añadir puesta a cero inicial de las variables de trabajo.
 *  - Incluir y comentar el metodo leer().
 *  - Incluir y comentar el metodo desencriptarTemperatura().
 *  - Incluir y comentar el metodo desencriptarTiempo().
 * Notas de la version 4:
 *  - Cambio y comentarios en los pines.
 *  - Cambio de las variables de tiempo a float.
 *  - Modificacion del metodo de maceracion. Acabado completamente.
 *  - Programacion del metodo de coccion. Acabado completamente.
 *  - Metodo enviarTiempo(). Acabado completamente.
 *  - Metodo finProceso(). Acabadocompletamente.
 *  
 *  Notas de la version 4.1:
 *  - Hemos añadido los pines de reserva en el LAYOUT de pines.
 *  
 *  Notas de la version 5:
 *  - Metodo menu(). Acabado completamente.
 *  - Metodo recircular(). Acabado completamente.
 *  - Metodo calentar(). Acabado completamente.
 *  - Metodo trasvase(). Acabado completamente.
 *  - Metodo fermentacion(). Falta programarlo.
 *  - Modificacion desencriptarTemperatura(). Daba fallo y 
 *    se han modificado los indices del substring.
 *  - Modificacion desencriptarTiempo(). Daba fallo y 
 *    se han modificado los indices del substring.
 *    
 *  Notas de la version 5.1:
 *  - Creacion de variables cronometro para contar segundos.
 *  - Metodo fermentacion(). Acabado completamente.
 *  - Metodo trasvase(). Añadimos tiempo maximo de seguridad.
 *  
 *  Notas de la version 6:
 *  - Comentar todos los metodos completamente.
 *  - Modificacion desencriptarTemperatura(). Daba fallo y 
 *    se han modificado los indices del substring.
 *    (Han dado varios fallos, si da un fallo en todos los procesos puede venir de aqui)
 *  - Modificacion desencriptarTiempo(). Daba fallo y 
 *    se han modificado los indices del substring.
 *    (Han dado varios fallos, si da un fallo en todos los procesos puede venir de aqui)
 *  - Retraso en el arranque de las bombas para el trasvase.
 *  - Añadir pin para el zumbador
 *  
 *  Notas para la version 7: 
 *  - Metodo cancion1(zumbador). Cancion de piratas del caribe.
 *    Para añadir mas canciones usar la misma estructura: Funcion con la cancion cuyo 
 *    parametro sea el pin del zumbador para facilitar la modificacion de pines en un futuro.
 *    FALTA AÑADIR LA CANCION DESPUES DE LOS PROCESOS QUE SE QUIERA.
 *  - Se ajusta el tiempo de trasvase a 3 minutos y medio segun prueba en maqueta
 *  
 *  Notas de la version 7.1:
 *  - Eliminacion de la variable tsensor
 *  - Modificacion desencriptarTemperatura(). Daba fallo y 
 *    se han modificado los indices del substring.
 *    (Han dado varios fallos depende del sistema operativo o versiones del programa)
 *  - Modificacion desencriptarTiempo(). Daba fallo y 
 *    se han modificado los indices del substring.
 *    (Han dado varios fallos depende del sistema operativo o versiones del programa)
 *
 *  Notas de la version 8:
 *  - Actualizacion de pines para el NodeMCU.
 *  - Eliminacion de electrovalvulas y bomba principal, por Bomba Recirculacion y por
 *    Bomba Trasvase.
 *  - Se ha renombrado el pin reserva1 por peltier, para las Celulas Peltier.
 *  - Se ha renombrado el pin reserva2 por reserva, para tener un pin de reserva.
 *  - En la ecuacion de temperatura hemos cambiado el valor del voltaje de referencia
 *    de 5000mV (Arduino) a 3300mV (NodeMCU).
 */

/*
 * PINES Y VARIABLES
 */
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClientSecureBearSSL.h>
  #include <Wire.h>                           // incluye libreria para interfaz I2C
  #include <RTClib.h>                         // incluye libreria para el manejo del modulo RTC
  #include <TimeLib.h>
  
//LAYOUT Pines
  #define pinSonda A0                    //Sonda de la temperatura
  #define resis D0                       //Resistencia para calentar               
  #define bombaRecirculacion D3          //Bomba de recirculacion 230V
  #define bombaTrasvase D4               //Bomba trasvase 230V
  #define bombaFrio D5                   //Bomba refrigeracion 230V
  #define peltier D6                     //Celulas Peltier
  #define sensorLiquido D7               //Sensor de liquido en tubo
  
//Variables configurables
  const float anchoVentana = 1;               //Rango para la temperatura
  const float tiempoTrasvase = 210000;        //Tiempo maximo de seguridad que dura el trasvase (Se pone 4 minutos)
  const int retrasoBombas = 1000;             //Tiempo de retraso entre el arranque de la bomba frio y el resto

//Variables globales
  const char* ssid = "";                      //Nombre de la red WiFi a la que se va a conectar
  const char* password = "";                  //Contraseña de la red WiFi a la que se va a conectar
  int dato;                                   //Dato leido para entrar el menu
  unsigned long tiempoi;                      //Tiempo inicial para los procesos en seg
  unsigned long tiempof;                      //Tiempo final para los procesos en seg
  unsigned long tiempoActual;                 //Tiempo actual del proceso en seg
  long tiempoRestante;                        //Tiempo que falta para el final de los procesos en seg
  float tempMacer;                            //Temperatura de maceración de la receta seleccionada
  unsigned long tiempoMacer;                  //Tiempo maceración de la recta selecionada
  float tempCoc;                              //Temperatura de cocción de la receta seleccionada
  unsigned long tiempoCoc;                    //Tiempo cocción de la recta selecionada
  unsigned long tiempoTrans;                  //Tiempo transvase de la recta selecionada
  float tempFermen;                           //Temperatura de fermentación de la receta seleccionada
  unsigned long tiempoFermen;                 //Tiempo fermentación de la recta selecionada
  bool falloProceso = 0;                      //Guarda si falla el tiempo
  unsigned char procesoActual;
  unsigned char estado;
  String mac;
  int IDplaca;
  //const uint8_t fingerprint[20] = {0x5A, 0xCF, 0xFE, 0xF0, 0xF1, 0xA6, 0xF4, 0x5F, 0xD2, 0x11, 0x11, 0xC6, 0x1D, 0x2F, 0x0E, 0xBC, 0x39, 0x8D, 0x50, 0xE0};
    
//Objetos
  HTTPClient http;                              // Object of the class HTTPClient.
  RTC_DS3231 rtc;                               // crea objeto del tipo RTC_DS3231

/*
 * CICLO DE ARRANQUE
 * Configuramos los pines. Puesta a cero inicial.
 */
void setup(){
  
//Inicializamos el puerto serie
  Serial.begin(115200);
  delay(10);
  
//Iniciamos la comunicación con el RTC
  Wire.begin(D2,D1);
  
// Conectar con la red WiFi
  Serial.println("");
  Serial.print("Connecting");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {       //Mostrar ... mientras se conacta al WiFi
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());               //Mostrar la IP que tiene el dispositivo
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  mac = WiFi.macAddress();

//Solicitud de Identificador de placa según la mac
  while (true){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //client->setFingerprint(fingerprint);
    client->setInsecure();
    String consulta = "https://192.168.1.150/arduino/get_id.php?mac=";
    consulta = consulta + mac;
    Serial.println(consulta);
    http.begin(*client, consulta);  // Request destination.
    int httpCode = http.GET(); // Send the request.
      if (httpCode == 200 || httpCode == 201) {
        String stringIDplaca = http.getString();
        http.end();
        IDplaca = stringIDplaca.toInt();
        break;
      }else{
        Serial.println("------------------------------");
        Serial.println("Error al solicitar el identificador de palca");
        Serial.println("------------------------------");
      }
  }
  
//Configuracion de pines
  pinMode(resis,OUTPUT);
  pinMode(bombaRecirculacion,OUTPUT);
  pinMode(bombaTrasvase,OUTPUT);
  pinMode(bombaFrio,OUTPUT);
  pinMode(peltier,OUTPUT);
  pinMode(sensorLiquido,INPUT);
  
//Puesta a cero inicial de las variables de trabajo
  tiempoi = 0;
  tiempof = 0;
  tiempoActual = 0;
  tiempoRestante = 0;
}
/*
 * CICLO PRINCIPAL
 * Indicamos a la Raspberry que hemos arrancado. 
 * Leemos datos en bucle hasta entrar en un proceso.
*/
  

void loop(){
  Serial.println(IDplaca);
  
//Mensaje inicial
  Serial.println("------------------------------");
  Serial.println("Ready");
  Serial.println("------------------------------");
  pregunta();
  menuinicio(dato);
}
void pregunta(){
  while(true){
  delay(100);
  if (WiFi.status() == WL_CONNECTED){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //client->setFingerprint(fingerprint);
    client->setInsecure();
    String consulta = "https://192.168.1.150/arduino/menu.php?menu=1&IDplaca=";
    consulta = consulta + IDplaca;
    http.begin(*client, consulta);  // Request destination.
    int httpCode = http.GET(); // Send the request.
      if (httpCode == 200 || httpCode == 201) {
        String datoString = http.getString();
        http.end();
        dato = datoString.toInt();
        //Serial.println(dato);
        if (dato != 0){
          String consulta = "https://192.168.1.150/arduino/menu.php?resetmenu=1&IDplaca=";
          consulta = consulta + IDplaca;
          http.begin(*client, consulta);
          http.GET();
          http.end();
          break;
        }
        }else{
          Serial.println("El servidor no responde");
        }
        
      }
  }
}

/*
 * Funcion menu.
 * Es el encargado de gestionar el numero de proceso, del 1 al 9, y 
 * elegir el proceso a realizar.
 * Datos del menu:
 *  - 1: Maceracion.
 *  - 2: Coccion.
 *  - 3: Trasvase.
 *  - 4: Fermentacion.
 *  
 *  Parametros: numero de proceso introducido como caracter
 *  No devuelve nada
 */
void menuinicio(int n){ 
       if (n==1) { receta();}
  else if (n==2) { ajustes();}
  else if (n==3) { procesos();}
  else if (n==4) { limpieza();}
  else Serial.println("La accion deseada no existe");
}

void receta(){
  Serial.println("------------------------------");
  Serial.println("Selecciona receta: ");
  Serial.println("------------------------------");
  pregunta();
  leerdatos(dato);
}

void ajustes(){
  Serial.println("Ajustes");
  pregunta();
       if (dato==1) { time_set();}
  else if (dato==2) { showtime();}
  else Serial.println("La accion deseada no existe");
}

void procesos(){
  if (tiempoMacer == 0){
    Serial.println("Primero selecciona una receta");
    return;
  }
  Serial.println("Selecciona proceso: ");
  pregunta();
       if (dato==1) { maceracion(); }
  else if (dato==2) { coccion();}
  else if (dato==3) { trasvase();}
  else if (dato==4) { fermentacion();}
  else Serial.println("Proceso no existente");
}

void limpieza(){
    Serial.println("------------------------");
    Serial.println("Se ha iniciado la limpieza");
    Serial.println("    Por favor, espere");
    Serial.println("------------------------");
    trasvase();
}

void showtime(){
  DateTime now = rtc.now();
  tiempoActual = now.unixtime();
  Serial.println(tiempoActual);
  DateTime fecha = rtc.now();      // funcion que devuelve fecha y horario en formato
            // DateTime y asigna a variable fecha
        Serial.print(fecha.day());     // funcion que obtiene el dia de la fecha completa
        Serial.print("/");       // caracter barra como separador
        Serial.print(fecha.month());     // funcion que obtiene el mes de la fecha completa
        Serial.print("/");       // caracter barra como separador
        Serial.print(fecha.year());      // funcion que obtiene el año de la fecha completa
        Serial.print(" ");       // caracter espacio en blanco como separador
        Serial.print(fecha.hour());      // funcion que obtiene la hora de la fecha completa
        Serial.print(":");       // caracter dos puntos como separador
        Serial.print(fecha.minute());      // funcion que obtiene los minutos de la fecha completa
        Serial.print(":");       // caracter dos puntos como separador
        Serial.println(fecha.second());    // funcion que obtiene los segundos de la fecha completa
}

void gettime(){
  DateTime now = rtc.now();
  tiempoActual = now.unixtime();
}

void time_set (){
  if (WiFi.status() == WL_CONNECTED){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //client->setFingerprint(fingerprint);
    client->setInsecure();
    http.begin("https://192.168.1.150/arduino/time.php");  // Request destination.
    int httpCode = http.GET(); // Send the request.
      if (httpCode == 200 || httpCode == 201) {
        String stringtime = http.getString();
        http.end();
        unsigned long timeset = (long) strtol(stringtime.c_str(),NULL,0);
        rtc.adjust(DateTime(year(timeset),month(timeset),day(timeset),hour(timeset),minute(timeset),second(timeset)));
        DateTime fecha = rtc.now();      // funcion que devuelve fecha y horario en formato
            // DateTime y asigna a variable fecha
        Serial.print(fecha.day());     // funcion que obtiene el dia de la fecha completa
        Serial.print("/");       // caracter barra como separador
        Serial.print(fecha.month());     // funcion que obtiene el mes de la fecha completa
        Serial.print("/");       // caracter barra como separador
        Serial.print(fecha.year());      // funcion que obtiene el año de la fecha completa
        Serial.print(" ");       // caracter espacio en blanco como separador
        Serial.print(fecha.hour());      // funcion que obtiene la hora de la fecha completa
        Serial.print(":");       // caracter dos puntos como separador
        Serial.print(fecha.minute());      // funcion que obtiene los minutos de la fecha completa
        Serial.print(":");       // caracter dos puntos como separador
        Serial.println(fecha.second());    // funcion que obtiene los segundos de la fecha completa
      }
  }
 }


/*  
 *  Funcion para realizar MACERACION.
 *  Avisar a la Raspberry de que esta preparado para empezar el proceso. 
 *  Recibe una consigna y la desencripta volcando la temperatura y el tiempo en variables.
 *  La consigna empieza por "T", seguida de cuatro numeros (centenas de temperatura, decenas de 
 *  temperatura, unidades de temperatura y decimas de temperatura), seguidas por "S" y el tiempo
 *  del proceso en segundos, acabando la consigna en "."
 *  Se pone en modo recirculacion y realiza el ciclo de calentamiento hasta la temperatura 
 *  recibida durante el tiempo recibido con un rango de temperatura especificado en la constante.
 *  Encripta las variables y las envia constantemente para que la Raspberry tenga la informacion 
 *  del tiempo restante del proceso.
 *  Una vez alcanzado el tiempo envia mensaje de fin.  
 *  
 *  Parametros: No lleva parametros
 *  No devuelve nada
 */

  
void maceracion (){
//Confirmacion del inicio de proceso de maceracion
  Serial.println("O1");
  procesoActual = 1;
  estado = 1;
  logInfo(procesoActual,estado);
  
//LECTURA DE VARIABLES
  float temperaturaMaceracion = tempMacer;                          //Variable con la temperatura del proceso
  float tiempoMaceracion = tiempoMacer;                             //Variable del tiempo del proceso en minutos
//MODO RECIRCULACION
  recircular();
  
//CICLO DE CALENTAMIENTO
  calentar(temperaturaMaceracion,tiempoMaceracion);
  
//APAGADO DE BOMBAS Y RELES
  digitalWrite(resis,LOW);
  digitalWrite(bombaRecirculacion,LOW);
  digitalWrite(bombaTrasvase,LOW);
  digitalWrite(bombaFrio,LOW);
  digitalWrite(peltier,LOW);
  
//Envio mensaje de fin de proceso.
  if (falloProceso) estado = 3;
  else estado = 2;
  logInfo(procesoActual,estado);
  finProceso(procesoActual,falloProceso);
  
}

/*
 *  Funcion para realizar COCCION.
 *  Avisar a la Raspberry de que esta preparado para empezar el proceso. 
 *  Recibe una consigna y la desencripta volcando la temperatura y el tiempo en variables.
 *  La consigna empieza por "T", seguida de cuatro numeros (centenas de temperatura, decenas de 
 *  temperatura, unidades de temperatura y decimas de temperatura), seguidas por "S" y el tiempo
 *  del proceso en segundos, acabando la consigna en "."
 *  Se pone en modo recirculacion y realiza el ciclo de calentamiento hasta la temperatura 
 *  recibida durante el tiempo recibido con un rango de temperatura especificado en la constante.
 *  Encripta las variables y las envia constantemente para que la Raspberry tenga la informacion 
 *  del tiempo restante del proceso.
 *  Una vez alcanzado el tiempo envia mensaje de fin.  
 *  
 *  Parametros: No lleva parametros
 *  No devuelve nada
 */
void coccion (){ 
//Confirmacion del inicio de proceso de cocción
  Serial.println("O2");
  procesoActual = 2;
  estado = 1;
  logInfo(procesoActual,estado);
  
//LECTURA DE VARIABLES            
  float temperaturaCoccion = tempCoc;                         //Variable con la temperatura del proceso
  float tiempoCoccion = tiempoCoc;                            //Variable del tiempo del proceso en minutos

//MODO RECIRCULACION
  recircular();

//CICLO DE CALENTAMIENTO
  calentar(temperaturaCoccion, tiempoCoccion);
  
//APAGADO DE BOMBAS Y RELES
  digitalWrite(resis,LOW);
  digitalWrite(bombaRecirculacion,LOW);
  digitalWrite(bombaTrasvase,LOW);
  digitalWrite(bombaFrio,LOW);
  digitalWrite(peltier,LOW);
//Envio mensaje de fin de proceso.
  if (falloProceso) estado = 3;
  else estado = 2;
  logInfo(procesoActual,estado);
  finProceso(procesoActual,falloProceso);
}

/*
 *  Funcion para realizar TRASVASE.
 *  Avisar a la Raspberry de que esta preparado para empezar el proceso y activa las 
 *  bombas y reles necesarios.
 *  Se pone en modo trasvase hasta que recibe un mensaje de fin que viene dado por la 
 *  siguiente consigna: "T0000S0."
 *    
 *  Parametros: No lleva parametros
 *  No devuelve nada
 */
void trasvase(){
//Confirmacion del inicio de proceso de trasvase
  Serial.println("O3");
  procesoActual = 3;
  estado = 1;
  logInfo(procesoActual,estado);
  
  
//Trasvase ON
  digitalWrite(bombaFrio,HIGH);
  delay(retrasoBombas);
  digitalWrite(bombaRecirculacion,LOW);
  digitalWrite(bombaTrasvase,HIGH);
  digitalWrite(peltier,HIGH);

//Control de tiempo y sensor de liquido
    Serial.println("------------------------");
    Serial.print("El tiempo de seguridad es de: ");
    Serial.print("10");
    Serial.println(" Minutos");
    Serial.println("------------------------");
    gettime();
    tiempoi = tiempoActual;
    tiempof = tiempoi + (10 * 60);
    long tiempoCancelacion = tiempoActual + 5;
    do{
      gettime();
      tiempoRestante = tiempof - tiempoActual;
      if (tiempoActual >= tiempoCancelacion){
        tiempoCancelacion = tiempoActual + 5;
        comprobarCancelar();
        if (falloProceso){
          break;
        }
      }
      if (tiempoRestante <= 0 || sensorLiquido == HIGH) break;
    delay(1000);
  }while(true);
//Trasvase OFF  
  digitalWrite(bombaTrasvase,LOW);
  digitalWrite(peltier,LOW);
  digitalWrite(bombaFrio,LOW);
  
//Envio mensaje de fin de proceso.
  if (falloProceso) estado = 3;
  else estado = 2;
  logInfo(procesoActual,estado);
  finProceso(procesoActual,falloProceso);
}



/*
 *  Funcion para realizar FERMENTACION.
 *  Avisar a la Raspberry de que esta preparado para empezar el proceso. 
 *  Recibe una consigna y la desencripta volcando la temperatura y el tiempo en variables.
 *  Finaliza el proceso cuando acaba el tiempo
 *  Encripta las variables y las envia.
 *  Una vez alcanzado el tiempo envia mensaje de fin.  
 *  
 *  Parametros: No lleva parametros
 *  No devuelve nada
 */
void fermentacion(){
//Confirmacion para RASPBERRY del inicio de proceso de fermentacion
  Serial.println("O4");
  procesoActual = 4;
  estado = 1;
  logInfo(procesoActual,estado);
  
//LECTURA DE VARIABLES
  float temperaturaFermentacion = tempFermen;
  long tiempoFermentacion = tiempoFermen;
  gettime();
  tiempoi = tiempoActual;
  tiempof = tiempoi + (tiempoFermen * 2628000);
  long tiempoCancelacion = tiempoActual + 5;
  long tiempoMtiempo = tiempoActual;
  do{
    gettime();
    tiempoRestante = tiempof - tiempoActual;
    if (tiempoActual >= tiempoCancelacion){
      tiempoCancelacion = tiempoActual + 5;
      comprobarCancelar();
      if (falloProceso){
        break;
      }
    }
    if (tiempoActual >= tiempoMtiempo){
      tiempoMtiempo = tiempoActual + 60;
      Serial.print(day(tiempoRestante));     // funcion que obtiene el dia de la fecha completa
      Serial.print(" dias /");       // caracter barra como separador
      Serial.print(hour(tiempoRestante));      // funcion que obtiene la hora de la fecha completa
      Serial.print(":");       // caracter dos puntos como separador
      Serial.println(minute(tiempoRestante));      // funcion que obtiene los minutos de la fecha completa
    }
    if (tiempoRestante <= 0) break;
      delay(1000);
  }while(true);
//PUESTA A CERO FINAL
  tiempoi = 0;
  tiempof = 0;
  tiempoActual = 0;
  
//Envio mensaje de fin de proceso.
  if (falloProceso) estado = 3;
  else estado = 2;
  logInfo(procesoActual,estado);
  finProceso(procesoActual,falloProceso);
}



/*
 * Metodo para recircular. 
 * Sirve poner los reles y bombas en la posicion de recirculacion.
 * 
 * No tiene parametros
 * No devuelve nada
 */
void recircular(){
  digitalWrite(bombaRecirculacion,HIGH);
  digitalWrite(bombaTrasvase,LOW);
  digitalWrite(peltier,LOW);
  digitalWrite(bombaFrio,LOW);
}

/*
 * Metodo calentar. 
 * Sirve realizar un ciclo de calentamiento hasta la temperatura alcanzar la temperatura pasada como parametro
 * Parametros: tiempo del proceso en segundos
 *             temperatura del proceso en grados
 * No devuelve nada
 */
void calentar( float temperaturaProceso, long tiempoProceso){
//TRATAMIENTO DE LAS VARIABLES
  //Tratamiento de la ventana de temperatura
    Serial.println("------------------------");
    Serial.print("El proceso dura: ");
    Serial.print(tiempoProceso);
    Serial.println(" Minutos");
    Serial.println("------------------------");
    float tmax = temperaturaProceso+anchoVentana;
    float tmin = temperaturaProceso-anchoVentana;
    
    gettime();
    tiempoi = tiempoActual;
    tiempof = tiempoi + (tiempoProceso * 60);
    long tiempoCancelacion = tiempoActual + 5;
    do{
      if (tiempoActual >= tiempoCancelacion){
        tiempoCancelacion = tiempoActual + 5;
        comprobarCancelar();
        if (falloProceso){
          break;
        }
      }
      
      gettime();
      tiempoRestante = tiempof - tiempoActual;
      //enviarTiempo(tiempoRestante);
      Serial.print("Quedan ");
      Serial.print(minute(tiempoRestante));
      Serial.print(" Min y ");
      Serial.print(second(tiempoRestante));
      Serial.println(" Segundos");
  //Tratamiento de la temperatura
    int sensorTemperatura = analogRead(pinSonda);
    float milivoltios = (sensorTemperatura / 1023.0) * 3300;
    float celsius = milivoltios / 10;
  //Mantenimiento de la ventana de temperatura
    if(celsius > tmax){digitalWrite(resis,LOW);}
    if(celsius < tmin){digitalWrite(resis,HIGH);}
    if (tiempoRestante <= 0) break;
    delay(1000);
  }while(true);
  
//PUESTA A CERO FINAL
  tmax = 0;
  tmin = 0;
  tiempoi = 0;
  tiempof = 0;
  tiempoActual = 0;
}

void comprobarCancelar() {
  if (WiFi.status() == WL_CONNECTED){
    Serial.println("Comprobación de cancelación");
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //client->setFingerprint(fingerprint);
    client->setInsecure();
    String consulta = "https://192.168.1.150/arduino/menu.php?fallo=1&IDplaca=";
    consulta = consulta + IDplaca;
    http.begin(*client,consulta);  // Request destination.
    int httpCode = http.GET(); // Send the request.
      if (httpCode == 200 || httpCode == 201) {
        String stringcancelar = http.getString();
        int cancelar = stringcancelar.toInt();
        if (cancelar == 1){
          falloProceso = 1;
          http.end();
        }
    }else{
      Serial.println("El servidor no responde");
    }
   }
}

/*
 * Metodo leer. 
 * Sirve para leer una cadena de caracteres que se recibe de la SQL
 * y se almacena en un String. Los datos vienen separados por ";".
 * 
 * Contiene el parametro del ID de la receta que tiene que leer de la SQL.
 * Asigna los valores oportunos a las variables de cotrol (tmperatura y tiempo de cada proceso).
 */

void leerdatos(int n){
  if (WiFi.status() == WL_CONNECTED) {
    
    String peticion = "https://192.168.1.150/arduino/pedirdatos.php?id=";
    peticion = peticion + n;
    Serial.println("------------------------------");
    Serial.print("Petición al servidor: ");
    Serial.println(peticion);
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //client->setFingerprint(fingerprint);
    client->setInsecure();
    http.begin(*client,peticion);  // Request destination.
    int httpCode = http.GET(); // Send the request.
    Serial.println(httpCode);
    if (httpCode == 200 || httpCode == 201) { //Check the returning code
      String datos = http.getString(); 
      http.end();   //Close connection
      Serial.println("------------------------------");                           // Obtiene la string
      Serial.print("String recibida: ");
      Serial.println(datos);
      int longitud = datos.length();
      Serial.println("------------------------------");

    //Procesar datos nombre Receta
      int pnombre = datos.indexOf("nombre=");
      String nombre = "";
      for (int i = pnombre + 7; i < longitud; i ++){
        if (datos[i] == ';') i = longitud;
        else nombre += datos[i];
      }
      

    //Procesar datos de la Temperatura de Maceración
      int ptempMacer = datos.indexOf("tempMacer=");               //Posicion de temp
      String stempMacer = "";
      for (int i = ptempMacer + 10; i < longitud; i ++){
        if (datos[i] == ';') i = longitud;
        else stempMacer += datos[i];
      }
      tempMacer = stempMacer.toFloat();
      

    //Procesar datos tiempo la Maceración
      int ptiempoMacer = datos.indexOf("tiempoMacer=");
      String stiempoMacer = "";
      for (int i = ptiempoMacer + 12; i < longitud; i ++){
        if (datos[i] == ';') i = longitud;
        else stiempoMacer += datos[i];
      }
      tiempoMacer = (long) strtol(stiempoMacer.c_str(),NULL,0);
      

    //Procesar datos de la Temperatura de Cocción
      int ptempCoc = datos.indexOf("tempCoc=");
      String stempCoc = "";
      for (int i = ptempCoc + 8; i < longitud; i ++){
        if (datos[i] == ';') i = longitud;
        else stempCoc += datos[i];
      }
      tempCoc = stempCoc.toFloat();
      

    //Procesar datos tiempo de Cocción
      int ptiempoCoc = datos.indexOf("tiempoCoc=");
      String stiempoCoc = "";
      for (int i = ptiempoCoc + 10; i < longitud; i ++){
        if (datos[i] == ';') i = longitud;
        else stiempoCoc += datos[i];
      }
      tiempoCoc = (long) strtol(stiempoCoc.c_str(),NULL,0);
      

    //Procesar datos tiempo del Fermentación
      int ptempFermen = datos.indexOf("tempFermen=");
      String stempFermen = "";
      for (int i = ptempFermen + 11; i < longitud; i ++){
        if (datos[i] == ';') i = longitud;
        else stempFermen += datos[i];
      }
      tempFermen = stempFermen.toFloat();
      

    //Procesar datos de la Temperatura de Fermentación
      int ptiempoFermen = datos.indexOf("tiempoFermen=");
      String stiempoFermen = "";
      for (int i = ptiempoFermen + 13; i < longitud; i ++){
        if (datos[i] == ';') i = longitud;
        else stiempoFermen += datos[i];
      }
      tiempoFermen = (long) strtol(stiempoFermen.c_str(),NULL,0);
      

    //Mostrar información de la receta por Serial
    if (tiempoMacer != 0){
      //Nombre de la cerveza
        Serial.print("Nombre de la cerveza= ");
        Serial.println(nombre);
      //Temperaturas
        Serial.print("Temperatura del proceso Maceración= ");
        Serial.println(tempMacer);
        Serial.print("Temperatura del proceso Cocción= ");
        Serial.println(tempCoc);
        Serial.print("Temperatura del proceso de Fermentación= ");
        Serial.println(tempFermen);
      //Tiempos en segundos
        Serial.print("Tiempo en Minutos del proceso Maceración= ");
        Serial.println(tiempoMacer);
        Serial.print("Tiempo en Segundos del proceso Cocción= ");
        Serial.println(tiempoCoc);
        Serial.print("Tiempo en Meses del proceso Fermentación= ");
        Serial.println(tiempoFermen);
    }else{
      Serial.println("La receta no existe");
    }
    }else{

      Serial.println("El servidor no responde");

    }

}
}



/*
 * Metodo para enviar el final del proceso con errores.
 * Envia un mensaje a la Rasberry con los errores del proceso.
 * 
 * Parametros: dato  Representa el numero de proceso
 *             error Representa el numero de error (0 si no hay)
 * No devuelve nada
 */
void finProceso (unsigned char proceso,bool error){
//Variables locales
  String mensaje = "Proceso ";
//Conversion a String
  mensaje.concat(proceso);
  mensaje.concat(" Fallo ");
  mensaje.concat(error);

  if (falloProceso){
    if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //client->setFingerprint(fingerprint);
    client->setInsecure();
    String consulta = "https://192.168.1.150/arduino/menu.php?resetfallo=1&IDplaca=";
    consulta = consulta + IDplaca;
    http.begin(*client, consulta);
    http.GET();
    http.end();
    falloProceso = 0;
  }
  }
  
  
//Envia el string por a la Raspberry
  Serial.println(mensaje);
}

void logInfo(unsigned char proceso,unsigned char estado) {
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //client->setFingerprint(fingerprint);
    client->setInsecure();
    String peticion = "https://192.168.1.150/arduino/log.php?proceso=";
    peticion = peticion + proceso;
    peticion = peticion + "&estado=";
    peticion = peticion + estado;
    peticion = peticion + "&IDplaca=";
    peticion = peticion + IDplaca;
    http.begin(*client, peticion);
    http.GET();
    http.end();
  }
}
