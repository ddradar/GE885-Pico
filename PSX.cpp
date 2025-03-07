/*
  This source code is based on the work of Shendo in the MemCARDuino project.

  MemCARDuino - Arduino PlayStation 1 Memory Card reader
  Shendo 2013. - 2024.
  Source: https://github.com/ShendoXT/memcarduino/blob/f506860d0118bcef710503f4069833251da5d685/MemCARDuino.ino
  License: https://github.com/ShendoXT/memcarduino/blob/f506860d0118bcef710503f4069833251da5d685/LICENSE
*/
#include <SPI.h>
#include "PSX.h"

namespace PSX {
  volatile int __state = HIGH;
  bool __isCompatibleMode = false;

  /**
   * @brief Acknowledge the PSX device
   * @note This function is called when the PSX device sends an ACK signal
   */
  void __acknowledge() {
    __state = !__state;
  }

  /**
   * @brief Activate or deactivate slot
   * @param slot Slot number (0: Port 1, 1: Port 2)
   * @param status LOW to activate, HIGH to deactivate
   */
  void __attention(int slot, int status) {
    if (slot == 0) {
      digitalWrite(PSX_ATTENTION_PIN_1, status);
    } else {
      digitalWrite(PSX_ATTENTION_PIN_2, status);
    }
  }

  /**
   * @brief Send a command to the PSX port
   * @param command Command to send
   * @param timeOut ACK signal timeout in microseconds
   * @param delay Delay in microseconds before sending the command
   * @return Response from the PSX device
   * @note Before sending a command, the attention signal must be set to LOW
   */
  uint8_t __sendCommand(uint8_t command, int timeOut, int delay) {
    if (!__isCompatibleMode) timeOut = 3000;
    __state = HIGH;

    if (delay > 0) delayMicroseconds(delay);

    uint8_t res = SPI.transfer(command);

    // Wait for the ACK signal from the Memory Card
    while (__state == HIGH) {
      timeOut--;
      delayMicroseconds(1);
      if (timeOut == 0) {
        // Timeout reached, card doesn't report ACK properly
        __isCompatibleMode = true;
        break;
      }
    }

    return res;
  }
}

void PSX::setup() {
  pinMode(PSX_ATTENTION_PIN_1, OUTPUT);
  pinMode(PSX_ATTENTION_PIN_2, OUTPUT);
  pinMode(PSX_ACKNOWLEDGE_PIN, INPUT_PULLUP);
  digitalWrite(PSX_ATTENTION_PIN_1, HIGH);
  digitalWrite(PSX_ATTENTION_PIN_2, HIGH);

#ifdef ARDUINO_ARCH_RP2040
  SPI.setMISO(PSX_DATA_PIN);
  SPI.setMOSI(PSX_COMMAND_PIN);
  SPI.setSCK(PSX_CLOCK_PIN);
#endif
  SPI.begin();
  SPI.beginTransaction(SPISettings(125000, LSBFIRST, SPI_MODE3));
  pinMode(MISO, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PSX_ACKNOWLEDGE_PIN), __acknowledge, RISING);
// Pico needs to be reminded of the ack pin configuration...
#ifdef ARDUINO_ARCH_RP2040
  pinMode(PSX_ACKNOWLEDGE_PIN, INPUT_PULLUP);
#endif
}

bool PSX::tryReadFromMemoryCard(int slot, int address, uint8_t *output) {
  // Use ACK detection mode by default
  __isCompatibleMode = false;

  // Activate device
  __attention(slot, LOW);
  delayMicroseconds(PSX_TRANSFER_WAIT);

  __sendCommand(Device::MemoryCard, 500, 70);             // Access Memory Card
  if (__sendCommand('R', 500, 45) == 0xff) return false;  // Send read command
  uint8_t id = __sendCommand(0x00, 500, 45);              // Receive Memory Card ID1
  __sendCommand(0x00, 500, 45);                           // Receive Memory Card ID2 (ignore)
  if (output == nullptr) {
    // Only check if the memory card is inserted
    __attention(slot, HIGH);
    return id == 0x5a;
  }
  __sendCommand(highByte(address), 500, 45);                // Address MSB
  __sendCommand(lowByte(address), 500, 45);                 // Address LSB
  if (__sendCommand(0x00, 2800, 45) != 0x5c) return false;  // Receive Memory Card ACK1
  __sendCommand(0x00, 2800, 0);                             // Receive Memory Card ACK2 (ignore)

  uint8_t checksum = __sendCommand(0x00, 2800, 0);  // Receive Confirmed MSB
  checksum ^= __sendCommand(0x00, 2800, 0);         // Receive Confirmed LSB
  // Get 128 bytes data from the frame
  for (int i = 0; i < PSX_MEMCARD_FRAME_SIZE; i++) {
    uint8_t b = __sendCommand(0x00, 150, 0);
    *output++ = b;
    checksum ^= b;
  }
  uint8_t receivedSum = __sendCommand(0x00, 500, 0);  // Checksum (MSB xor LSB xor Data)
  uint8_t status = __sendCommand(0x00, 500, 0);       // Memory Card status byte

  //Deactivate device
  __attention(slot, HIGH);

  return status == 'G' && checksum == receivedSum;  // 'N': Bad checksum, 0xFF: Bad sector
}

bool PSX::tryWriteToMemoryCard(int slot, int address, uint8_t *input) {
  // Use ACK detection mode by default
  __isCompatibleMode = false;

  // Activate device
  __attention(slot, LOW);
  delayMicroseconds(PSX_TRANSFER_WAIT);

  __sendCommand(Device::MemoryCard, 300, 45);             // Access Memory Card
  if (__sendCommand('W', 300, 45) == 0xff) return false;  // Send write command
  __sendCommand(0x00, 300, 45);                           // Memory Card ID1 (ignore)
  __sendCommand(0x00, 300, 45);                           // Memory Card ID2 (ignore)
  __sendCommand(highByte(address), 300, 45);              // Address MSB
  __sendCommand(lowByte(address), 300, 45);               // Address LSB

  uint8_t checksum = highByte(address) ^ lowByte(address);
  // Write 128 bytes data to the frame
  for (int i = 0; i < PSX_MEMCARD_FRAME_SIZE; i++) {
    __sendCommand(input[i], 150, 0);
    checksum ^= input[i];
  }
  __sendCommand(checksum, 200, 0);             // Checksum (MSB xor LSB xor Data)
  __sendCommand(0x00, 200, 0);                 // Memory Card ACK1 (ignore)
  __sendCommand(0x00, 200, 0);                 // Memory Card ACK2 (ignore)
  uint8_t status = __sendCommand(0x00, 0, 0);  // Memory Card status byte

  // Deactivate device
  __attention(slot, HIGH);

  return status == 'G';  // 'N': Bad checksum, 0xFF: Bad sector
}

bool PSX::tryReadControllerInput(int slot, uint8_t *output) {
  // Use ACK detection mode by default
  __isCompatibleMode = false;

  // Activate device
  __attention(slot, LOW);
  delayMicroseconds(PSX_TRANSFER_WAIT);

  __sendCommand(Device::Controller, 300, 45);   // Access Controller
  uint8_t lo = __sendCommand('B', 300, 45);     // Read Command (Returns Controller ID LSB)
  uint8_t hi = __sendCommand(0x00, 300, 45);    // Returns Controller ID MSB
  bool connected = (lo == 0x41 && hi == 0x5a);  // 5a41: Digital Controller

  if (connected) {
    *output++ = __sendCommand(0x00, 200, 0);  // Returns Digital switches LSB
    *output++ = __sendCommand(0x00, 200, 0);  // Returns Digital switches MSB
  } else {
    *output++ = 0x00;
    *output++ = 0x00;
  }

  //Deactivate device
  __attention(slot, HIGH);

  return connected;
}
