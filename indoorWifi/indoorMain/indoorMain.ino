#include "jsonFS.h"
#include "indoorWiFi.h"
#include "server.h"
#include "utils.h"
#include "hardware.h"
#include "rules.h"

#define DISCONNECTED_STATE  0
#define CONFIG_STATE        1
#define CONNECTED_STATE     2
#define ERROR_STATE         3

#define GREEN_LED_PIN       16
#define CONFIG_BTN_PIN      0
#define I2C_SCL             5
#define I2C_SDA             4

int state = DISCONNECTED_STATE;
int rule_state = 0;

ButtonState buttonState(CONFIG_BTN_PIN);

void setup() {  
  Serial.begin(9600);
  while (! Serial) {
  }
  
  Serial.println("Inicializando...");

  // Inicializa sistema de archivos.
  if (! initFS()) {
    state = ERROR_STATE;
    return;
  }
  
  // Ciclo de config. de WiFi por comando
  int state_count = 100;
  while (state_count > 0 || state == CONFIG_STATE) { 
      
    int button = buttonState.buttonPressed();
    
    if (state == DISCONNECTED_STATE && button == HIGH) {
      state = CONFIG_STATE;
      buttonState.buttonReset();
      Serial.println("Modo configuracion");
    }
    
    if (state == CONFIG_STATE) {
      String command = readCommand();
      if (command.length() > 0) {
        if (command == "end") {
          Serial.println("Fin de configuracion.");
          state = DISCONNECTED_STATE;
        } else {
          int stat = executeCommand(command);
          command = "";
          if (stat != COMMAND_OK) {
            state = ERROR_STATE;
            Serial.println("Error en comando: "+String(stat));
            return;            
          } else {
            Serial.println("OK");          
          }
        }        
      }
    }
    
    if (state != CONFIG_STATE) {
      state_count--;
    }
    delay(100);
  }

  // Configuracion de hardware
  configHardware();

  registerHost();
  int stat = serverConfig();
  if (stat != SERVER_STARTED) {
    state = ERROR_STATE;
    return;
  }
  
  state = ERROR_STATE;
  buttonState.buttonReset();
}

void loop() {
  if (state == ERROR_STATE) {
    WiFiConfig conf = parseWiFiConfig();
    if (connectWiFi(conf) == CONNECT_OK) {
      state = CONNECTED_STATE;
    }
  } else {
    int button = buttonState.buttonPressed();
    readHardware();
    if (rule_state % 5 == 0) {
      evaluateRules();
    }
    rule_state = (rule_state + 1) % 5;
    if (wiFiStatus() == CONNECT_ERROR) {
      Serial.println("Red desconectada");
      disconnectWiFi();
      state = ERROR_STATE;      
    }
  }
  serverLoop();
  delay(1000);
}

