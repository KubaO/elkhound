// bitarray.cc            see license.txt for copyright and terms of use
// code for bitarray.h

#include "bitarray.h"     // this module
#include "flatten.h"      // Flatten

#include <string.h>       // memset


BitArray::BitArray(int count, bool value)
{
  allocBits(count, value);
}


void BitArray::allocBits(int count, bool value)
{
  bits.clear();
  bits.resize((count + 7) >> 3, value ? 0xFF : 0x00);
  if (value)
    ensureInvariant();
  numBits = count;
}


void BitArray::ensureInvariant() noexcept
{
  if (numBits & 7)
    bits.back() &= 0xFF >> (8 - (numBits & 7));
}


void BitArray::xfer(Flatten &flat)
{
  int count = this->numBits;
  flat.xferInt(count);

  if (flat.reading()) {
    allocBits(count);
  }
  flat.xferSimple(bits.data(), bits.size());
}


// these rely on the invariant that the unused trailing
// bits are always set to 0
bool BitArray::operator== (BitArray const &obj) const noexcept
{
  return numBits == obj.numBits && bits == obj.bits;
}

bool BitArray::operator!= (BitArray const& obj) const noexcept
{
  return numBits != obj.numBits || bits != obj.bits;
}


void BitArray::clearAll() noexcept
{
  if (numBits)
    memset(bits.data(), 0, bits.size());
}


BitArray& BitArray::flip() noexcept
{
  for (uint8_t& byte : bits)
    byte ^= 0xFF;
  ensureInvariant();
  return *this;
}


void BitArray::selfCheck() const
{
  // check that enough bytes are allocated
  xassert(bits.size() == (numBits + 7) >> 3);
  if (numBits & 7) {
    // there are some trailing bits that I need to check
    unsigned char mask = (1 << (numBits & 7)) - 1;     // bits to *not* check
    unsigned char zero = bits.back() & ~mask;
    xassert(zero == 0);
  }
}


BitArray& BitArray::unionWith(BitArray const &obj)
{
  xassert(numBits == obj.numBits);
  for (int i=0; i<bits.size(); i++) {
    bits[i] |= obj.bits[i];
  }
  return *this;
}


BitArray& BitArray::intersectWith(BitArray const &obj) noexcept
{
  xassert(numBits == obj.numBits);
  for (int i=0; i<bits.size(); i++) {
    bits[i] &= obj.bits[i];
  }
  return *this;
}


bool BitArray::anyEvenOddBitPair() const noexcept
{
  for (uint8_t b : bits) {
    if (b & (b >> 1) & 0x55) {        // 01010101
      return true;
    }
  }
  return false;    // no such pair
}


BitArray stringToBitArray(string_view src)
{
  BitArray ret(src.size());
  for (int i=0; i<src.size(); i++) {
    if (src[i]=='1') {
      ret[i] = true;
    }
  }
  return ret;
}

string toString(BitArray const &src)
{
  string ret(src.length(), '0');
  for (int i=0; i<ret.length(); i++) {
    if (src[i])
      ret[i] = '1';
  }
  return ret;
}

// -------------------- test code -------------------
#ifdef TEST_BITARRAY

#include "test.h"     // USUAL_MAIN

string toStringViaIter(BitArray const &b)
{
  string ret;
  ret.reserve(b.length());

  for (bool bit : b) {
    ret.push_back(bit ? '1' : '0');
  }
  return ret;
}


void testIter(char const *str)
{
  BitArray b = stringToBitArray(str);
  b.selfCheck();

  string s1 = toString(b);
  string s2 = toStringViaIter(b);
  if (s1 != s2 || s1 != str) {
    std::cout << "str: " << str << std::endl;
    std::cout << " s1: " << s1 << std::endl;
    std::cout << " s2: " << s2 << std::endl;
    xbase("testIter failed");
  }

  // also test the inverter
  BitArray c = ~b;
  c.selfCheck();

  stringBuilder inv;
  int len = strlen(str);
  for (int i=0; i<len; i++) {
    inv << (str[i]=='0'? '1' : '0');
  }

  string cStr = toString(c);
  if (inv != cStr) {
    std::cout << " inv: " << inv << std::endl;
    std::cout << "cStr: " << cStr << std::endl;
    xbase("test inverter failed");
  }
}


void testUnionIntersection(char const *s1, char const *s2)
{
  int len = strlen(s1);
  xassert(len == (int)strlen(s2));

  BitArray b1 = stringToBitArray(s1);
  BitArray b2 = stringToBitArray(s2);

  stringBuilder expectUnion, expectIntersection;
  for (int i=0; i<len; i++) {
    expectUnion        << ((s1[i]=='1' || s2[i]=='1')? '1' : '0');
    expectIntersection << ((s1[i]=='1' && s2[i]=='1')? '1' : '0');
  }

  BitArray u = b1 | b2;
  BitArray i = b1 & b2;

  string uStr = toString(u);
  string iStr = toString(i);

  if (uStr != expectUnion) {
    std::cout << "         s1: " << s1 << std::endl;
    std::cout << "         s2: " << s2 << std::endl;
    std::cout << "       uStr: " << uStr << std::endl;
    std::cout << "expectUnion: " << expectUnion << std::endl;
    xbase("test union failed");
  }

  if (iStr != expectIntersection) {
    std::cout << "                s1: " << s1 << std::endl;
    std::cout << "                s2: " << s2 << std::endl;
    std::cout << "              iStr: " << iStr << std::endl;
    std::cout << "expectIntersection: " << expectIntersection << std::endl;
    xbase("test intersection failed");
  }
}


void testAnyEvenOddBitPair(char const *s, bool expect)
{
  BitArray b = stringToBitArray(s);
  bool answer = b.anyEvenOddBitPair();
  if (answer != expect) {
    static char const *boolName[] = { "false", "true" };
    std::cout << "     s: " << s << std::endl;
    std::cout << "answer: " << boolName[answer] << std::endl;
    std::cout << "expect: " << boolName[expect] << std::endl;
    xbase("test anyEvenOddBitPair failed");
  }
}


void entry()
{
        //            1111111111222222222233333333334444444444555555555566
        //  01234567890123456789012345678901234567890123456789012345678901
  testIter("00000000111111111111000000000000");
  testIter("00000000000000000000000000000000000000111111111111000000000000");
  testIter("000000000000000000000000000000000000000111111111111000000000000");
  testIter("0000000000000000000000000000000000000000111111111111000000000000");
  testIter("00000000000000000000000000000000000000000111111111111000000000000");
  testIter("000000000000000000000000000000000000000000111111111111000000000000");
  testIter("0000000000000000000000000000000000000000000111111111111000000000000");
  testIter("00000000000000000000000000000000000000000000111111111111000000000000");
  testIter("000000000000000000000000000000000000000000000111111111111000000000000");
  testIter("0000000000000000000000000000000000000000000000111111111111000000000000");
  testIter("00000000000000000000000000000000000000000000000111111111111000000000000");
  testIter("000000000000000000000000000000000000000000000000111111111111000000000000");

  testIter("0101");
  testIter("1");
  testIter("0");
  testIter("");
  testIter("1111");
  testIter("0000");
  testIter("000000000000111111111111000000000000");
  testIter("111111111111111000000000000011111111");
  testIter("10010110010101010100101010101010100110001000100001010101111");

  testUnionIntersection("",
                        "");

  testUnionIntersection("1",
                        "0");

  testUnionIntersection("10",
                        "00");

  testUnionIntersection("1001000100111110101001001001011111",
                        "0001100101011101011010000111010110");

  testUnionIntersection("1111111111111111111111111111111111",
                        "0000000000000000000000000000000000");

  testUnionIntersection("0000111111000001111110000011110000",
                        "1111000000111110000001111100001111");

  testAnyEvenOddBitPair("0000", false);
  testAnyEvenOddBitPair("0001", false);
  testAnyEvenOddBitPair("0010", false);
  testAnyEvenOddBitPair("0100", false);
  testAnyEvenOddBitPair("1000", false);
  testAnyEvenOddBitPair("0110", false);
  testAnyEvenOddBitPair("1110", true);
  testAnyEvenOddBitPair("0111", true);
  testAnyEvenOddBitPair("1111", true);
  testAnyEvenOddBitPair("11110", true);
  testAnyEvenOddBitPair("01100", false);

  std::cout << "bitarray is ok\n";
}

USUAL_MAIN

#endif // TEST_BITARRAY
