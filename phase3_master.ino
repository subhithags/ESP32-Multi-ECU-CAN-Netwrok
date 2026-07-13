#include "driver/twai.h"

void setup() {
  Serial.begin(115200);

  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(
          GPIO_NUM_21,
          GPIO_NUM_22,
          TWAI_MODE_NORMAL);

  twai_timing_config_t t_config =
      TWAI_TIMING_CONFIG_500KBITS();

  twai_filter_config_t f_config =
      TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(
          &g_config,
          &t_config,
          &f_config) == ESP_OK) {
    Serial.println("Driver Installed");
  } else {
    Serial.println("Driver Install Failed");
    while (1);
  }

  if (twai_start() == ESP_OK) {
    Serial.println("CAN Started");
  } else {
    Serial.println("CAN Start Failed");
    while (1);
  }

  Serial.println();
  Serial.println("BCM Ready");
  Serial.println("Format:");
  Serial.println("Subsystem Component State");
  Serial.println("Example: 2 1 1");
}

void loop() {

  if (Serial.available()) {

    // Read the entire line
    String input = Serial.readStringUntil('\n');
    input.trim();

    // Ignore empty lines
    if (input.length() == 0)
      return;

    int subsystem;
    int component;
    int state;

    // Parse the command
    if (sscanf(input.c_str(),
               "%d %d %d",
               &subsystem,
               &component,
               &state) != 3) {
      Serial.println("Invalid Format");
      Serial.println("Use: Subsystem Component State");
      return;
    }

    // Validate component
    if (component < 1 || component > 2) {
      Serial.println("Invalid Component");
      return;
    }

    // Validate state
    if (state != 0 && state != 1) {
      Serial.println("Invalid State");
      return;
    }

    uint32_t canID;

    switch (subsystem) {

      case 2:
        canID = 0x200;   // RCM
        break;

      case 3:
        canID = 0x300;   // ICM
        break;

      case 4:
        canID = 0x400;   // IC
        break;

      default:
        Serial.println("Invalid Subsystem");
        return;
    }

    twai_message_t msg;

    msg.identifier = canID;
    msg.extd = 0;
    msg.rtr = 0;
    msg.data_length_code = 2;

    msg.data[0] = component;
    msg.data[1] = state;

    if (twai_transmit(
            &msg,
            pdMS_TO_TICKS(1000))
        == ESP_OK) {

      Serial.print("Sent -> ");
      Serial.print(subsystem);
      Serial.print(" ");
      Serial.print(component);
      Serial.print(" ");
      Serial.println(state);
    }
    else {

      Serial.println("Transmission Failed");

      twai_status_info_t status;
      twai_get_status_info(&status);

      Serial.println("------ TWAI STATUS ------");

      Serial.print("State: ");
      Serial.println(status.state);

      Serial.print("TX Error Counter: ");
      Serial.println(status.tx_error_counter);

      Serial.print("RX Error Counter: ");
      Serial.println(status.rx_error_counter);

      Serial.print("Bus Error Count: ");
      Serial.println(status.bus_error_count);

      Serial.print("Messages To TX: ");
      Serial.println(status.msgs_to_tx);

      Serial.print("Messages To RX: ");
      Serial.println(status.msgs_to_rx);
    }
  }
}
