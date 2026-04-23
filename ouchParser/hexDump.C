#include <iomanip>
#include "hexDump.H"

constexpr size_t bytesPerLine = 16;
void printAscii(const uint8_t *data, const uint8_t size, const uint8_t numLeadingSpace = 1)
{
  for (size_t i = 0; i < numLeadingSpace; ++i)
  {
    printf(" ");
  }

  for (size_t i = 0; i < size; ++i) {
    printf("%c", std::isprint(data[i]) ? data[i] : '.');
  }
  printf("\n");
}

void hexDump([[maybe_unused]]const Packet& data)
{
#ifdef DEBUG
  for (size_t i = 0; i < data.size(); ++i)
  {
    printf("%02x ", data[i]); // Hex bytes
    const auto numBytesInLine = (i + 1) % bytesPerLine;
    if (numBytesInLine == 0) // Print ASCII after hex bytes
    {
      printAscii(data.data() + i - (bytesPerLine - 1), bytesPerLine);
    }
    else if (i == (data.size()-1)) // Print last line's ASCII characters
    {
      printAscii(data.data() + ( data.size() - numBytesInLine),  /* Start address of last line */
                 numBytesInLine,
                 (bytesPerLine - numBytesInLine) * 3 + 1 /* Number of leading spaces in the last ASCII line */);

    }
  }
  printf("\n");
#endif
}
