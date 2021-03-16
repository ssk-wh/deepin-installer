#include <iostream>
#include <gtest/gtest.h>

int add(int i, int k) {
    return i + k;
}

// 正常场景用例
TEST(FooTestCase1, HandleNoneZeroInput)
{
    EXPECT_EQ(2, add(1, 1));
    EXPECT_EQ(3, add(1, 2));
    EXPECT_EQ(4, add(1, 3));
}

// 异常场景用例
TEST(FooTestCase2, HandleNoneZeroInput)
{
    EXPECT_EQ(2.2, add(1.1, 1.1));
    EXPECT_EQ(3.3, add(1.1, 2.2));
    EXPECT_EQ(4.4, add(1.1, 3.3));
}

