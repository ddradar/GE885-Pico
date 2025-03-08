#pragma region
#ifndef PSX_DATA_PIN
// PSX Data(MISO) Pin
#define PSX_DATA_PIN 0
#endif
#ifndef PSX_COMMAND_PIN
// PSX Command(MOSI) Pin
#define PSX_COMMAND_PIN 3
#endif
#ifndef PSX_ATTENTION_PIN_1
// PSX Attention Pin for Port 1
#define PSX_ATTENTION_PIN_1 14
#endif
#ifndef PSX_ATTENTION_PIN_2
// PSX Attention Pin for Port 2
#define PSX_ATTENTION_PIN_2 15
#endif
#ifndef PSX_CLOCK_PIN
// PSX Clock(SCK) Pin
#define PSX_CLOCK_PIN 2
#endif
#ifndef PSX_ACKNOWLEDGE_PIN
// PSX Acknowledge Pin
#define PSX_ACKNOWLEDGE_PIN 8
#endif
#pragma endregion

// Memory card block count (First block is used as Directory)
#define PSX_MEMCARD_BLOCK_COUNTS 16
// Memory card frames in a block (1 Block = 64 frames)
#define PSX_MEMCARD_FRAMES_IN_BLOCK 64
// Memory card frame size (128 bytes)
#define PSX_MEMCARD_FRAME_SIZE 128
#define PSX_TRANSFER_WAIT 20

namespace PSX {
  /**
   * @brief PSX device IDs
   */
  enum Device {
    /**
     * @brief Standard controller
     */
    Controller = 0x01,
    // PS2Multitap = 0x21,
    // PS2DVDRemote = 0x61,
    /**
     * @brief Memory card
     */
    MemoryCard = 0x81,
  };

  /**
   * @brief Setup the pins for the PSX port.
   */
  void setup();

  /**
   * @brief Read the input from PS1 digital controller.
   * @param slot Slot number (`0`: Port 1, `1`: Port 2)
   * @param output Output buffer (2 bytes)
   * @return `true` if the input was read successfully, `false` otherwise
   */
  bool tryReadControllerInput(int slot, uint8_t* output);

  /**
   * @brief Read the data from PS1 memory card.
   * @param slot Slot number (`0`: Port 1, `1`: Port 2)
   * @param address Address of sector (0x00-0x3ff)
   * @param output Output buffer (128 bytes) if sets `nullptr`, this function only checks memory card availability.
   * @return `true` if the input was read successfully, `false` otherwise
   */
  bool tryReadFromMemoryCard(int slot, int address, uint8_t* output);

  /**
   * @brief Write the data to PS1 memory card.
   * @param slot Slot number (`0`: Port 1, `1`: Port 2)
   * @param address Address of sector (0x00-0x3ff)
   * @param input Input buffer (128 bytes)
   * @return `true` if the input was written successfully, `false` otherwise
   */
  bool tryWriteToMemoryCard(int slot, int address, uint8_t* input);
}
