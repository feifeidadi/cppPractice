/* 
 * Benchmark: const std::string& vs std::string_view performance
 * Claude Haiku 4.5
 * 
 * Compile: g++ -std=c++17 -Wall -Wextra -O2 osiSymbolParser_benchmark.C -o osiSymbolParser_benchmark
 * Run:     ./osiSymbolParser_benchmark
 */

#include <iostream>
#include <cstdint>
#include <chrono>
#include <charconv>
#include <string_view>
#include <vector>

// ============================================================================
// VERSION 1: Using const std::string&
// ============================================================================

struct osiSymbol_StringRef
{
  public:
    std::string symbol{};
    std::string expirationDate{};
    char callOrPut{'\0'};
    float strikePrice{0.0};

    void getSymbol(const std::string& osiSymbol_)
    {
      auto pos = osiSymbol_.find(' ');
      if (pos == std::string::npos)
      {
        pos = 6;
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
      const size_t start = osiSymbolLen - 8 - 1 - 6;
      expirationDate = osiSymbol_.substr(start, 6);
    }

    void getCallOrPut(const std::string& osiSymbol_)
    {
      const auto pos = osiSymbolLen - 8 - 1;
      callOrPut = osiSymbol_[pos];
    }

    void getStrikePrice(const std::string& osiSymbol_)
    {
      if (std::from_chars(osiSymbol_.data() + osiSymbolLen - 8, osiSymbol_.data() + osiSymbolLen, strikePrice).ec != std::errc{})
      {
        return;
      }
      strikePrice = strikePrice / 1000.0;
    }

    uint8_t osiSymbolLen{0};
};

class osiSymbolParser_StringRef
{
  public:
    osiSymbolParser_StringRef(const std::string& symbol_)
    {
      if (symbol_.length() <= 21 && symbol_.length() >= 16)
      {
        m_symbol.getOsiSymbolInfo(symbol_);
      }
    }

    const osiSymbol_StringRef& getSymbol() const { return m_symbol; }

  private:
    osiSymbol_StringRef m_symbol;
};

// ============================================================================
// VERSION 2: Using std::string_view
// ============================================================================

struct osiSymbol_StringView
{
  public:
    std::string_view symbol{};
    std::string_view expirationDate{};
    char callOrPut{'\0'};
    float strikePrice{0.0};

    void getSymbol(std::string_view osiSymbol_)
    {
      auto pos = osiSymbol_.find(' ');
      if (pos == std::string::npos)
      {
        pos = 6;
      }
      symbol = osiSymbol_.substr(0, pos);
    }

    void getOsiSymbolInfo(std::string_view osiSymbol_)
    { 
      osiSymbolLen = osiSymbol_.length();
      getSymbol(osiSymbol_);
      getExpirationDate(osiSymbol_);
      getCallOrPut(osiSymbol_);
      getStrikePrice(osiSymbol_);
    }

  private:
    void getExpirationDate(std::string_view osiSymbol_)
    {
      const size_t start = osiSymbolLen - 8 - 1 - 6;
      expirationDate = osiSymbol_.substr(start, 6);
    }

    void getCallOrPut(std::string_view osiSymbol_)
    {
      const auto pos = osiSymbolLen - 8 - 1;
      callOrPut = osiSymbol_[pos];
    }

    void getStrikePrice(std::string_view osiSymbol_)
    {
      if (std::from_chars(osiSymbol_.data() + osiSymbolLen - 8, osiSymbol_.data() + osiSymbolLen, strikePrice).ec != std::errc{})
      {
        return;
      }
      strikePrice = strikePrice / 1000.0;
    }

    uint8_t osiSymbolLen{0};
};

class osiSymbolParser_StringView
{
  public:
    osiSymbolParser_StringView(std::string_view symbol_)
    {
      m_osiSymbolInput = std::string(symbol_);  // Store for lifetime
      if (symbol_.length() <= 21 && symbol_.length() >= 16)
      {
        m_symbol.getOsiSymbolInfo(m_osiSymbolInput);
      }
    }

    const osiSymbol_StringView& getSymbol() const { return m_symbol; }

  private:
    osiSymbol_StringView m_symbol;
    std::string m_osiSymbolInput;
};

// ============================================================================
// BENCHMARK CODE
// ============================================================================

void benchmark_StringRef()
{
  std::cout << "\n=== Benchmark: const std::string& ===" << std::endl;
  
  std::vector<std::string> testSymbols = {
    "TSLA 250216P00500000",
    "AAPL 240119C00180000",
    "MSFT 250331P00300000",
    "GOOGL250516C02500000",
    "AMZN 260117P00140000",
  };

  auto start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < 1000000; ++i)
  {
    for (const auto& symbol : testSymbols)
    {
      osiSymbolParser_StringRef parser(symbol);
      [[maybe_unused]] volatile auto s = parser.getSymbol().strikePrice;  // Use result to prevent optimization
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  
  std::cout << "Time: " << duration.count() << " ms" << std::endl;
  std::cout << "Iterations: 5,000,000 (5 symbols × 1,000,000 loops)" << std::endl;
}

void benchmark_StringView()
{
  std::cout << "\n=== Benchmark: std::string_view ===" << std::endl;
  
  std::vector<std::string> testSymbols = {
    "TSLA 250216P00500000",
    "AAPL 240119C00180000",
    "MSFT 250331P00300000",
    "GOOGL250516C02500000",
    "AMZN 260117P00140000",
  };

  auto start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < 1000000; ++i)
  {
    for (const auto& symbol : testSymbols)
    {
      osiSymbolParser_StringView parser(symbol);
      [[maybe_unused]] volatile auto s = parser.getSymbol().strikePrice;  // Use result to prevent optimization
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  
  std::cout << "Time: " << duration.count() << " ms" << std::endl;
  std::cout << "Iterations: 5,000,000 (5 symbols × 1,000,000 loops)" << std::endl;
}

void benchmark_StringLiteralRef()
{
  std::cout << "\n=== Benchmark: const std::string& with string literals ===" << std::endl;
  
  auto start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < 5000000; ++i)
  {
    // Passing string literals directly - forces temporary std::string creation
    osiSymbolParser_StringRef parser("TSLA 250216P00500000");
    [[maybe_unused]] volatile auto s = parser.getSymbol().strikePrice;
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  
  std::cout << "Time: " << duration.count() << " ms" << std::endl;
  std::cout << "Iterations: 5,000,000 (with string literal conversion)" << std::endl;
}

void benchmark_StringLiteralView()
{
  std::cout << "\n=== Benchmark: std::string_view with string literals ===" << std::endl;
  
  auto start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < 5000000; ++i)
  {
    // Passing string literals directly - no conversion needed
    osiSymbolParser_StringView parser("TSLA 250216P00500000");
    [[maybe_unused]] volatile auto s = parser.getSymbol().strikePrice;
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  
  std::cout << "Time: " << duration.count() << " ms" << std::endl;
  std::cout << "Iterations: 5,000,000 (no string literal conversion)" << std::endl;
}

int main()
{
  std::cout << "==============================================================" << std::endl;
  std::cout << "OSI Symbol Parser - Performance Benchmark" << std::endl;
  std::cout << "==============================================================" << std::endl;

  std::cout << "\nTest 1: Parsing pre-created std::string objects" << std::endl;
  std::cout << "(Both methods should be similar)" << std::endl;
  benchmark_StringRef();
  benchmark_StringView();

  std::cout << "\n\nTest 2: Parsing string literals directly" << std::endl;
  std::cout << "(string_view should be faster - no temp allocation)" << std::endl;
  benchmark_StringLiteralRef();
  benchmark_StringLiteralView();

  std::cout << "\n==============================================================" << std::endl;
  std::cout << "Benchmark Complete" << std::endl;
  std::cout << "==============================================================" << std::endl;

  return 0;
}
