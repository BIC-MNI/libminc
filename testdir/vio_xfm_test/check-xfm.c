#define _GNU_SOURCE 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/


#include <stdio.h>
#include <stdlib.h>

#include <volume_io.h>

 
static VIO_Real tolerance = 1e-8;


static int is_equal_real( VIO_Real e, VIO_Real a )
{
    return fabs(e-a) < tolerance;
}

/* Args: expected, actual.
 */
static void assert_equal_point( 
       VIO_Real ex, VIO_Real ey, VIO_Real ez,
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
    FILE *in;
    int line=1;


    if ( ac != 4 ) {
      fprintf( stderr, "usage: %s transform.xfm control_table.txt tolerance\n", av[0] );
      return 1;
    }

    if ( input_transform_file( av[1], &xfm ) != VIO_OK ) {
      fprintf( stderr, "Failed to load transform '%s'\n", av[1] );
      return 2;
    }

    tolerance = atof( av[3] );

    if(!(in=fopen(av[2],"r")))
    {
      fprintf( stderr, "Failed to load table '%s'\n", av[2] );
      return 2;
    }
      
    /*Set the same seed number*/
    while (!feof(in)) {
      VIO_Real x,y,z;
      VIO_Real tx,ty,tz;
      VIO_Real a,b,c;
      int check=1;
      char line_c[1024];
      
      check=fscanf(in,"%lg,%lg,%lg,%lg,%lg,%lg",&x,&y,&z,&a,&b,&c);
      if(check<=0) break;
      
      if(check!=3 && check!=6)
      {
        fprintf( stderr,"Unexpected input file format at line %d , read %d values!\n",line,check);
        return 3;
      }

      if(general_transform_point( &xfm,  x,y,z,  &tx,&ty,&tz ) != VIO_OK)
      {
        fprintf( stderr, "Failed to transform point %f,%f,%f \n", x,y,z );
        return 3;
      }

      if(check==3)
      {
        fprintf( stdout,"%.20lg,%.20lg,%.20lg,%.20lg,%.20lg,%.20lg\n",x,y,z,tx,ty,tz);

      } else {
        sprintf(line_c,"Line:%d",line);
        assert_equal_point( tx,ty,tz, a,b,c,
              line_c );
      }

      line++;
      fgetc(in);
    }

    fclose(in);

    return 0;
}

