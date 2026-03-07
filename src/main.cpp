#include <Arduino.h>
#include <PSX.h>
#include <JVS.h>
#include <Buffer.h>
#include "Debug.h"

#pragma region Pin Definitions
#ifndef JVS_SENSE_PIN
// JVS sense Pin (USB VBUS)
#define JVS_SENSE_PIN 7
#endif
#ifndef JVS_DATA_PLUS_PIN
// JVS DATA+ Pin
#define JVS_DATA_PLUS_PIN 5
#endif
#ifndef JVS_DATA_MINUS_PIN
// JVS DATA- Pin
#define JVS_DATA_MINUS_PIN 6
#endif
#ifndef JVS_TX_PIN
// TX Pin (Unused)
#define JVS_TX_PIN 4
#endif
#pragma endregion

enum MemoryCardStatus {
  // Default value, also used after writing?
  Uninitialized = 0x0000,
  Error = 0x0002,
  // Card is not inserted
  Unavailable = 0x0008,
  // Read request is executing
  Reading = 0x0200,
  // Write request is executing
  Writing = 0x0400,
  // Can be combined with MEMCARD_READING and MEMCARD_WRITING for busy state
  Available = 0x8000
};
static const char *__ioId = "KONAMI CO.,LTD.;White I/O;Ver1.0;White I/O PCB";

// private variables
namespace {
  // Acknowledge packet (Device -> Host)
  JVS::Packet __acknowledgePacket;
  JVS::Client __client = JVS::Client(Serial1, JVS_SENSE_PIN, JVS_DATA_MINUS_PIN, JVS_DATA_PLUS_PIN, JVS_TX_PIN);

  // Memory card status
  int __status = MemoryCardStatus::Uninitialized;
};


void setup() {
  D_SerialBegin(115200);
  D_println("GE885-Pico Ver. 1.0");
  PSX::setup();
}

void loop() {
  // put your main code here, to run repeatedly:
}
