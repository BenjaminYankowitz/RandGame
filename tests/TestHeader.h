#ifdef FAKEGTESTFORCLANG
#define TEST(a, b) void a##b() // NOLINT
#define EXPECT_TRUE(a) (void)(a)
#define ASSERT_TRUE(a) EXPECT_TRUE(a)
#define EXPECT_FALSE(a) (void)!(a)
#define EXPECT_EQ(a, b) (void)((a) == (b))
#define ASSERT_EQ(a, b) EXPECT_EQ(a, b)
#define EXPECT_NE(a, b) (void)((a) != (b))
#define ASSERT_NE(a, b) EXPECT_NE(a, b)
#define EXPECT_DOUBLE_EQ(a, b) (void)((a) == (b))
#define EXPECT_LE(a, b) (void)((a) <= (b))
#define EXPECT_GE(a, b) (void)((a) >= (b))
#define EXPECT_GT(a, b) (void)((a) > (b))
#define EXPECT_STREQ(a, b) (void)(std::string_view(a) == std::string_view(b))
#else
#include <gtest/gtest.h>
#endif