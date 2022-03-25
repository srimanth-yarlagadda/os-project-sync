#include <math.h>
#include <stdio.h>


// this program finds the Pythagorean triples with max values mx and my

int is_square (int x, int y)
{ double tx, ty, tz, tr, r;

  tx = (double) x;
  ty = (double) y;
  tz = tx * tx + ty * ty;
  tr = sqrt (tz);
  
  if ( tr == rint(tr) ) return ((int) tr);
  else return (0);
}

int relatively_prime (int x, int y)
{ int tmp, r;

  if (x < y) {tmp = x; x = y; y = tmp; }
  r = x % y;
  while (r != 0)
  { x = y; y = r;
    r = x % y;
  }
  return (y);
}

void find_triples (int mx, int my)
{ int x, y, r;

  for (x=1; x<=mx; x++) 
    for (y=x; y<=my; y++) 
    { r = is_square (x, y);
      /* if (r == 0)
        printf ("   %d, %d\n", x, y);
      else printf ("-- %d, %d, sqrt = %d\n", x, y, r); */
      if (r != 0)
      { if (relatively_prime (y, x) == 1)
          printf ("-- %d, %d, sqrt = %d\n", x, y, r);
      }
    }
}

