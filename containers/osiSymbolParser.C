/* 
 * OSI Symbol format:
 * Symbol + YYMMDD + C/P + STRIKE (scaled)
 *
 * Symbol 1–6 characters (6-character field that is right-padded with spaces)
 * Strike is usually:
 *   8 digits
 *   implied decimal (divide by 1000)
 *
 * TSLA  250216P00500000 // Underlying stock: TSLA, Expiration: 2025-02-16, Put Option,  Stike  = 50.000
 * AAPL  240119C00180000 // Underlying stock: APPL, Expiration: 2024-01-19, Call Option, Strike = 180.000
 */

 // Compile: g++ -std=c++17 -Wall -Wextra -O2 osiSymbolParser.C -o osiSymbolParser

#include <iostream>
#include <cstdint>
#include <charconv> // std::from_chars()

struct osiSymbol
{
  public:
    std::string symbol{};
    std::string expirationDate{};
    char callOrPut{'\0'};   // CallOrPut is the 9th character from the end
    float strikePrice{0.0}; // Parse the last 8 characters as an integer and divide it by 1000

    void printSymbolInfo() const
    {
      std::cout << "Symbol     : " << symbol << std::endl
                << "Expiration : " << expirationDate << std::endl
                << "CallOrPut  : " << callOrPut << std::endl
                << "Strike     : " << strikePrice << std::endl
                << std::endl;
    }

    void getSymbol(const std::string& osiSymbol_)
    {
      auto pos = osiSymbol_.find(' '); // Find symbol
      if (pos == std::string::npos)
      {
        pos = maxSymbolLength;
      }

      symbol = osiSymbol_.substr(0, pos);
    }

    void getOsiSymbolInfo(const std::string& osiSymbol_)
    { 
      osiSymbolLen = osiSymbol_.length();

      getSymbol(osiSymbol_);
      getExpirationDate(osiSymbol_);
      getCallOrPut(osiSymbol_);
      getStrikePrice(osiSymbol_);
    }

  private:
    void getExpirationDate(const std::string& osiSymbol_)
    {
      const size_t start = osiSymbolLen - strikePriceStrLen - 1 - expirationDateLength;
      expirationDate = osiSymbol_.substr(start, expirationDateLength);
    }

    void getCallOrPut(const std::string& osiSymbol_)
    {
      const auto pos = osiSymbolLen - strikePriceStrLen -1;
      callOrPut = osiSymbol_[pos];
    }

    void getStrikePrice(const std::string& osiSymbol_)
    {
      if (std::from_chars(osiSymbol_.data() + osiSymbolLen - strikePriceStrLen, osiSymbol_.data() + osiSymbolLen, strikePrice).ec != std::errc{})
      {
        std::cout << "Error: Failed to get strike price for osi symbol - " << osiSymbol_ << std::endl;
        return;
      }

      strikePrice = strikePrice / scaleFactor;
    }

    uint8_t osiSymbolLen{0};
    static constexpr uint8_t strikePriceStrLen{8};
    static constexpr uint8_t maxSymbolLength{6}; // 6 characters symbol (no space padding)
    static constexpr uint8_t expirationDateLength{6};
    static constexpr uint16_t scaleFactor{1000};
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
    bool isValidOsiSymbol(const std::string& symbol_)
    {
      // Fow now, only validate length, we can add more checks if needed.
      if (symbol_.length() > osiSymbolMaxLength ||
          symbol_.length() < (osiSymbolMinLength - 5))
      {
        std::cout << "Error: Invalid OSI symbol - " << symbol_ << std::endl;
        return false;
      }

      return true;
    }

    void parseOsiSymbol(const std::string& symbol_)
    {
      if (isValidOsiSymbol(symbol_))
      {
        std::cout << "OSI Symbol : " << symbol_ << std::endl;
        m_symbol.getOsiSymbolInfo(symbol_);
      } 
    }

    mutable osiSymbol m_symbol;

    static constexpr uint8_t osiSymbolMaxLength{21};
    static constexpr uint8_t osiSymbolMinLength{osiSymbolMaxLength - 5}; // Minimum length of one character OSI symbol
};

int main()
{

  osiSymbolParser tsla("TSLA 250216P00500000");
  tsla.display();

  osiSymbolParser test("TEST.A260216C00000150");
  test.display();

  return 0;
}
