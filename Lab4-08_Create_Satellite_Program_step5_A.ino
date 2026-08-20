#include "src/HeptaSat.h"

HeptaCdh    cdh;
HeptaEps    eps;
HeptaSensor sensor;

// Bus-voltage thresholds for the 3V3 load switch (V4.1.1 get_bus_voltage).
// Hysteresis avoids chatter near the boundary; validate on hardware if the
// SAT_POWER / bus rail behaviour differs from the old VBAT curve.
const float BUS_VOLTAGE_TURN_OFF = 3.5;
const float BUS_VOLTAGE_TURN_ON  = 3.7;

const float temperature = 25.0; // Temperature in degrees Celsius
bool sw3V3_is_on = true;

void setup() {
  cdh.begin();
  eps.init();
  cdh.wait_for_serial();
  cdh.wait_for_sd();
}

void loop() {
  float bus_voltage = eps.get_bus_voltage();
  cdh.println("------------------------------");
  cdh.printf("Satellite Time: %.2f seconds\n", millis() / 1000.0); // Print time in seconds
  cdh.printf("Bus voltage: %.2f V\n", bus_voltage);
  cdh.printf("Temperature: %.2f °C\n", temperature);
  cdh.println("------------------------------");

  if (sw3V3_is_on && bus_voltage < BUS_VOLTAGE_TURN_OFF) {
    cdh.println("Bus voltage is low! Switching off 3.3V SW to save power.");
    eps.switch_3V3_off();
    sw3V3_is_on = false;
  } else if (!sw3V3_is_on && bus_voltage > BUS_VOLTAGE_TURN_ON) {
    cdh.println("Bus voltage recovered. Switching on 3.3V SW.");
    eps.switch_3V3_on();
    sw3V3_is_on = true;
  } else {
    cdh.printf("3.3V SW is %s.\n", sw3V3_is_on ? "on" : "off");
  }

  if (cdh.is_cmd_received()) {
    char cmd = cdh.get_command();

    if (cmd != '\0') {
      cdh.printf("Received command: %c\n", cmd);

      // Process the command and respond accordingly
      switch (cmd) {
        case 'a': {
          for(uint8_t i = 0; i < 10; i++) {
            cdh.println("Hello HEPTA-SAT");
            delay(1000);
          }
          break;
        }

        case 'b': {
          cdh.println("Saving voltages to the SD card...");
          File file = cdh.create_file("test.txt");
          if (file) {
            for(uint8_t i = 0; i < 10; i++) {
              float bus_voltage = eps.get_bus_voltage();
              cdh.printf_file(file, "Bus voltage: %f V\r\n", bus_voltage);
              delay(1000);
            }
            file.close();
          } else {
            cdh.println("Failed to create file on SD card.");
            break;
          }

          cdh.println("Done saving voltages to the SD card.");
          cdh.println("Reading voltages from the SD card...");

          file = cdh.open_file("test.txt");
          while (file && file.available()) {
            cdh.write(cdh.read_file(file));
          }
          file.close();

          cdh.println("Done reading voltages from the SD card.");
          break;
        }

        case 'c': {
          cdh.println("Reading accelerometer data...");
          float ax, ay, az;
          for (uint8_t i = 0; i < 10; i++) {
            sensor.get_acceleration(&ax, &ay, &az);
            cdh.printf("Acceleration: ax=%.2f, ay=%.2f, az=%.2f m/s^2\n", ax, ay, az);
            delay(1000);
          }
          cdh.println("Done reading accelerometer data.");
          break;
        }

        case 'd': {
          cdh.println("Reading gyroscope data...");
          float gx, gy, gz;
          for (uint8_t i = 0; i < 10; i++) {
            sensor.get_gyro(&gx, &gy, &gz);
            cdh.printf("Gyroscope: gx=%.2f, gy=%.2f, gz=%.2f °/s\n", gx, gy, gz);
            delay(1000);
          }
          cdh.println("Done reading gyroscope data.");
          break;
        }

        default:
          cdh.println("Unknown command");
          break;
      }
    }
  }
  delay(1000);
}
