/* 
 * OSI Symbol format:
 * ROOT + YYMMDD + C/P + STRIKE (scaled)
 *
 * ROOT 1–6 characters (6-character field that is right-padded with spaces)
 * Strike is usually:
 *   8 digits
 *   implied decimal (divide by 1000)
 *
 * TSLA  250216P00500000 // Underlying stock: TSLA, Expiration: 2025-02-16, Put option,  Stike  = 50.000
 * AAPL  240119C00180000 // Underlying stock: APPL, Expiration: 2024-01-19, Call Option, Strike = 180.000
 */

 // Compile: g++ -std=c++17 -Wall -Wextra -O2 osiSymbolParser.C -o osiSymbolParser

#include <iostream>
#include <charconv> // from_chars()

#define OSI_SYMBOL_LENGTH 21
#define EXPIRATION_DATE_LENGTH 6

struct osiSymbol
{
  public:
    std::string rootSymbol = "";
    std::string expirationDate = "";
    char callOrPut = '\0';
    float strikePrice = 0.0;

    void getRootSymbol(const std::string& osiSymbol_)
    {
      pos = osiSymbol_.find(' '); // Find symbol
      if (pos == std::string::npos)
      {
        pos = 6; // 6 characters root symbol
      }

      rootSymbol = osiSymbol_.substr(0, pos);
      if (pos < 6)
      {
        pos++; // Point to expiration date after getting root symbol
      }
    }

    void getOsiSymbolInfo(const std::string& osiSymbol_)
    { 
      // TODO: Getting osi symbol info in the reversed sequence to avoid using pos internal variable.
      getRootSymbol(osiSymbol_);
      getExpirationDate(osiSymbol_);
      getCallOrPut(osiSymbol_);
      getStrikePrice(osiSymbol_);
    }

    void getExpirationDate(const std::string& osiSymbol_)
    {
      expirationDate = osiSymbol_.substr(pos, EXPIRATION_DATE_LENGTH);
      // Strip space at beginning if it's available
      pos += EXPIRATION_DATE_LENGTH;
    }

    void getCallOrPut(const std::string& osiSymbol_)
    {
      callOrPut = osiSymbol_[pos++];
    }

    void getStrikePrice(const std::string& osiSymbol_)
    {
      const auto len = osiSymbol_.length();
      if (std::from_chars(osiSymbol_.data() + len - 8, osiSymbol_.data() + len, strikePrice).ec != std::errc{})
      {
        std::cout << "Error: Failed to get strike price for osi symbol - " << osiSymbol_ << std::endl;
      }
      else
      {
        strikePrice = strikePrice / 1000;
      }
    }

    void printSymbolInfo() const
    {
      std::cout << "ROOT Symbol: " << rootSymbol << std::endl
                << "Expiration : " << expirationDate << std::endl
                << "CallOrPut  : " << callOrPut << std::endl
                << "Strike     : " << strikePrice << std::endl;
    }
  private:
    size_t pos{0};
};

class osiSymbolParser
{
  public:
    osiSymbolParser(const std::string& symbol_)
    {
      parseOsiSymbol(symbol_);
    }
    ~osiSymbolParser() {}

    void display() const
    {
      m_symbol.printSymbolInfo();
    }

  private:
    int parseOsiSymbol(const std::string& symbol_)
    {
      if (symbol_.length() > OSI_SYMBOL_LENGTH ||
          symbol_.length() < (OSI_SYMBOL_LENGTH - 5) /* Minimum length of one character OSI symbol  */)
      {
        std::cout << "Error: Invalid OSI symbol - " << symbol_ << std::endl;
      }

      m_symbol.getOsiSymbolInfo(symbol_);

      std::cout << symbol_ << std::endl;
      return 0;
    }

    mutable osiSymbol m_symbol;
};

int main()
{

  osiSymbolParser tsla("TSLA 250216P00500000");
  tsla.display();

  osiSymbolParser test("TEST.A260216P00000150");
  test.display();

  return 0;
}
