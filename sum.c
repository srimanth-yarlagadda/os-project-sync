#include <stdio.h>

// compute sum from 1 to n
// integer mod is used to prevent the sum go over max integer

void modular_sum (int n, int mod)
{ int sum, i;

  sum = 0;
  for (i=1; i<=n; i++) sum = (sum + i) % mod;
  printf ("sum from 1 to %d is %d\n", n, sum);
}

