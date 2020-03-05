#include <unity.h>
#include <iostream>
#include "QuantizeMethods.h"

using namespace std;

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void print(int value) {
  cout << "value: " << value << endl;
}

int numSteps = 8;
int ppqn = 24;

void test_quantize_16th() {
  QuantizeMode mode = QUANT_16;
  int value;
  
  value = quantize(mode, 22, 0, numSteps, ppqn);
  print(value);
  TEST_ASSERT_EQUAL(24, value);
  
  value = quantize(mode, 26, 1, numSteps, ppqn);
  print(value);
  TEST_ASSERT_EQUAL(24, value);

  value = quantize(mode, 27, 1, numSteps, ppqn);
  print(value);
  TEST_ASSERT_EQUAL(30, value);

  value = quantize(mode, 191, 7, numSteps, ppqn);
  print(value);
  TEST_ASSERT_EQUAL(0, value);
}


int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_quantize_16th);
    UNITY_END();
    return 0;
}