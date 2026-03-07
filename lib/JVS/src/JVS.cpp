#include "JVS.h"

namespace JVS {
#pragma region Packet
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
    assert(this->length < JVS_MAX_DATA_SIZE);
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
    assert(this->length + length <= JVS_MAX_DATA_SIZE);
    memcpy(this->data + this->length, data, length);
    this->length += length;
    for (int i = 0; i < length; i++) {
      this->sum += data[i];
    }
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
#pragma endregion Packet

#pragma region Client
  /**
   * @brief Create a JVS Client.
   * @param serial Serial port for JVS communication
   * @param sensePin JVS Sense pin (USB VBUS pin 1)
   * @param dataMinusPin JVS DATA- pin (USB pin 2)
   * @param dataPlusPin JVS DATA+ pin (USB pin 3)
   * @param txPin TX pin (Unused)
   */
  Client::Client(SerialUART &serial, pin_size_t sensePin, pin_size_t dataMinusPin, pin_size_t dataPlusPin, pin_size_t txPin) {
    this->__serial = &serial;
    this->__sensePin = sensePin;
    this->__dataMinusPin = dataMinusPin;

    this->__serial->setTX(txPin);
    this->__serial->setRX(dataPlusPin);

    pinMode(sensePin, INPUT);
    pinMode(dataMinusPin, OUTPUT);
    digitalWrite(sensePin, LOW);  // pullup to 5V (terminated)
    digitalWrite(dataMinusPin, LOW);

    this->__serial->begin(JVS_BAUD_RATE);
    this->__isInitialized = true;
  }

  /**
   * @brief Try to get a packet from the JVS serial port. received packet can be accessed via `getRequestPacket()`.
   * @return True if a complete packet was received
   * @note Until `tryGetRequest` returns true, `getRequestPacket()` returns an under construction packet.
   */
  bool Client::tryGetRequest() {
    if (!this->__isInitialized) return false;

    // At least 5 bytes are needed ([SYNC], [Node No.], [Byte Count], [Data], [SUM])
    int available = this->__serial->available();
    if (this->__phase == ReceiveStatus::WaitSync && available < 5) return false;

    for (int i = 0; i < available; i++) {
      uint8_t b = this->__serial->read();
      if (b == SpecialChar::Sync) {
        // Start of packet
        this->__phase = ReceiveStatus::ReceivedSync;
        this->__escaped = false;
        continue;
      }
      // Wait until [SYNC] is received
      if (this->__phase == ReceiveStatus::WaitSync) continue;

      // Escape & Unescape
      if (b == SpecialChar::Escape) {
        this->__escaped = true;
        continue;
      }
      if (this->__escaped) {
        b++;
        this->__escaped = false;
      }

      switch (this->__phase) {
        case ReceiveStatus::ReceivedSync:  // b is [Node No.]
          if (b != JVS_ADDRESS_BROADCAST && b != __nodeNo) {
            // Ignore packets not addressed to us
            this->__phase = ReceiveStatus::WaitSync;
            return false;
          }
          this->__requestPacket = Packet(b);
          this->__phase = ReceiveStatus::ReceivedMyNodeNo;
          break;
        case ReceiveStatus::ReceivedMyNodeNo:  // b is [Byte Count]
          this->__requestPacket.length = b;
          this->__dataCount = 0;
          this->__phase = ReceiveStatus::ProcessingData;
          break;
        case ReceiveStatus::ProcessingData:  // b is [Data]
          this->__requestPacket.data[this->__dataCount++] = b;
          if (this->__requestPacket.length == this->__dataCount) this->__phase = ReceiveStatus::ProcessedData;
          break;
        case ReceiveStatus::ProcessedData:  // b is [SUM]
          this->__requestPacket.sum = b;
          this->__phase = ReceiveStatus::WaitSync;
          return this->__requestPacket.validate();
      }
    }
    // Reached end of buffer, continue on next loop
    return false;
  }

  /**
   * @brief Get the request packet (after `tryGetRequest` returns true)
   * @return Reference to the request packet
   * @note Until `tryGetRequest` returns true, `getRequestPacket()` returns an under construction packet.
   */
  Packet &Client::getRequestPacket() {
    return this->__requestPacket;
  }

  /**
   * @brief Set this Client's JVS Node No.
   * @param nodeNo JVS Node No (0x00: Uninitialized, 0x01-0x1f: Valid Node No, 0xff: Broadcast)
   * @note If you set `nodeNo` to `0x00`, it will be reset.
   */
  void Client::setAddress(uint8_t nodeNo) {
    this->__nodeNo = nodeNo;
    if (nodeNo == 0) {
      // Reset to uninitialized
      pinMode(this->__sensePin, INPUT);
      this->__isInitialized = false;
      return;
    }
    pinMode(this->__sensePin, OUTPUT);  // Set to 0V
    this->__isInitialized = true;
  }

  /**
   * @brief Send an acknowledgement packet to host.
   * @param packet Packet to send
   */
  void Client::sendPacket(Packet &packet) {
    digitalWrite(this->__dataMinusPin, HIGH);  // RS485_TX

    this->__serial->write(SpecialChar::Sync);
    this->__serial->write(packet.nodeNo);  // [Node No.] never to be Sync or Escape (0-31, 0xff)
    this->__writeWithEscape(packet.length);
    for (int i = 0; i < packet.length; i++) {
      this->__writeWithEscape(packet.data[i]);
    }
    this->__writeWithEscape(packet.sum);
    this->__serial->flush();  // Wait for all data to be sent

    digitalWrite(this->__dataMinusPin, LOW);  // RS485_RX
  }

  /**
   * @brief Write a byte to the serial port with escape handling
   * @param data Byte to write
   * @note If the byte is `Sync` or `Escape`, it will be escaped by writing `Escape` first, then the byte minus 1.
   * @note This function is used in `sendPacket` to write the packet data.
   */
  void Client::__writeWithEscape(uint8_t data) {
    if (data == SpecialChar::Sync || data == SpecialChar::Escape) {
      this->__serial->write(SpecialChar::Escape);
      data--;
    }
    this->__serial->write(data);
  }
#pragma endregion Client
};
