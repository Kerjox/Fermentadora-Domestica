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
  if (recovery == 1){
    procesoActual = 4;
    estado = 1;
  }else{
//Confirmacion para RASPBERRY del inicio de proceso de fermentacion
  Serial.println("O4");
  procesoActual = 4;
  estado = 1;
  porcentaje = 0;
  sendInfo(procesoActual,pasoProceso);
  }
  
//LECTURA DE VARIABLES
  float temperaturaFermentacion = tempFermen[pasoProceso].toFloat();
  int tiempoFermentacion = tiempoFermen[pasoProceso].toInt();
  if (recovery == 1){
    Serial.println("------------------------");
    Serial.print("El proceso dura: ");
    Serial.print(month(recoveryTiempoRestante));
    Serial.print(":");
    Serial.print(day(recoveryTiempoRestante));
    Serial.print(":");
    Serial.println(hour(recoveryTiempoRestante));
    Serial.println("------------------------");
    gettime();
    tiempoi = tiempoActual;
    tiempof = tiempoi + (tiempoFermentacion * 2629750);
    tiempoProcesoSeg = tiempof - tiempoi;
    tiempof = tiempoi + recoveryTiempoRestante;
  }else{
    Serial.println("------------------------");
    Serial.print("El proceso dura: ");
    Serial.print(month(tiempoFermentacion));
    Serial.print(":");
    Serial.print(day(tiempoFermentacion));
    Serial.println(":");
    Serial.print(hour(tiempoFermentacion));
    Serial.println("------------------------");
    gettime();
    tiempoi = tiempoActual;
    tiempof = tiempoi + (tiempoFermentacion * 2629750);
    tiempoProcesoSeg = tiempof - tiempoi;
  }

  long tiempoCancelacion = tiempoActual + 5;
  int tiempoPorcentaje = tiempoActual + 2;
  do{
    gettime();
    tiempoRestante = tiempof - tiempoActual;
    if (tiempoActual >= tiempoCancelacion){
      tiempoCancelacion = tiempoActual + 5;
      comprobarCancelar();
      sendInfo(procesoActual,pasoProceso);
      if (falloProceso){
        break;
      }
    }
    if (tiempoRestante <= 0) {break; porcentaje = 100;}
      if (tiempoActual >= tiempoPorcentaje){
        tiempoPorcentaje = tiempoActual + 2;
        int timepoIncremental = tiempoProcesoSeg - tiempoRestante;
        porcentaje = (timepoIncremental * 100) / tiempoProcesoSeg;
        Serial.print("Leeva el ");
        Serial.print(porcentaje);
        Serial.print("%");
        Serial.println(" completado");
      }
      delay(1000);
  }while(true);
  
//Envio mensaje de fin de proceso.
  if (falloProceso) {estado = 3; porcentaje = 100;}
  else {estado = 2; c_nokia_c(); porcentaje = 100;};
  recovery = 0;
  sendInfo(procesoActual,pasoProceso);
  finProceso(procesoActual,falloProceso);
}