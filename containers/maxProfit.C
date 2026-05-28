/*
 * Best Time to Buy and Sell Stock
 * You are given an array prices where prices[i] is the price of a given stock on the ith day.
 * You want to maximize your profit by choosing a single day to buy one stock and choosing a different day in the future to sell that stock.
 * Return the maximum profit you can achieve from this transaction. If you cannot achieve any profit, return 0.
 */

#include <iostream>
#include <vector>
#include <algorithm>

int maxProfit(const std::vector<int>& prices)
{
  if (prices.empty())
  {
    return 0;
  }

  int profit{0}, maxProfit{0}, minPrice{prices[0]};
  for (size_t i = 1; i < prices.size(); i++)
  {
    minPrice = std::min(minPrice, prices[i]);
    profit = prices[i] - minPrice;
    maxProfit = std::max(maxProfit, profit);
  }

  return maxProfit;
}

#ifdef __GOOGLE_TEST__
#include <gtest/gtest.h>
class MaxProfitTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test cases
TEST_F(MaxProfitTest, NonTradable) {
    std::vector<int> prices = {};
    EXPECT_EQ(maxProfit(prices), 0);
}

TEST_F(MaxProfitTest, NonProfitable) {
    std::vector<int> prices = {100, 90, 80, 70, 60, 50};
    EXPECT_EQ(maxProfit(prices), 0);
}

TEST_F(MaxProfitTest, samePriceEveryday) {
    std::vector<int> prices = {10, 10, 10, 10, 10, 10};
    EXPECT_EQ(maxProfit(prices), 0);
}

TEST_F(MaxProfitTest, StandardCase) {
    std::vector<int> prices = {7, 1, 5, 3, 6, 4};
    EXPECT_EQ(maxProfit(prices), 5);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#else

int main()
{
  
  std::cout << maxProfit({}) << std::endl;
  std::cout << maxProfit({100}) << std::endl;
  std::cout << maxProfit({100, 10}) << std::endl;
  std::cout << maxProfit({0, 100, 300, 10, 1000}) << std::endl;
  std::cout << maxProfit({3, 10, 900, 1, 1000, 10}) << std::endl;
  return 0;
}

#endif
