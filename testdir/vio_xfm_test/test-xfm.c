#define _GNU_SOURCE 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/


#include <stdio.h>
#include <stdlib.h>

#include <volume_io.h>


static VIO_Real tolerance = 1e-8;

/*Windows compatibility hack*/
#ifndef HAVE_SRAND48
void srand48(long seed)
{
  srand((unsigned int)seed);
}
#endif /*HAVE_SRAND48*/

#ifndef HAVE_DRAND48
double drand48(void)
{
  return (double)rand() / ( + 1);
}
#endif /*HAVE_DRAND48*/



static int is_equal_real( VIO_Real e, VIO_Real a )
{
    return fabs(e-a) < tolerance;
}



/* Args: expected, actual.
 */
static void assert_equal_point( VIO_Real ex, VIO_Real ey, VIO_Real ez,
			 VIO_Real ax, VIO_Real ay, VIO_Real az,
			 const char* msg )
{
    if ( is_equal_real(ex,ax) &&
	 is_equal_real(ey,ay) &&
	 is_equal_real(ez,az) )
	return;

    printf( "%s failure.\n"
	    "Expected: %f %f %f\n"
	    "  Actual: %f %f %f\n",
	    msg, ex,ey,ez,  ax,ay,az );

    exit(3);
}



int main( int ac, char* av[] )
{
    int N;
    VIO_General_transform xfm;


    if ( ac != 3 && ac != 4 ) {
      fprintf( stderr, "usage: %s N transform.xfm [tolerance]\n", av[0] );
      return 1;
    }

    N = atoi( av[1] );
    if ( input_transform_file( av[2], &xfm ) != VIO_OK ) {
      fprintf( stderr, "Failed to load transform '%s'\n", av[2] );
      return 2;
    }

    if ( ac == 4 ) {
      tolerance = atof( av[3] );
      printf( "Setting tolerance to %f.\n", tolerance );
    }
    /*Set the same seed number*/
    srand48(1);
    while (N-- > 0) {
      VIO_Real x = 500.0 * ( drand48() - 0.5 );
      VIO_Real y = 500.0 * ( drand48() - 0.5 );
      VIO_Real z = 500.0 * ( drand48() - 0.5 );

      VIO_Real tx,ty,tz;
      VIO_Real a,b,c;

      if(general_transform_point( &xfm,  x,y,z,  &tx,&ty,&tz ) != VIO_OK)
      {
        fprintf( stderr, "Failed to transform point %f,%f,%f \n", x,y,z );
        return 3;
      }

      /* Check that general_inverse_transform_point() and
        invert_general_transform() behave sensibly.
      */
      if(general_inverse_transform_point( &xfm,  tx,ty,tz,  &a,&b,&c ) != VIO_OK)
      {
        fprintf( stderr, "Failed to invert transform point %f,%f,%f \n", tx,ty,tz );
        return 3;
      }
      assert_equal_point( x,y,z, a,b,c,
              "general_inverse_transform_point()" );

      invert_general_transform( &xfm );

      if(general_transform_point( &xfm, tx,ty,tz,  &a,&b,&c ) != VIO_OK)
      {
        fprintf( stderr, "Failed to transform point %f,%f,%f \n", x,y,z );
        return 3;
      }

      assert_equal_point( x,y,z, a,b,c,
              "general_transform_point() / inverted xfm" );

      if(general_inverse_transform_point( &xfm,  x,y,z,  &a,&b,&c ) != VIO_OK)
      {
        fprintf( stderr, "Failed to invert transform point %f,%f,%f \n", x,y,z );
        return 3;
      }

      assert_equal_point( tx,ty,tz, a,b,c,
              "general_inverse_transform_point() / inverted xfm" );
    }

    delete_general_transform( &xfm );
    return 0;
}

