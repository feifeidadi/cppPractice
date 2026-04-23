#include "ouchParser.H"

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <capture_file>\n";
    return 1;
  }

  ouchParser parser;
  if (0 == parser.parsePacketFile(argv[1]))
  {
     parser.printStats();
  }

  return 0;
}
