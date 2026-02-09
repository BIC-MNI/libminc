#define _GNU_SOURCE 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/


#include <stdio.h>
#include <stdlib.h>
#include <volume_io.h>

int main( int ac, char* av[] )
{
    VIO_General_transform xfm;

    if ( ac != 3 ) {
      fprintf( stderr, "usage: %s in.xfm out.xfm %d\n", av[0],ac );
      return 1;
    }

    if ( input_transform_file( av[1], &xfm ) != VIO_OK ) {
      fprintf( stderr, "Failed to load transform '%s'\n", av[1] );
      return 2;
    }

    if ( output_transform_file( av[2], "created by copy-xfm",&xfm ) != VIO_OK ) {
      fprintf( stderr, "Failed to save transform '%s'\n", av[2] );
      return 2;
    }

    delete_general_transform( &xfm );

    return 0;
}

