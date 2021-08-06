#ifndef __FACTORIAL_H_
#define __FACTORIAL_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* how large the underlying array size should be */
#define UNIT_SIZE 4

/* These are dedicated to UNIT_SIZE == 4 */
#define UTYPE uint32_t
#define UTYPE_TMP uint64_t
#define FORMAT_STRING "%.08x"
#define MAX_VAL ((UTYPE_TMP)0xFFFFFFFF)

#define RADIX_10_DIGIT 0x3B9ACA00U

#define BN_ARRAY_SIZE (128 / UNIT_SIZE) /* size of big-numbers in bytes */

/* bn consists of array of TYPE */
struct bn {
  UTYPE array[BN_ARRAY_SIZE];
};

static inline void bn_init(struct bn *n) {
  for (int i = 0; i < BN_ARRAY_SIZE; ++i)
    n->array[i] = 0;
}

static inline void bn_from_int(struct bn *n, UTYPE_TMP i) {
  bn_init(n);
  /* FIXME: what if machine is not little-endian? */
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

  n->array[start] = i;
  /* bit-shift with U64 operands to force 64-bit results */
  UTYPE_TMP tmp = i >> 32;
  n->array[next] = tmp;
}

void bn_to_str(struct bn *n, char *str, int nbytes);
void bn_dec(struct bn *n);
void bn_add(struct bn *a, struct bn *b, struct bn *c);

static inline void lshift_unit(struct bn *a, int n_units) {
  int i;
  /* Shift whole units */
  for (i = (BN_ARRAY_SIZE - 1); i >= n_units; --i)
    a->array[i] = a->array[i - n_units];
  /* Zero pad shifted units */
  for (; i >= 0; --i)
    a->array[i] = 0;
}

void bn_mul(struct bn *a, struct bn *b, struct bn *c);
bool bn_is_zero(struct bn *n);
void bn_assign(struct bn *dst, struct bn *src);
void radix10_to_str(struct bn *n, char *out);
void bn_2_str(struct bn *n, char *out);
#endif