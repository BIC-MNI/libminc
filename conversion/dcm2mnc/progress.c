
// This function prints a text progress bar within a term window
// Input arguments assume a for loop starting at zero:
//
//      for (index =  0; index < end; index++) { ...

static const char rcsid[] = "$Header: /private-cvsroot/minc/conversion/dcm2mnc/progress.c,v 1.2.2.1 2005-05-12 21:16:48 bert Exp $";

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "progress.h"

void
progress(long index, int end, const char *message)
{
    int ix;
    const int width = 50;
    int nchars;

    if (index == 0) {
        printf("%-20.20s |<--", message);
        for (ix = 0; ix < width; ix++) { 
            printf(" ");
        }
        printf("|");
        for (ix = 0; ix < width+1; ix++) { 
            printf("\b");
        }
    } 
    else if ((index > 0) && (index < end)) {

        nchars = (((float)index/(float)(end-1)) * width) - 
            floor(((float)(index-1)/(float)(end-1)) * width);

        for (ix = 0; ix < nchars; ix++) {
            printf("\b->");
            fflush(stdout);
        }

        // print terminating newline at end if we're done
        if (index == end-1) {
            printf("\n");
        }
    } 
    else {
        fprintf(stderr,"PROGRESS:  bad input indices!\n");
    }
}





