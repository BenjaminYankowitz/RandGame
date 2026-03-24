#ifdef FAKEGTESTFORCLANG
#define TEST(a,b) void a##b() //NOLINT
#define EXPECT_TRUE(a) (void) (a)
#define ASSERT_TRUE(a) if(!a) std::exit(1)
#define EXPECT_FALSE(a) (void) !(a)
#define EXPECT_EQ(a,b) (void) ((a)==(b))
#define EXPECT_DOUBLE_EQ(a,b) (void) ((a)==(b))
#define EXPECT_LE(a,b) (void) ((a)<=(b))
#define EXPECT_GE(a,b) (void) ((a)>=(b))
#define EXPECT_NE(a,b) (void) ((a)!=(b))
#define EXPECT_STREQ(a , b) (void) (std::string_view(a)==std::string_view(b))
#else
#include <gtest/gtest.h>
#endif