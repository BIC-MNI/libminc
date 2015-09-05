#if HAVE_CONFIG_H
#include "config.h"
#endif

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
   char filename[256];
   int cdfid;
   int dim[MAX_VAR_DIMS];
   snprintf(filename, sizeof(filename), "test_mconv-%d.mnc", getpid());
   cdfid=nccreate(filename, NC_CLOBBER);
   dim[0]=ncdimdef(cdfid, MIzspace, 3L);
   dim[1]=ncdimdef(cdfid, MIyspace, 5L);
   dim[2]=ncdimdef(cdfid, MIxspace, 6L);
   (void) micreate_std_variable(cdfid, MIimage, NC_SHORT, 3, dim);
   (void) micreate_std_variable(cdfid, MIimagemax, NC_DOUBLE, 1, dim);
   (void) micreate_std_variable(cdfid, MIimagemin, NC_DOUBLE, 1, dim);
   (void) micreate_std_variable(cdfid, MIzspace, NC_DOUBLE, 1, NULL);
   (void) micreate_std_variable(cdfid, MIzspace_width, NC_DOUBLE, 1, &dim[0]);
   (void) micreate_group_variable(cdfid, MIpatient);
   (void) micreate_group_variable(cdfid, MIstudy);
   (void) micreate_group_variable(cdfid, MIacquisition);
   (void) ncclose(cdfid);

   unlink(filename);

   return 0;
}
