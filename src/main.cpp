#include <Arduino.h>
#include <PSX.h>
#include <JVS.h>

#define BUFFER_SIZE 1024

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
  JVS::Packet __requestPacket;
  JVS::Packet __acknowledgePacket;

  int __status = MemoryCardStatus::Uninitialized;
};


void setup() {
  PSX::setup();
  JVS::setup();
}

void loop() {
  // put your main code here, to run repeatedly:
}
