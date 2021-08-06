#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "factorial.h"

void bn_to_str(struct bn *n, char *str, int nbytes) {
  /* index into array - reading "MSB" first -> big-endian */
  int j = BN_ARRAY_SIZE - 1;
  int i = 0; /* index into string representation */

  /* reading last array-element "MSB" first -> big endian */
  while ((j >= 0) && (nbytes > (i + 1))) {
    sprintf(&str[i], FORMAT_STRING, n->array[j]);
    i += (2 * UNIT_SIZE); /* step UNIT_SIZE hex-byte(s) forward in the string */
    j -= 1;               /* step one element back in the array */
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
void bn_dec(struct bn *n) {
  for (int i = 0; i < BN_ARRAY_SIZE; ++i) {
    UTYPE tmp = n->array[i];
    UTYPE res = tmp - 1;
    n->array[i] = res;

    // COND;
    if (!(res > tmp))
      break;
  }
}

void bn_add(struct bn *a, struct bn *b, struct bn *c) {
  int carry = 0;
  for (int i = 0; i < BN_ARRAY_SIZE; ++i) {
    UTYPE_TMP tmp = (UTYPE_TMP)a->array[i] + b->array[i] + carry;
    carry = (tmp > MAX_VAL);
    c->array[i] = (tmp & MAX_VAL);
  }
}

void bn_mul(struct bn *a, struct bn *b, struct bn *c) {
  struct bn row, tmp;
  bn_init(c);

  for (int i = 0; i < BN_ARRAY_SIZE; ++i) {
    bn_init(&row);

    for (int j = 0; j < BN_ARRAY_SIZE; ++j) {
      if (i + j < BN_ARRAY_SIZE) {
        bn_init(&tmp);
        // III;
        UTYPE_TMP intermediate = a->array[i] * (UTYPE_TMP)b->array[j];
        bn_from_int(&tmp, intermediate);
        lshift_unit(&tmp, i + j);
        bn_add(&tmp, &row, &row);
      }
    }
    bn_add(c, &row, c);
  }
}

bool bn_is_zero(struct bn *n) {
  for (int i = 0; i < BN_ARRAY_SIZE; ++i)
    if (n->array[i])
      return false;
  return true;
}

/* Copy src into dst. i.e. dst := src */
void bn_assign(struct bn *dst, struct bn *src) {
  for (int i = 0; i < BN_ARRAY_SIZE; ++i)
    dst->array[i] = src->array[i];
}

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



static UTYPE ddivi(struct bn *n, size_t size) {
  size_t tsize = size;
  UTYPE s1 = 0;
  do {
    UTYPE q, r;
    if (s1 == 0) {
      UTYPE s0 = n->array[tsize];
      q = s0 / 10;
      r = s0 % 10;
    } else {
      UTYPE_TMP s0 = s1;
      s0 = s0 << 32;
      s0 += n->array[tsize];
      q = s0 / 10;
      r = s0 % 10;
      //printf("64 s0 %ld\n", s0);
    }
    n->array[tsize] = q;
    s1 = r;
    //printf("s1 %d tsize %ld\n", s1, tsize);
    tsize--;
  } while (tsize != -1);
  return s1;
}

static const char radix_chars[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
void bn_2_str(struct bn *n, char *out) {
  size_t size = 0;
  char *const head_out = out;
  NORMAL_SIZE(n->array, size);
  struct bn _n = *n;
  /*
  for (int i = 0; i < size; i += 2) {
    // multi
    UTYPE_TMP tmp = _n.array[i + 1];
    tmp = tmp << 32;
    tmp += _n.array[i];
    // single
    for (int d = 0; d < 64; d++) {
      UTYPE_TMP q = tmp / 10;
      UTYPE_TMP r = tmp % 10;
      *out++ = radix_chars[r];
      tmp = q;
    }
  }*/

  do {
    // extract
    UTYPE s1 = ddivi(&_n, size);
    //printf("extract %d\n", s1);
    *out++ = radix_chars[s1];
  } while(!bn_is_zero(&_n));


  char *f = out - 1;
  /* Eliminate leading (trailing) zeroes */
  while (*f == '0')
    --f;
  /* NULL terminate */
  f[1] = '\0';
  /* Reverse digits */
  for (char *s = head_out; s < f; ++s, --f)
    SWAP(*s, *f);
  out = head_out;
}