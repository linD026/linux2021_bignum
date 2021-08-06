#ifndef __FACTORIAL_H_
#define __FACTORIAL_H_

#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstring>

/* how large the underlying array size should be */
#define UNIT_SIZE 4

/* These are dedicated to UNIT_SIZE == 4 */
#define UTYPE uint32_t
#define UTYPE_TMP uint64_t
#define FORMAT_STRING "%.08x"
#define MAX_VAL ((UTYPE_TMP)0xFFFFFFFF)

#define BN_ARRAY_SIZE (128 / UNIT_SIZE) /* size of big-numbers in bytes */

#define POSITIVE true
#define NAGETIVE false

#define NORMAL_SIZE(array, size)                                               \
  do {                                                                         \
    for (int _i = 0; _i < BN_ARRAY_SIZE; _i++) {                               \
      if (!array[_i]) {                                                        \
        size = _i - 1;                                                         \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  } while (0)

#define digit_div(n1, n0, d, q, r)                                             \
  __asm__("divq %4" : "=a"(q), "=d"(r) : "0"(n0), "1"(n1), "rm"(d))

#define SWAP(A, B)                                                             \
  do {                                                                         \
    typeof(A) tmp = A;                                                         \
    A = B;                                                                     \
    B = tmp;                                                                   \
  } while (0)

class big_number {
private:
  static void _to_inc(void) {_to++;}
  static void _to_dec(void) {_to--;}

    /* bn consists of array of TYPE */
  struct bn {
    UTYPE array[BN_ARRAY_SIZE];
  } n;

  bool sign;

public:
  char *strbuf;
  static size_t _to;
  size_t total_object(void) {return _to;}

  big_number() {
    bn_init();
    sign = POSITIVE;
    _to_inc();
  }

  big_number(int i) {
    printf("allocated\n");
    bn_init();
    sign = POSITIVE;
    strbuf = new char[8912];
    _to_inc();
  }

  ~big_number() {
    _to_dec();
  }

  void release(void) {
    printf("delete\n");
    delete [] strbuf;
  }

  inline void bn_init(void) {
    for (int i = 0; i < BN_ARRAY_SIZE; ++i)
      n.array[i] = 0;
  }

  inline void bn_from_int(UTYPE_TMP i) {
    bn_init();

    const union {
      uint16_t set;
      char get[2];
    } _type = {.set = 0x1122};
    int start = 0;
    int next = 1;
    if (_type.get[0] == 0x11) {
      start = BN_ARRAY_SIZE - 1;
      next = start - 1;
    }
    n.array[start] = i;

    /* bit-shift with U64 operands to force 64-bit results */
    UTYPE_TMP tmp = i >> 32;
    n.array[next] = tmp;
  }

  void bn_to_str(char *str, int nbytes);
  void bn_dec(void);
  void bn_add(big_number b, big_number &c);

  inline void lshift_unit(int n_units) {
    int i;
    /* Shift whole units */
    for (i = (BN_ARRAY_SIZE - 1); i >= n_units; --i)
      n.array[i] = n.array[i - n_units];
    /* Zero pad shifted units */
    for (; i >= 0; --i)
      n.array[i] = 0;
  }

  void bn_mul(big_number b, big_number &c);
  bool bn_is_zero(void);
  void bn_assign(big_number & dst);

  const char radix_chars[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  UTYPE ddivi(big_number & _n);
  void bn_2_str(char *out);
  
  /**
   * change sign
   */
  void pos_to_neg(void) {
    if (sign) {
      sign = NAGETIVE;
    }
  }
  void neg_to_pos(void) {
    if (!sign) {
      sign = POSITIVE;
    }
  }
  big_number bn_minus(big_number src) {
    big_number tmp;
    tmp = *this;
    for (big_number i = src; !i.bn_is_zero() ; i.bn_dec()) {
      tmp.bn_dec();
    }
    return tmp;
  }

  /**
   * operator overloading
   */
  big_number & operator=(big_number src) {
    src.bn_assign(*this);
    sign = src.sign;
    return *this;
  }
  big_number & operator=(uint64_t src) {
    bn_from_int(src);
    return *this;
  }

  big_number operator-(big_number src) {
    big_number tmp;
    if (!sign && src.sign) {
      //printf("- +\n");
      bn_add(src, tmp);
      tmp.sign = NAGETIVE;
    }
    else if (sign && !src.sign) {
      // this - src , src: neg
      //printf("+ -\n");
      bn_add(src, tmp);
      tmp.sign = POSITIVE; 
    }
    else if (!sign && !src.sign){
      //printf("- -\n");
       // this - src, this: neg
      if (bn_cmp(src)) {
        tmp = bn_minus(src);
        tmp.sign = POSITIVE;
      } else {
        tmp = src.bn_minus(*this);
        tmp.sign = NAGETIVE;
      }
    }
    else {
      // this > src
      if (bn_cmp(src)) {
        tmp = bn_minus(src);
      } else {
        tmp = src.bn_minus(*this);
        tmp.sign = NAGETIVE;
      }
    }
    return tmp;
  }
  big_number & operator-=(big_number src) {
    *this = operator-(src);
    return *this;
  }
  void operator--(const int i) {
    bn_dec();
  }

  big_number operator+(big_number src) {
    big_number tmp;
    if (!sign && !src.sign) {
      bn_add(src, tmp);
      tmp.sign = NAGETIVE;
    }
    else if (sign && !src.sign ) {
      // + -
      if (bn_cmp(src)) {
        tmp = bn_minus(src);
        tmp.sign = POSITIVE;
      }
      else {
        tmp = src.bn_minus(*this);
        tmp.sign = NAGETIVE;      
      }
    }
    else if  ( !sign &&  src.sign ) {
      // - +
      if (bn_cmp(src)) {
        tmp = bn_minus(src);
        tmp.sign = NAGETIVE;
      }
      else {
        tmp = src.bn_minus(*this);
        tmp.sign = POSITIVE;      
      }
    }
    else {
      bn_add(src, tmp);
    }
    return tmp;
  }
  big_number & operator+=(big_number src) {
    bn_add(src, *this);
    return *this;
  }
  void operator++(const int i) {
    big_number tmp;
    tmp.bn_from_int(1);
    bn_add(tmp, *this);
  }

  big_number operator*(big_number & rhs) {
    big_number tmp;
    bn_mul(rhs, tmp);
    return tmp;
  }
  big_number & operator*=(big_number & rhs) {
    bn_mul(rhs, *this);
    return *this;
  }

  bool bn_eq(big_number & cmp) {
    for (int i = 0;i < BN_ARRAY_SIZE;++i) {
      UTYPE _a = n.array[i];
      UTYPE _b = cmp.n.array[i];
      if (_a  != _b)
        return false;
    }
    return true;
  }

  int bn_cmp(big_number & cmp) {
    for (int i = 0;i < BN_ARRAY_SIZE;++i) {
      UTYPE _a = n.array[i];
      UTYPE _b = cmp.n.array[i];
      if (_a >= _b) 
        return 1;
      else if (_a < _b)
        return 0;
    }
    return -1;
  }

  void bn_lshift_bit(int t) {
    for (int _t = 0;_t < t;_t++) {
      UTYPE c = 0;
      for (int i = 0;i < BN_ARRAY_SIZE;++i) {
        UTYPE tmp = n.array[i];
        tmp = (tmp << 1) + c;
        n.array[i] = tmp;
        if (tmp > 0 && ((tmp << 1) == 0))
          c = 1;
        else
          c = 0;
      }    
    }
  }

  void _align(big_number & buf, big_number & d) {
    size_t buf_tsize = 0;
    NORMAL_SIZE(buf.n.array, buf_tsize);
    size_t buf_lsb = 64 - __builtin_clzl(buf.n.array[buf_tsize]);
    
    size_t d_tsize = 0;
    NORMAL_SIZE(d.n.array, d_tsize);
    size_t d_lsb = 64 - __builtin_clzl(d.n.array[d_tsize]);
    
    
    if (buf_tsize > d_tsize) {
      d.lshift_unit(buf_tsize - d_tsize);

      if (buf_lsb > d_lsb) {
        d.bn_lshift_bit(buf_lsb - d_lsb);
      } 
      else if (buf_lsb < d_lsb) {
        buf.bn_lshift_bit(d_lsb - buf_lsb);
      }
    }
    else if (buf_tsize < d_tsize) {
      buf.lshift_unit(d_tsize - buf_tsize);
      
      if (buf_lsb > d_lsb) {
        d.bn_lshift_bit(buf_lsb - d_lsb);
      } 
      else if (buf_lsb < d_lsb) {
        buf.bn_lshift_bit(d_lsb - buf_lsb);
      }
    } 
    else {
      if (buf_lsb > d_lsb) {
        d.bn_lshift_bit(buf_lsb - d_lsb);
      } 
      else if (buf_lsb < d_lsb) {
        buf.bn_lshift_bit(d_lsb - buf_lsb);
      }
    }
  }
  /* abandon */
  big_number bn_div(big_number buf, big_number d, big_number tmp_d) {
    big_number q;
    q.bn_from_int(1);

    if (buf.bn_eq(tmp_d)) {
      printf("equal\n");
      q.bn_from_int(1);
      return q;
    }
    if (buf.bn_cmp(tmp_d) == 0) {
      printf("divide product 0\n");
      q.bn_init();
      return q;
    }

    do {
      tmp_d.bn_lshift_bit(1);
      q.bn_lshift_bit(1);
    } while (tmp_d.bn_cmp(buf) == 1);

    big_number tmp = bn_div(buf - tmp_d, d, d);
    q = q + tmp;
    return q;
  }

  uint64_t _bn_div(big_number buf, big_number d) {
    size_t _c = 0;
    while (buf.bn_cmp(d) == 1) {
      buf = buf - d;
      _c++;
    }
    return _c;  
  }

  // a / b = q
  big_number operator/(big_number & rhs) {
    uint64_t tmp = _bn_div(*this, rhs);
    big_number _t;
    _t = tmp;
    return _t;
  }
  big_number & operator/=(big_number & rhs) {
    *this = operator/(rhs);
    return *this;
  } 

  friend std::ostream & operator<<(std::ostream & os, big_number & ta) {
    for (int i = 0;i < 8912;i++)
      ta.strbuf[i] = '\0';
    ta.bn_2_str(ta.strbuf);
    if (ta.sign)
      os << ta.strbuf;
    else 
      os << "-" << ta.strbuf;
    return os;
  }

  void printvalue(void) {
    bn_2_str(strbuf);
    printf("%s", strbuf);
    printf("_10\n");
  }

};

size_t big_number::_to = 0;

class bn_complex {
private :

public :
  big_number _r;
  big_number _c;
  bn_complex()
  {}
  bn_complex(int dummy) :
  _r(1), _c(1)
  {} 
  bn_complex(uint64_t r, uint64_t c) :
  _r(1), _c(1)
  {
    _r.bn_from_int(r);
    _c.bn_from_int(c);
  }

  void release() 
  {
    _r.release();
    _c.release();
  }

  /**
   * operator overloading
   */
  void operator=(bn_complex src) {
    src._r.bn_assign(_r);
    src._c.bn_assign(_c);
  }

  bn_complex operator-(bn_complex src) {
    bn_complex tmp;
    tmp._r = _r - src._r;
    tmp._c = _c - src._c;
    return tmp;
  }
  bn_complex & operator-=(bn_complex & src) {
    *this = operator-(src);
    return *this;
  }

  bn_complex operator+(bn_complex & src) {
    bn_complex tmp;
    tmp._r = _r + src._r;
    tmp._c = _c + src._c;
    return tmp;
  }
  bn_complex & operator+=(bn_complex & src) {
    *this = operator+(src);
    return *this;
  }

  bn_complex operator*(bn_complex & rhs) {
    bn_complex tmp;
    big_number _rt1 = (_r * rhs._r);
    big_number _rt2 = (_c * rhs._c);
    tmp._r = _rt1 - _rt2;

    big_number _ct1 = (_c * rhs._r);
    big_number _ct2 = (_r * rhs._c);
    tmp._c = _ct1 + _ct2;
    return tmp;
  }
  bn_complex & operator*=(bn_complex & rhs) {
    *this = operator*(rhs);
    return *this;
  }

  friend std::ostream & operator<<(std::ostream & os, bn_complex & ta) {
    for (int i = 0;i < 8912;i++) {
      ta._r.strbuf[i] = '\0';
      ta._c.strbuf[i] = '\0';
    }
    ta._r.bn_2_str(ta._r.strbuf);
    ta._c.bn_2_str(ta._c.strbuf);
    os << ta._r.strbuf << " + " << ta._c.strbuf << "i"; 
    return os;
  }

};

#endif