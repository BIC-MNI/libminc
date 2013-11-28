/* ----------------------------- MNI Header -----------------------------------
@NAME       : minc_format_convert.c
 * $Log: mincconvert.c,v $
 * Revision 1.10  2008-01-17 02:33:02  rotor
 *  * removed all rcsids
 *  * removed a bunch of ^L's that somehow crept in
 *  * removed old (and outdated) BUGS file
 *
 * Revision 1.9  2008/01/12 19:08:15  stever
 * Add __attribute__ ((unused)) to all rcsid variables.
 *
 * Revision 1.8  2007/12/10 13:25:12  rotor
 *  * few more fixes for CMake build
 *  * started adding static to globals for a wierd zlib bug
 *
 * Revision 1.7  2007/08/09 17:05:25  rotor
 *  * added some fixes of Claudes for chunking and internal compression
 *
 * Revision 1.6  2006/07/28 16:49:55  baghdadi
 * added message to disallow conversion to itself and exit with success
 *
 * Revision 1.6  2006/06/21 11:30:00  Leila
 * added return value for main function
 * Revision 1.5  2006/04/10 11:30:00  Leila
 * check the version of file and Abort if converting to itself!
 * Revision 1.4  2005/08/26 21:07:17  bert
 * Use #if rather than #ifdef with MINC2 symbol, and be sure to include config.h whereever MINC2 is used
 *
 * Revision 1.3  2004/11/01 22:38:38  bert
 * Eliminate all references to minc_def.h
 *
 * Revision 1.2  2004/09/09 19:25:32  bert
 * Force V1 file format creation if -2 not specified
 *
 * Revision 1.1  2004/04/27 15:27:57  bert
 * Initial checkin, MINC 1 <-> MINC 2 converter
 *
 * 
@COPYRIGHT  : Copyright 2013 Vladimir S. FONOV , McConnell Brain Imaging Centre,
              Copyright 2003 Robert Vincent, McConnell Brain Imaging Centre, 
              Montreal Neurological Institute, McGill University.
              Permission to use, copy, modify, and distribute this
              software and its documentation for any purpose and without
              fee is hereby granted, provided that the above copyright
              notice appear in all copies.  The author and McGill University
              make no representations about the suitability of this
              software for any purpose.  It is provided "as is" without
              express or implied warranty.
---------------------------------------------------------------------------- */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <minc.h>


static int micopy(int old_fd, int new_fd)
{
    /* Copy all variable definitions (and global attributes).
     */
    micopy_all_var_defs(old_fd, new_fd, 0, NULL);


    ncendef(new_fd);
    micopy_all_var_values(old_fd, new_fd, 0, NULL);
    
    return MI_NOERROR;
}

int minc_format_convert(const char *input,const char *output)
{
    int old_fd;
    int new_fd;
    int flags;
    struct mi2opts opts;
    
    old_fd = miopen(input, NC_NOWRITE);
    if (old_fd < 0) {
        perror(input);
        return EXIT_FAILURE;
    }

    flags = NC_CLOBBER|MI2_CREATE_V2;

    opts.struct_version = MI2_OPTS_V1;

    new_fd = micreatex(output, flags, &opts);
    if (new_fd < 0) {
        perror(output);
        exit(EXIT_FAILURE);
    }

    micopy(old_fd, new_fd);

    miclose(old_fd);
    miclose(new_fd);
    
    return MI_NOERROR;
}
