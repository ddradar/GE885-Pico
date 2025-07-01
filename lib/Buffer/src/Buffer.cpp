
#include "Buffer.h"
#include <Arduino.h>

namespace Buffer {
  namespace {
    uint8_t __buffer[INTERNAL_BUFFER_SIZE];
    int __startAddress = 0;
    int __written = 0;

    /**
     * @brief Reset the internal buffer
     * @param address Address to reset the buffer to (0x00 - 0x3FFFFF)
     */
    void __resetBuffer(int address) {
      __startAddress = address;
      __written = 0;
      memset(__buffer, 0, sizeof(__buffer));
    }
  }

  /**
   * @brief Add data to the buffer
   * @param address Address to set (0x00 - 0x3FFFFF)
   * @param data Data to add
   */
  void add(int address, uint8_t data) {
    assert(address >= 0 && address < GE885_RAM_SIZE);

    if (address < __startAddress || address >= __startAddress + INTERNAL_BUFFER_SIZE) {
      __resetBuffer(address);
    }

    int offset = address - __startAddress;
    __buffer[offset] = data;
    if (offset + 1 > __written) {
      __written = offset + 1;
    }
  }

  /**
   * @brief Add data to the buffer
   * @param address Address to set (0x00 - 0x400000)
   * @param data Data pointer to add
   * @param length Length of the data
   */
  void add(int address, uint8_t *data, int length) {
    assert(length > 0);
    assert(address >= 0 && address + length <= GE885_RAM_SIZE);

    if (address < __startAddress || address + length > __startAddress + INTERNAL_BUFFER_SIZE) {
      __resetBuffer(address);
    }

    int offset = address - __startAddress;
    int writable = INTERNAL_BUFFER_SIZE - offset;
    if (length > writable) length = writable;
    memcpy(&__buffer[offset], data, length);
    if (offset + length > __written) {
      __written = offset + length;
    }
  }

  /**
   * @brief Get a pointer to the data at the specified address
   * @param address Address to get the pointer for (0x00 - 0x3FFFFF)
   * @return Pointer to the data
   */
  uint8_t *getPointer(int address) {
    assert(address >= 0 && address < GE885_RAM_SIZE);
    if (address < __startAddress || address >= __startAddress + __written) {
      __resetBuffer(address);
    }
    return &__buffer[address - __startAddress];
  }
};
