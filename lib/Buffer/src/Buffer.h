
#include <Arduino.h>

// Buffer size of real GE885-PWB(A) hardware (4MB)
#define GE885_RAM_SIZE 0x400000
// Buffer size of the internal buffer (128KB)
#define INTERNAL_BUFFER_SIZE 0x20000

namespace Buffer {
  /**
   * @brief Add data to the buffer
   * @param address Address to set (0x00 - 0x3FFFFF)
   * @param data Data to add
   */
  void add(int address, uint8_t data);

  /**
   * @brief Add data to the buffer
   * @param address Address to set (0x00 - 0x3FFFFF)
   * @param data Data pointer to add
   * @param length Length of the data
   */
  void add(int address, uint8_t *data, int length);

  /**
   * @brief Get a pointer to the data at the specified address
   * @param address Address to get the pointer for (0x00 - 0x3FFFFF)
   * @return Pointer to the data
   */
  uint8_t *getPointer(int address);
};
