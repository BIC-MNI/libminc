#include <stdio.h>
#include <stdlib.h>
#include "minc2.h"

#define CZ 142
#define CY 245
#define CX 100
#define CU 5
#define NDIMS 3


int main ( int argc, char **argv )
{
  mihandle_t hvol;
  int r;
  misize_t coords[3];
  double min, max;
  int i;

  while ( --argc > 0 ) {
    const char *fname=*++argv;
    printf("Checking %s\n",fname);
    r = miopen_volume ( fname, MI2_OPEN_READ, &hvol );
    if ( r < 0 ) {
      fprintf ( stderr, "can't open %s, error %d\n", *argv, r );
      return 1;
    } else {
      for ( i = 0; i < CZ; i++ ) {
        coords[0] = i;         /*Z*/
        coords[1] = rand()%CX; /*X*/
        coords[2] = rand()%CY; /*Y*/

        r = miget_slice_min ( hvol, coords, 3, &min );
        if ( r < 0 ) {
          fprintf ( stderr, "error %d getting slice minimum at %d %d %d\n", r, (int)coords[0],(int)coords[1],(int)coords[2] );
          return 1;
        }

        r = miget_slice_max ( hvol, coords, 3, &max );
        if ( r < 0 ) {
          fprintf ( stderr, "error %d getting slice maximum at %d %d %d\n", r,(int)coords[0],(int)coords[1],(int)coords[2] );
          return 1;
        }
        /*printf ( "%d. min %f max %f\n", i, min, max );*/
        /*PASS*/
      }
      miclose_volume ( hvol );
    }
  }
  return ( 0 );
}


/* kate: indent-mode cstyle; indent-width 2; replace-tabs on; */
