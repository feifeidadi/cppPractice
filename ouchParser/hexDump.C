#include <iomanip>
#include "hexDump.H"

static constexpr size_t bytesPerLine = 16;
void printLeadingSpaces(const uint8_t numLeadingSpace)
{
  for (size_t i = 0; i < numLeadingSpace; ++i)
  {
    printf(" ");
  }
}

void printASCII(const uint8_t *data, const size_t size)
{
  const auto numSpace = (size == bytesPerLine ? 1: ((bytesPerLine - size) * 3 + 1));
  printLeadingSpaces(numSpace);

  for (size_t i = 0; i < size; ++i)
  {
    printf("%c", std::isprint(data[i]) ? data[i] : '.');
  }
  printf("\n");
}

void printHexBytes(const uint8_t *data, const size_t size)
{
  for (size_t i = 0; i < size; ++i)
  {
    printf("%02x ", data[i]); // Hex bytes
  }
}

// Print hex bytes alongside ASCII characters.
void printHexAndAscii(const uint8_t *data, const size_t size)
{
  printHexBytes(data, size);
  printASCII(data, size);
}

void hexDump([[maybe_unused]]const uint8_t *data, [[maybe_unused]]const size_t size)
{
#ifdef DEBUG
  for (size_t i = 0; i < size; i+=bytesPerLine)
  {
    printHexAndAscii(data + i, 
                     // data size is (size - i) for last line
                     i + bytesPerLine < size ? bytesPerLine : size - i);
  }

  printf("\n");
#endif
}
