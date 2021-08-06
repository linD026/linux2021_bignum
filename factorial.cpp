#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "factorial_cpp.h"

void big_number::bn_to_str(char *str, int nbytes) {
    /* index into array - reading "MSB" first -> big-endian */
    int j = BN_ARRAY_SIZE - 1;
    int i = 0; /* index into string representation */

    /* reading last array-element "MSB" first -> big endian */
    while ((j >= 0) && (nbytes > (i + 1))) {
        sprintf(&str[i], FORMAT_STRING, n.array[j]);
        i += (2 *
              UNIT_SIZE); /* step UNIT_SIZE hex-byte(s) forward in the string */
        j -= 1;           /* step one element back in the array */
    }

    /* Count leading zeros: */
    for (j = 0; str[j] == '0'; j++)
        ;

    /* Move string j places ahead, effectively skipping leading zeros */
    for (i = 0; i < (nbytes - j); ++i)
        str[i] = str[i + j];

    str[i] = 0;
}

/* Decrement: subtract 1 from n */
void big_number::bn_dec(void) {
    for (int i = 0; i < BN_ARRAY_SIZE; ++i) {
        UTYPE tmp = n.array[i];
        UTYPE res = tmp - 1;
        n.array[i] = res;
        // COND;
        if (!(res > tmp)) break;
    }
}

void big_number::bn_add(big_number b, big_number &c) {
    int carry = 0;
    for (int i = 0; i < BN_ARRAY_SIZE; ++i) {
        UTYPE_TMP tmp = (UTYPE_TMP) this->n.array[i] + b.n.array[i] + carry;
        carry = (tmp > MAX_VAL);
        c.n.array[i] = (tmp & MAX_VAL);
    }
}

void big_number::bn_mul(big_number b, big_number &c) {
    big_number row, tmp;
    c.bn_init();

    for (int i = 0; i < BN_ARRAY_SIZE; ++i) {
        row.bn_init();

        for (int j = 0; j < BN_ARRAY_SIZE; ++j) {
            if (i + j < BN_ARRAY_SIZE) {
                tmp.bn_init();
                UTYPE_TMP intermediate = this->n.array[i] * (UTYPE_TMP) b.n.array[j];
                tmp.bn_from_int(intermediate);
                tmp.lshift_unit(i + j);
                tmp.bn_add(row, row);
            }
        }
        c.bn_add(row, c);
    }
}
bool big_number::bn_is_zero(void) {
    for (int i = 0; i < BN_ARRAY_SIZE; ++i)
        if (n.array[i])
            return false;
    return true;
}

/* Copy src into dst. i.e. dst := src */
void big_number::bn_assign(big_number & dst) {
    for (int i = 0; i < BN_ARRAY_SIZE; ++i)
        dst.n.array[i] = n.array[i];
}

UTYPE big_number::ddivi(big_number &_n) {
  size_t tsize = 0;
  NORMAL_SIZE(n.array, tsize);
  UTYPE s1 = 0;
  while (tsize != -1) {
    UTYPE q, r;
    if (s1 == 0) {
      UTYPE s0 = _n.n.array[tsize];
      q = s0 / 10;
      r = s0 % 10;
    } else {
      UTYPE_TMP s0 = s1;
      s0 = s0 << 32;
      s0 += _n.n.array[tsize];
      q = s0 / 10;
      r = s0 % 10;
      //printf("64 s0 %ld\n", s0);
    }
    _n.n.array[tsize] = q;
    s1 = r;
    //printf("s1 %d tsize %ld\n", s1, tsize);
    tsize--;
  }
  return s1;
}

void big_number::bn_2_str(char *out) {
  size_t size = 0;
  char *const head_out = out;
  big_number _n = *this;
  do {
    // extract
    UTYPE s1 = ddivi(_n);
    // printf("extract %d\n", s1);
    *out++ = radix_chars[s1];
  } while(!_n.bn_is_zero());

  char *f = out - 1;
  /* Eliminate leading (trailing) zeroes */
  while (*f == '0') {
    --f;
  }
  /* NULL terminate */
  f[1] = '\0';
  /* Reverse digits */
  for (char *s = head_out; s < f; ++s, --f)
    SWAP(*s, *f);
  out = head_out;
}




#define print(src) std::cout << #src << ": " << src << std::endl
#define test_print(src) std::cout << "\n" << src << std::endl
int main() {

    test_print("< big_number x assigned 10000000000 (integer type) >");
    big_number x(1);
    x = 10000000000;
    print(x);

    test_print("< x + y , 10000000000 + 10000000000 >");
    big_number y(1);
    y = 10000000000;
    x = x + y;
    print(x);

    test_print("< x - y , 10000000000 - 1 >");
    x = 10000000000;
    y = 1;
    print(x);
    print(y);
    x = x - y;
    print(x);

    test_print("< x * y , 20000000000 * 10000000000 >");
    x = 20000000000;
    y = 10000000000;
    print(x);
    print(y);
    x = x * y;
    print(x);

    test_print("< x / y , 15 / 2 >");
    x = 15;
    y = 2;
    print(x);
    print(y);
    x = x / y;
    print(x);

    test_print("< negative x - y >");
    x = 10;
    y = 11;
    x.pos_to_neg();
    print(x);
    print(y);
    x = x - y;
    print(x);

    test_print("< negative y - x >");
    x = 11;
    y = 5;
    print(x);
    print(y);
    x = y - x;    
    print(x);

    test_print("< negative x + y >");
    x = 11;
    y = 5;
    x.pos_to_neg();
    print(x);
    print(y);
    x = x + y;    
    print(x);

    x.release();
    y.release();


    test_print("<complex : a = 6 + 4i>");
    bn_complex a(6, 4);
    print(a);

    test_print("<complex : a + b >");
    bn_complex b(3, 2);
    print(a);
    print(b);
    a = a + b;
    print(a);

    test_print("<complex : a - b >");
    print(a);
    print(b);
    a = a - b;
    print(a);

    test_print("<complex : a * b >");
    print(a);
    print(b);
    a = a * b;
    print(a);

    a.release();
    b.release();

    return 0;
}