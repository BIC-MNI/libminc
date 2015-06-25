/* ----------------------------- MNI Header -----------------------------------
@NAME       : test
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <minc.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

int main()
{
   int cdf, cdf2;
   int img;
   int dim[MAX_VAR_DIMS];
   int dim2[MAX_VAR_DIMS];
   long start[MAX_VAR_DIMS];
   long count[MAX_VAR_DIMS];
   double image[256*256];
   int i, j, k, ioff;
   char filename[256];
   char filename2[256];

   ncopts=NC_VERBOSE|NC_FATAL;
   snprintf(filename, sizeof(filename), "test_minc-%d.mnc", getpid());
   snprintf(filename2, sizeof(filename2), "test_minc2-%d.mnc", getpid());
   cdf=micreate(filename, NC_CLOBBER);
   count[2]=5;
   count[1]=3;
   count[0]=7;
   dim[2]=ncdimdef(cdf, MIzspace, count[2]);
   dim[1]=ncdimdef(cdf, MIxspace, count[1]);
   dim[0]=ncdimdef(cdf, MIyspace, count[0]);
   dim2[0]=ncdimdef(cdf, MItime, NC_UNLIMITED);
   dim2[1]=dim[0];
   dim2[2]=dim[1];
   img=ncvardef(cdf, MIimage, NC_SHORT, 3, dim);
   (void) ncvardef(cdf, "testvar", NC_FLOAT, 2, dim2);
   (void) miattputstr(cdf, img, MIsigntype, MI_SIGNED);
   for (j=0; j<count[0]; j++) {
      for (i=0; i<count[1]; i++) {
         ioff=(j*count[1]+i)*count[2];
         for (k=0; k<count[2]; k++)
            image[ioff+k]=ioff+k+10;
      }
   }
   cdf2=micreate(filename2,NC_CLOBBER);
   (void) ncdimdef(cdf2, "junkdim", NC_UNLIMITED);
   (void) micopy_all_var_defs(cdf, cdf2, 1, &img);
   (void) ncendef(cdf2);
   (void) ncendef(cdf);
   (void) miset_coords(3,0L,start);
   (void) mivarput(cdf, img, start, count, NC_DOUBLE, NULL, image);

   (void) micopy_all_var_values(cdf, cdf2, 1, &img);
   (void) miclose(cdf2);
   (void) miclose(cdf);
   unlink(filename);
   unlink(filename2);
   return(0);
}
