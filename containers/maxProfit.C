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

int main()
{
  
  std::cout << maxProfit({}) << std::endl;
  std::cout << maxProfit({100}) << std::endl;
  std::cout << maxProfit({100, 10}) << std::endl;
  std::cout << maxProfit({0, 100, 300, 10, 1000}) << std::endl;
  std::cout << maxProfit({3, 10, 900, 1, 1000, 10}) << std::endl;
  return 0;
}
