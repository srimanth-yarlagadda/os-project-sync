#include <math.h>
#include <stdio.h>

// this function finds the roots for quadratic function ax^2 + bx + c
void quadratic_solution (int a, int b, int c)
{ 
  double fa = (double) a;
  double fb = (double) b;
  double fc = (double) c;
  double x = fb * fb - 4 * fa * fc;
  double s1 = ( - fb + sqrt (x) ) / (2*fa);
  double s2 = ( - fb - sqrt (x) ) / (2*fa);
  printf ("(%d, %d, %d) solutions = %f, %f\n", a, b, c, s1, s2);

  // int count = 0;
  // if ( s1 == rint(s1) ) count++;
  // if ( s2 == rint(s2) ) count++;
  // printf ("%d integer solutions\n", count);
}

void solve_quadratic (int b, int c)
{ 
  quadratic_solution (1, b, c);
}
