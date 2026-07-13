#include "driver/twai.h"

#define LED1 2
#define LED2 4

#define MY_CAN_ID 0x300
#define ECU_NAME "ICM"

void setup() {
  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(
          GPIO_NUM_21,
          GPIO_NUM_22,
          TWAI_MODE_NORMAL);

  twai_timing_config_t t_config =
      TWAI_TIMING_CONFIG_500KBITS();

  twai_filter_config_t f_config =
      TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config,
                          &t_config,
                          &f_config) == ESP_OK) {
    Serial.println("TWAI Driver Installed");
  } else {
    Serial.println("Driver Installation Failed");
    while (1);
  }

  if (twai_start() == ESP_OK) {
    Serial.print(ECU_NAME);
    Serial.println(" Ready");
  } else {
    Serial.println("CAN Start Failed");
    while (1);
  }
}

void loop() {

  twai_message_t msg;

  if (twai_receive(&msg,
                   pdMS_TO_TICKS(100))
      == ESP_OK) {

    // Print every received frame
    Serial.println("\n===== CAN Message Received =====");

    Serial.print("ID: 0x");
    Serial.println(msg.identifier, HEX);

    Serial.print("DLC: ");
    Serial.println(msg.data_length_code);

    Serial.print("Data: ");
    for (int i = 0; i < msg.data_length_code; i++) {
      Serial.print(msg.data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Ignore messages not for this ECU
    if (msg.identifier != MY_CAN_ID) {
      Serial.println("Not for this ECU");
      return;
    }

    if (msg.data_length_code < 2) {
      Serial.println("Invalid DLC");
      return;
    }

    int component = msg.data[0];
    int state = msg.data[1];

    Serial.print("Component: ");
    Serial.println(component);

    Serial.print("State: ");
    Serial.println(state);

    switch (component) {

      case 1:
        digitalWrite(LED1, state ? HIGH : LOW);

        Serial.print("Radio -> ");
        Serial.println(state ? "ON" : "OFF");
        break;

      case 2:
        digitalWrite(LED2, state ? HIGH : LOW);

        Serial.print("Bluetooth -> ");
        Serial.println(state ? "ON" : "OFF");
        break;

      default:
        Serial.print("Invalid Component: ");
        Serial.println(component);
        break;
    }
  }
}
