/**
 * @file Lector_UHF_EPC_Final_LED.ino
 * @author MGTI. Saul Isai Soto Ortiz
 * @brief Lectura limpia de código EPC (YRM1004) con indicador LED
 * 
 * REQUISITO FÍSICO: Pin EN del YRM1004 firmemente conectado a 3.3V o 5V.
 * CONEXIÓN: TX del lector a GPIO 14 | RX del lector a GPIO 13.
 */

#include <Arduino.h>

// --- MAPEO DE PINES ---
#define RFID_RX_PIN 14 // Conectar al TX del YRM1004
#define RFID_TX_PIN 13 // Conectar al RX del YRM1004
#define LED_PIN     12 // Pin del LED azul integrado en la LilyGO T-SIM7070G
#define BAUD_RFID   115200 

HardwareSerial LectorRFID(2);

// Comando 0x22: Single Polling (Buscar una etiqueta en el ambiente)
byte comandoLeerTag[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};

void setup() {
  Serial.begin(115200);
  
  // Configurar el LED integrado como salida y asegurar que inicie apagado
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Iniciar comunicación con el módulo RFID
  LectorRFID.begin(BAUD_RFID, SERIAL_8N1, RFID_RX_PIN, RFID_TX_PIN);

  Serial.println("==================================================");
  Serial.println("   Laboratorio IoT - Lector YRM1004 Activo");
  Serial.println("   Listo para extraer códigos EPC...");
  Serial.println("==================================================");
  delay(1000);
}

void loop() {
  // 1. Limpiar buffer de lecturas anteriores o ruido eléctrico
  while (LectorRFID.available()) {
    LectorRFID.read();
  }

  // 2. Disparar la orden de lectura de etiqueta
  LectorRFID.write(comandoLeerTag, sizeof(comandoLeerTag));
  
  // 3. Dar tiempo al módulo para emitir radiofrecuencia y procesar
  delay(150); 

  // 4. Analizar la respuesta si el módulo envió datos
  if (LectorRFID.available()) {
    byte cabecera = LectorRFID.read();
    
    // Verificamos si la trama es válida (empieza con 0xBB)
    if (cabecera == 0xBB) {
      byte tipo = LectorRFID.read();
      byte comando = LectorRFID.read();
      
      // Si responde 0x01, no hay etiquetas cerca
      if (tipo == 0x01 && comando == 0x22) {
         // Silencio en el Monitor Serie, el LED permanece apagado.
      } 
      // Si responde 0x02, ¡encontró una etiqueta!
      else if (tipo == 0x02 && comando == 0x22) {
         
         // -> ENCENDER EL LED INDICADOR DE LECTURA EXITOSA
         digitalWrite(LED_PIN, HIGH);
         
         // Leer los 5 bytes iniciales de datos (Longitud, RSSI y PC)
         byte lenMSB = LectorRFID.read(); 
         byte lenLSB = LectorRFID.read(); 
         byte rssi = LectorRFID.read();   
         byte pc1 = LectorRFID.read();    
         byte pc2 = LectorRFID.read();    

         String epcLimpio = "";
         
         // Extraer exactamente los 12 bytes del código EPC
         for(int i = 0; i < 12; i++) {
            byte epcByte = LectorRFID.read();
            if (epcByte < 0x10) epcLimpio += "0"; // Agregar cero a la izquierda si es menor a 16
            epcLimpio += String(epcByte, HEX);
         }
         epcLimpio.toUpperCase(); // Formatear a mayúsculas

         // Limpiar la basura restante en el buffer (Checksum y Fin de trama)
         while(LectorRFID.available()) {
            LectorRFID.read();
         }

         // Imprimir los datos limpios y empaquetados
         Serial.println("=========================================");
         Serial.println(">>> ETIQUETA CAPTURADA EXITOSAMENTE <<<");
         Serial.println("EPC: " + epcLimpio);
         Serial.println("Intensidad (RSSI): -" + String(256 - rssi) + " dBm");
         Serial.println("=========================================\n");
         
         // Retardo breve de 300 milisegundos para que el destello del LED sea visible a simple vista
         delay(300);
         
         // -> APAGAR EL LED
         digitalWrite(LED_PIN, LOW);
      }
    }
  }

  // Pausa entre escaneos (1 segundo) para evitar colisiones
  delay(1000); 
}
