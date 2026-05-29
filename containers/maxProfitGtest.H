#pragma once

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

