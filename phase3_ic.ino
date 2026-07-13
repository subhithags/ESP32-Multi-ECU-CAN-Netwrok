#include "driver/twai.h"

#define LED1 2
#define LED2 4

#define MY_CAN_ID 0x400
#define ECU_NAME "IC"

void setup() {
  Serial.begin(115200);

  // LED setup
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  // TWAI (CAN) configuration
  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(
          GPIO_NUM_21,
          GPIO_NUM_22,
          TWAI_MODE_NORMAL);

  twai_timing_config_t t_config =
      TWAI_TIMING_CONFIG_500KBITS();

  // Accept all messages for now
  // We will implement hardware filtering later
  twai_filter_config_t f_config =
      TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install driver
  if (twai_driver_install(&g_config,
                          &t_config,
                          &f_config) == ESP_OK) {
    Serial.println("TWAI Driver Installed");
  }
  else {
    Serial.println("Driver Installation Failed");
    return;
  }

  // Start CAN
  if (twai_start() == ESP_OK) {
    Serial.print(ECU_NAME);
    Serial.println(" Ready");
  }
  else {
    Serial.println("CAN Start Failed");
  }
}

void loop() {

  twai_message_t msg;

  // Wait for a CAN message
  if (twai_receive(&msg,
                   pdMS_TO_TICKS(100))
      == ESP_OK) {

    // Software filtering
    if (msg.identifier != MY_CAN_ID)
      return;

    // Ensure message contains 2 bytes
    if (msg.data_length_code < 2)
      return;

    int component = msg.data[0];
    int state = msg.data[1];

    // Component control
    switch (component) {

      case 1:
        digitalWrite(LED1, state ? HIGH : LOW);

        Serial.print("Battery Warning -> ");
        Serial.println(state ? "ON" : "OFF");
        break;

      case 2:
        digitalWrite(LED2, state ? HIGH : LOW);

        Serial.print("Check Engine -> ");
        Serial.println(state ? "ON" : "OFF");
        break;

      default:
        Serial.print("Invalid Component: ");
        Serial.println(component);
        break;
    }
  }
}
