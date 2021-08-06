#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "factorial.h"

/*
 * factorial(N) := N * (N-1) * (N-2) * ... * 1
 */ 
static void factorial(struct bn *n, struct bn *res) {
    struct bn tmp; 
    bn_assign(&tmp, n);
    bn_dec(n); 
        
    while (!bn_is_zero(n)) {
        bn_mul(&tmp, n, res); /* res = tmp * n */
        bn_dec(n);            /* n -= 1 */
        bn_assign(&tmp, res); /* tmp = res */
    }
    bn_assign(res, &tmp);
}   
    
int main() {   
    struct bn num, result;
    char buf[8192];                        
    bn_from_int(&num, 100);
    factorial(&num, &result);
    bn_to_str(&result, buf, sizeof(buf));
    printf("factorial(100) = %s\n", buf);

    struct bn e;
    bn_init(&e);
    bn_from_int(&e, 9876543210);
    struct bn base;
    bn_init(&base);
    bn_from_int(&base, 10000000000);
    bn_mul(&e, &base, &num);
    printf("array[3] %d array[2] %d array[1] %d array[0] %d\n", num.array[3], num.array[2], num.array[1], num.array[0]);
    char tmp[1000];
    bn_2_str(&num, tmp);
    printf("result: %s\n", tmp);
    return 0;
}
