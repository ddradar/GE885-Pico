#include "JVS.h"
#include <HardwareSerial.h>

namespace JVS {
  // private
  namespace {
    /**
     * @brief Internal received status (used in `isReceivedRequest`)
     */
    enum ReceiveStatus {
      WaitSync = 0,
      ReceivedSync = 1,
      ReceivedMyNodeNo = 2,
      ProcessingData = 3,
      ProcessedData = 4,
    };

    /**
     * @brief Serial port for JVS communication
     */
    SerialUART *__serial;

    bool __isInitialized = false;
    uint8_t __nodeNo = 0;

    void __writeWithEscape(uint8_t data) {
      if (data == SpecialChar::Sync || data == SpecialChar::Escape) {
        __serial->write(SpecialChar::Escape);
        data--;
      }
      __serial->write(data);
    }
  }

  Packet::Packet(uint8_t nodeNo) {
    this->nodeNo = nodeNo;
    this->length = 0;
    memset(this->data, 0, JVS_MAX_DATA_SIZE);
    this->sum = nodeNo;
  }

  /**
   * @brief Add a byte to the packet data (and update `length` and `sum`)
   * @param data Byte to add
   */
  void Packet::add(uint8_t data) {
    this->data[this->length] = data;
    this->length++;
    this->sum += data;
  }

  /**
   * @brief Add multiple bytes to the packet data (and update `length` and `sum`)
   * @param data Byte array to add
   * @param length Length of the byte array
   */
  void Packet::add(uint8_t *data, uint8_t length) {
    memcpy(this->data + this->length, data, length);
    this->length += length;
  }

  /**
   * @brief Validate the packet by checking the checksum
   * @return `true` if the checksum is correct
   */
  bool Packet::validate() {
    uint8_t sum = this->nodeNo;
    for (int i = 0; i < this->length; i++) {
      sum += this->data[i];
    }
    return (sum == this->sum);
  }

  void setup() {
    __serial = &Serial1;
    __serial->setTX(JVS_TX_PIN);
    __serial->setRX(JVS_DATA_PLUS_PIN);

    pinMode(JVS_SENSE_PIN, INPUT);
    pinMode(JVS_DATA_MINUS_PIN, OUTPUT);
    digitalWrite(JVS_SENSE_PIN, LOW);  // pullup to 5V (terminated)
    digitalWrite(JVS_DATA_MINUS_PIN, LOW);

    __serial->begin(JVS_BAUD_RATE);
    __isInitialized = true;
  }

  bool tryGetRequest(Packet &requestPacket) {
    // Received escape character or not
    static bool __escaped;
    // Receive status
    static int __phase;
    // Processed packet.data count
    static uint8_t __dataCount;

    if (!__isInitialized) return false;

    // At least 5 bytes are needed ([SYNC], [Node No.], [Byte Count], [Data], [SUM])
    int available = __serial->available();
    if (__phase == ReceiveStatus::WaitSync && available < 5) return false;

    for (int i = 0; i < available; i++) {
      uint8_t b = __serial->read();
      if (b == SpecialChar::Sync) {
        // Start of packet
        __phase = ReceiveStatus::ReceivedSync;
        __escaped = false;
        continue;
      }
      // Wait until [SYNC] is received
      if (__phase == ReceiveStatus::WaitSync) continue;

      // Escape & Unescape
      if (b == SpecialChar::Escape) {
        __escaped = true;
        continue;
      }
      if (__escaped) {
        b++;
        __escaped = false;
      }

      switch (__phase) {
        case ReceiveStatus::ReceivedSync:  // b is [Node No.]
          if (b != JVS_ADDRESS_BROADCAST && b != __nodeNo) {
            // Ignore packets not addressed to us
            __phase = ReceiveStatus::WaitSync;
            return false;
          }
          requestPacket = Packet(b);
          __phase = ReceiveStatus::ReceivedMyNodeNo;
          break;
        case ReceiveStatus::ReceivedMyNodeNo:  // b is [Byte Count]
          requestPacket.length = b;
          __dataCount = 0;
          __phase = ReceiveStatus::ProcessingData;
          break;
        case ReceiveStatus::ProcessingData:  // b is [Data]
          requestPacket.data[__dataCount] = b;
          if (requestPacket.length == __dataCount) __phase = ReceiveStatus::ProcessedData;
          __dataCount++;
          break;
        case ReceiveStatus::ProcessedData:  // b is [SUM]
          requestPacket.sum = b;
          __phase = ReceiveStatus::WaitSync;
          return requestPacket.validate();
      }
    }
    // Reached end of buffer, continue on next loop
    return false;
  }

  void reset() {
    __nodeNo = 0;
    pinMode(JVS_SENSE_PIN, INPUT);
    __isInitialized = false;
  }

  void setAddress(uint8_t nodeNo) {
    __nodeNo = nodeNo;
    pinMode(JVS_SENSE_PIN, OUTPUT);  // Set to 0V
    __isInitialized = true;
  }

  void sendPacket(Packet &packet) {
    digitalWrite(JVS_DATA_MINUS_PIN, HIGH);  // RS485_TX

    __serial->write(SpecialChar::Sync);
    __serial->write(packet.nodeNo);  // [Node No.] never to be Sync or Escape (0-31, 0xff)
    __writeWithEscape(packet.length);
    for (int i = 0; i < packet.length; i++) {
      __writeWithEscape(packet.data[i]);
    }
    __writeWithEscape(packet.sum);
    __serial->flush();  // Wait for all data to be sent

    digitalWrite(JVS_DATA_MINUS_PIN, LOW);  // RS485_RX
  }
};
