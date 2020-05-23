/**
 * https://www.geeksforgeeks.org/bitwise-operators-in-c-cpp/
 * 
 * The & (bitwise AND) The result of AND is 1 only if both bits are 1.
 * The | (bitwise OR) The result of OR is 1 if any of the two bits is 1.
 * The ^ (bitwise XOR) The result of XOR is 1 if the two bits are different.
 * The << (left shift) in C or C++ takes two numbers, left shifts the bits of the first operand, the second operand decides the number of places to shift.
 * The >> (right shift) in C or C++ takes two numbers, right shifts the bits of the first operand, the second operand decides the number of places to shift.
 * The ~ (bitwise NOT) in C or C++ takes one number and inverts all bits of it 
 * 
 * 
 * settings, clearing, toggling single bits --> https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
 * 
 * combine two 8 bit values into 16 bit value --> https://stackoverflow.com/questions/11193918/combine-merge-two-bytes-into-one/11193978
**/


#include <iostream>
#include <bitset>

// using namespace std;

// compile -->  g++ -o bitwise bitwise.cpp
// run     -->  ./bitwise

int main()
{
  uint16_t num = 14507;
  int chanA = 0;
  int chanB = 1;
  int chanC = 2;
  int chanD = 3;

  int controlBits = 0xFF;
  
  std::bitset<16> integerOutput = num;

  std::bitset<8> byte1( controlBits << 2);
  
  std::bitset<8> byte2((num >> 8) & 0xFF);
  
  std::bitset<8> byte3(num & 0xFF);
  
  std::cout << byte1 << std::endl;
  std::cout << byte2 << std::endl;
  std::cout << byte3 << std::endl;
  
  
  std::cout << integerOutput << std::endl;
  return 0;
}