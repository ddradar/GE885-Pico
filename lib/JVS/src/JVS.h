
#include <Arduino.h>

#pragma region
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

#define JVS_BAUD_RATE 115200
// Maximum data size (255 - sum)
#define JVS_MAX_DATA_SIZE 254
#define JVS_ADDRESS_BROADCAST 0xff

namespace JVS {
  /**
   * @brief JVS Packet
   */
  struct Packet {
    /**
     * @brief Address (`0xff`: Broadcast)
     */
    uint8_t nodeNo;
    /**
     * @brief Data length (`data` + `sum`)
     */
    uint8_t length = 0;
    /**
     * @brief Data (in Acknowledge packet, first byte is status)
     * @note Data is "Raw" and not escaped
     */
    uint8_t data[JVS_MAX_DATA_SIZE];
    /**
     * @brief Checksum (SUM of [nodeNo] + [length] + [data])
     */
    uint8_t sum;

    Packet(uint8_t nodeNo = 0x00);

    /**
     * @brief Add a byte to the packet data (and update `length` and `sum`)
     * @param data Byte to add
     */
    void add(uint8_t data);

    /**
     * @brief Add multiple bytes to the packet data (and update `length` and `sum`)
     * @param data Byte array to add
     * @param length Length of the byte array
     */
    void add(uint8_t *data, uint8_t length);

    /**
     * @brief Validate the packet by checking the checksum
     * @return `true` if the checksum is correct
     */
    bool validate();
  };

  /**
   * @brief Special characters on JVS packet
   */
  enum SpecialChar {
    /**
     * @brief SYNC code (Start of packet)
     */
    Sync = 0xe0,
    /**
     * @brief Marker code (Escape next byte, next byte is - 1)
     */
    Escape = 0xd0,
  };

  /**
   * @brief JVS Commands
   */
  enum Command {
    /**
     * @brief Reset all JVS devices. {`0xf0`, `0xd9`}
     * @return None
     */
    Reset = 0xf0,
    /**
     * @brief Set the JVS node address. {`0xf1`, <Node No.>}
     * @return `AckReport.OK`
     */
    SetAddress = 0xf1,
    /**
     * @brief Get the JVS device I/O ID. {`0x10`}
     * @return {`AckReport.OK`, <I/O ID strings (Ascii)>, `0x00`}
     */
    IOId = 0x10,
    /**
     * @brief Get the JVS device's command format version. {`0x11`}
     * @return {`AckReport.OK`, <Revision code (1 byte, BCD, lower 4 bits are decimal point)>}
     */
    CommandRev = 0x11,
    /**
     * @brief Get the JVS revision version. {`0x12`}
     * @return {`AckReport.OK`, <Revision code (1 byte, BCD, lower 4 bits are decimal point)>}
     */
    JvRev = 0x12,
    /**
     * @brief Get the JVS protocol version. {`0x13`}
     * @return {`AckReport.OK`, <Version code (1 byte, BCD, lower 4 bits are decimal point)>}
     */
    ProtocolVer = 0x13,
    /**
     * @brief Get the I/O function. {`0x14`}
     * @return {`AckReport.OK`, <Function codes (4 bytes each, Code + Parameter 3 bytes)>, `0x00`}
     */
    FunctionCheck = 0x14,
    /**
     * @brief Resend previous packet. {`0x2f`}
     * @return Previous packet
     */
    Retry = 0x2f,

    /**
     * @brief Control RAM buffer. (see `K573BufferRead`, `K573BufferWrite`, and `K573BufferSetAddress`)
     */
    K573Buffer = 0x70,
    /**
     * @brief Read data from RAM. {`0x70`, `0x00`, <Address (3 bytes)>, <Length>}
     * @return {`AckReport.OK`, <Data>}
     */
    K573BufferRead = 0x00,
    /**
     * @brief Write data to RAM. {`0x70`, `0x01`, <Address (3 bytes)>, <Length>, <Data>}
     * @return {`AckReport.OK`}
     */
    K573BufferWrite = 0x01,
    /**
     * @brief Set execution address. (use later on `K573Execute`) {`0x70`, `0x02`, <Address (3 bytes)>}
     * @return {`AckReport.OK`}
     */
    K573BufferSetAddress = 0x02,
    K573Status = 0x71,
    /**
     * @brief Control security plate. (see `K573SecurityPlateInsertCheck`, `K573SecurityPlateSetPassword`, `K573SecurityPlateGetData`, and `K573SecurityPlateConfigRegister`)
     */
    K573SecurityPlate = 0x72,
    /**
     * @brief Check if security plate is inserted? {`0x72`, <`0x00` xor Slot>}
     * @return {`AckReport.OK`}
     */
    K573SecurityPlateInsertCheck = 0x00,
    /**
     * @brief Set password to unlock security dongle? {`0x72`, <`0x10` xor Slot>, <Password (8 bytes)>}
     * @return {`AckReport.OK`}
     */
    K573SecurityPlateSetPassword = 0x10,
    /**
     * @brief Get dongle data and store in RAM. {`0x72`, <`0x20` xor Slot>, <Unknown (2 bytes)>, <RAM Address (3 bytes)>, <Unknown (2 bytes)>}
     * @return {`AckReport.OK`}
     */
    K573SecurityPlateGetData = 0x20,
    /**
     * @brief Get some kind of registration info from dongle? {`0x72`, <`0x40` xor Slot>, <RAM Address (3 bytes)>}
     * @return {`AckReport.OK`}
     */
    K573SecurityPlateConfigRegister = 0x40,
    /**
     * @brief Execute from previously set (`K573BufferSetAddress`) address. {`0x73`}
     * @return {`AckReport.OK`}
     */
    K573Execute = 0x73,
    /**
     * @brief Control PS1 Memory card. (see `K573MemoryCardRead`, and `K573MemoryCardWrite`)
     */
    K573MemoryCard = 0x76,
    /**
     * @brief Read from memory card and store in RAM. {`0x76`, `0x74`, <Port xor Address (2 bytes)>, <RAM Address (3 bytes)>, <Flame Count (2 bytes)>}
     * @return {`AckReport.OK`, `0x01`}
     */
    K573MemoryCardRead = 0x74,
    /**
     * @brief Write to memory card from RAM. {`0x76`, `0x75`, <RAM Address (3 bytes)>, <Port xor Address (2 bytes)>, <Flame Count (2 bytes)>}
     * @return {`AckReport.OK`, `0x01`}
     */
    K573MemoryCardWrite = 0x75,
    /**
     * @brief Read input from PS1 digital controller. {`0x77`}
     * @return {`AckReport.OK`, <Port:1 Inputs (2 bytes)>, <Port:2 Inputs (2 bytes)>}
     */
    K573Controller = 0x77,
  };

  /**
   * @brief Acknowledge status (in `data[0]`)
   */
  enum AckStatus {
    StatusOK = 0x01,
    CommandUnknown = 0x02,
    SumError = 0x03,
    Overflow = 0x04,
  };

  /**
   * @brief Acknowledge report for each command
   */
  enum AckReport {
    OK = 0x01,
    ParamErrorNoResult = 0x02,
    ParamErrorIgnored = 0x03,
    Busy = 0x04,
  };

  void setup();

  /**
   * @brief Try to get a packet from the JVS serial port. received packet is stored in `requestPacket`.
   * @param requestPacket Received packet
   * @return True if a complete packet was received
   * @note Until `isReceivedRequest` returns true, `requestPacket` is being under construction.
   */
  bool tryGetRequest(Packet &requestPacket);

  void reset();
  void setAddress(uint8_t nodeNo);

  void sendPacket(Packet &packet);
}
