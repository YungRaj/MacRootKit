#include "fuzztest/fuzztest.h"
#include "gtest/gtest.h"

#include "macho.h"

TEST(MachOTest, BasicMachOFileParses) {
  EXPECT_EQ(1 + 2, 2 + 1);
}

void DummyMachOFuzz(int a, int b) {
  EXPECT_EQ(a + b, b + a);
}

FUZZ_TEST(MachOTest, DummyMachOFuzz);
