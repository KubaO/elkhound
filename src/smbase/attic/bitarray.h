// bitarray.h            see license.txt for copyright and terms of use
// one-dimensional array of bits

#ifndef BITARRAY_H
#define BITARRAY_H

#include <vector>         // std::vector<unsigned char>
#include "stdint.h"       // uint8_t
#include "xassert.h"      // xassert
#include "str.h"          // string

class Flatten;            // flatten.h

class BitArray {
private:
  std::vector<uint8_t> bits;
  int numBits;              // # of bits in the array

  // invariant: the bits in the last allocated byte that are beyond
  // 'numBits' (if any) are always 0

  template <bool> class ProtoIterator;
  friend class ProtoIterator<false>;

  class Reference {
    uint8_t* const byte;
    uint8_t const mask;
  public:
    Reference(uint8_t* byte, uint8_t mask) noexcept : byte(byte), mask(mask) {}
    operator bool() const noexcept { return *byte & mask; }
    Reference& operator=(bool b) noexcept {
      if (b)
        *byte |= mask;
      else
        *byte &= ~mask;
      return *this;
    }
    void flip() noexcept { *byte ^= mask; }
  };

  void allocBits(int numBits, bool value = false);
  void ensureInvariant() noexcept;

public:
  BitArray() = default;
  BitArray(const BitArray&) = default;
  BitArray(BitArray&&) = default;
  BitArray& operator=(const BitArray&) = default;
  BitArray& operator=(BitArray&&) = default;
  BitArray(int count, bool value = false);

  void xfer(Flatten &flat);

  bool operator== (BitArray const& obj) const noexcept;
  bool operator!= (BitArray const& obj) const noexcept;

  int length() const noexcept { return numBits; }
  int size() const noexcept { return numBits; }
  bool empty() const noexcept { return numBits == 0; }

  bool operator[](int bit) const
  {
    xassert(bit <= numBits);
    return bits[bit >> 3] & (1 << (bit & 7));
  }
  Reference operator[](int bit)
  {
    xassert(bit <= numBits);
    return Reference(bits.data() + (bit >> 3), 1 << (bit & 7));
  }

  // clear all bits
  void clearAll() noexcept;

  // invert the bits
  BitArray &flip() noexcept;

  // bitwise OR ('obj' must be same length)
  BitArray &unionWith(BitArray const &obj);

  // bitwise AND
  BitArray& intersectWith(BitArray const& obj) noexcept;

  // true if there is any pair of bits 2n,2n+1 where both are set
  bool anyEvenOddBitPair() const noexcept;

  // debug check
  void selfCheck() const;

  // operators
  BitArray operator~ () const {
    return BitArray(*this).flip();
  }

  BitArray& operator|= (BitArray const &obj) {
    return unionWith(obj);
  }

  BitArray operator| (BitArray const& obj) const {
    return BitArray(*this) |= obj;
  }

  BitArray& operator&= (BitArray const &obj) noexcept {
    return intersectWith(obj);
  }

  BitArray operator& (BitArray const &obj) const {
    return BitArray(*this) &= obj;
  }

  template <bool is_const>
  class ProtoIterator {
  protected:
    using value_type = std::conditional_t<is_const, const uint8_t, uint8_t>;
    value_type* byte;
    uint8_t mask;
    ProtoIterator(value_type* byte, uint8_t mask) : byte(byte), mask(mask) {}
  public:
    ProtoIterator() = default;
    ProtoIterator(const ProtoIterator&) = default;
    ProtoIterator& operator=(const ProtoIterator&) = default;
    bool operator==(const ProtoIterator& other) const noexcept
      { return byte == other.byte && mask == other.mask; }
    bool operator!=(const ProtoIterator& other) const noexcept
      { return byte != other.byte || mask != other.mask; }
    ProtoIterator& operator++() noexcept
    {
      if (mask & 0x7F) {
        mask <<= 1;
      }
      else {
        ++byte;
        mask = 1;
      }
      return *this;
    }
    ProtoIterator operator++(int) const noexcept
    {
      return ++Iterator(*this);
    }
    ProtoIterator& operator--() noexcept
    {
      if (mask & 0xFE) {
        mask >>= 1;
      }
      else {
        --byte;
        mask = 0x80;
      }
      return *this;
    }
    ProtoIterator operator--(int) const noexcept
    {
      return --Iterator(*this);
    }
  };
  class Iterator : public ProtoIterator<false>
  {
    friend class BitArray;
  public:
    using ProtoIterator::ProtoIterator;
    BitArray::Reference operator*() const noexcept { return BitArray::Reference(byte, mask); }
  };
  class ConstIterator : public ProtoIterator<true>
  {
    friend class BitArray;
  public:
    using ProtoIterator::ProtoIterator;
    bool operator*() const noexcept { return *byte & mask; }
  };

  Iterator begin() noexcept { return Iterator(bits.data(), 1); }
  Iterator end() noexcept { return Iterator(bits.data() + (numBits >> 3), 1 << (numBits & 7)); }
  ConstIterator begin() const noexcept { return ConstIterator(bits.data(), 1); }
  ConstIterator end() const noexcept { return ConstIterator(bits.data() + (numBits >> 3), 1 << (numBits & 7)); }
  ConstIterator cbegin() const noexcept { return begin(); }
  ConstIterator cend() const noexcept { return end(); }
};


BitArray stringToBitArray(string_view src);
string toString(BitArray const &src);


#endif // BITARRAY_H
