#include <stdio.h>
#include <stdlib.h>
#include "minc2.h"

/* This test will attempt to create a few different test images
 * which can be tested with minc2.0
 */

#define TESTRPT(msg, val) (error_cnt++, fprintf(stderr, \
"Error reported on line #%d, %s: %d\n", \
__LINE__, msg, val))

#define CZ 142
#define CY 245
#define CX 120
#define NDIMS 3


static int create_real_as_int_image(const char* fname)
{
  int r;
  int error_cnt=0;
  double start_values[NDIMS]={-6.96, -12.453,  -9.48};
  double separations[NDIMS]={0.09,0.09,0.09};
  midimhandle_t hdim[NDIMS];
  mihandle_t hvol;
  int *buf = ( int *) malloc(CX * CY * CZ * sizeof(int));
  int i;
  misize_t count[NDIMS];
  misize_t start[NDIMS];
  miboolean_t flag=1;
  
  double min = -1.0;
  double max =  1.0;
  /*TODO: add error checks in this functions*/
  r = micreate_dimension("yspace", MI_DIMCLASS_SPATIAL,
                         MI_DIMATTR_REGULARLY_SAMPLED, CY, &hdim[0]);
  
  if( r!= MI_NOERROR )    TESTRPT("micreate_dimension",r);
  
  r = micreate_dimension("xspace", MI_DIMCLASS_SPATIAL,
                         MI_DIMATTR_REGULARLY_SAMPLED, CX, &hdim[1]);
  if( r!= MI_NOERROR )    TESTRPT("micreate_dimension",r);
  
  r = micreate_dimension("zspace", MI_DIMCLASS_SPATIAL,
                         MI_DIMATTR_REGULARLY_SAMPLED, CZ, &hdim[2]);
  if( r!= MI_NOERROR )    TESTRPT("micreate_dimension",r);
  
  r = miset_dimension_starts(hdim, NDIMS, start_values);
  if( r!= MI_NOERROR )    TESTRPT("miset_dimension_starts",r);
  
  r = miset_dimension_separations(hdim, NDIMS, separations);
  if( r!= MI_NOERROR )    TESTRPT("miset_dimension_separations",r);
  
  r = micreate_volume(fname, NDIMS, hdim, MI_TYPE_INT,
                      MI_CLASS_REAL, NULL, &hvol);
  if( r!= MI_NOERROR )    TESTRPT("micreate_volume",r);
  
  /* set slice scaling flag to true */
  r = miset_slice_scaling_flag(hvol, flag);
  if( r!= MI_NOERROR )    TESTRPT("miset_slice_scaling_flag",r);
  
  r = micreate_volume_image(hvol);
  if( r!= MI_NOERROR )    TESTRPT("micreate_volume_image",r);
  
  for (i = 0; i < CY*CX*CZ; i++) {
    buf[i] = (int) (i * 0.001);
  }
  
  start[0] = start[1] = start[2] = 0;
  count[0] = CY; count[1] = CX; count[2] = CZ;
  
  r = miset_voxel_value_hyperslab(hvol, MI_TYPE_INT, start, count, buf);
  if( r!= MI_NOERROR )    TESTRPT("miset_voxel_value_hyperslab",r);
  /* Set random values to slice min and max for slice scaling*/
  start[0] =start[1]=start[2]=0;
  for (i=0; i < CY; i++) {
    start[0] = i;
    min += -0.1;
    max += 0.1;
    r = miset_slice_range(hvol,start,NDIMS , max, min);
    if(r<0)
    {
      if( r!= MI_NOERROR )    TESTRPT("miset_slice_range",r);
      return r;
    }
  }
  
  r = miclose_volume(hvol);
  if( r!= MI_NOERROR )    TESTRPT("miclose_volume",r);
  free(buf);
  return error_cnt;
}


static int create_real_as_float_image(const char* fname)
{
  int r;
  int error_cnt=0;
  
  double start_values[NDIMS]={-6.96, -12.453,  -9.48};
  double separations[NDIMS]={0.09,0.09,0.09};
  midimhandle_t hdim[NDIMS];
  mihandle_t hvol;
  float *buf = (float *) malloc(CX * CY * CZ * sizeof(float));
  int i;
  misize_t count[NDIMS];
  misize_t start[NDIMS];
  miboolean_t flag=1;
  
  double min = -1.0;
  double max =  1.0;
  r = micreate_dimension("yspace", MI_DIMCLASS_SPATIAL,
                         MI_DIMATTR_REGULARLY_SAMPLED, CY, &hdim[0]);
  if( r!= MI_NOERROR )    TESTRPT("micreate_dimension",r);
  
  r = micreate_dimension("xspace", MI_DIMCLASS_SPATIAL,
                         MI_DIMATTR_REGULARLY_SAMPLED, CX, &hdim[1]);
  if( r!= MI_NOERROR )    TESTRPT("micreate_dimension",r);
  
  r = micreate_dimension("zspace", MI_DIMCLASS_SPATIAL,
                         MI_DIMATTR_REGULARLY_SAMPLED, CZ, &hdim[2]);
  if( r!= MI_NOERROR )    TESTRPT("micreate_dimension",r);
  
  r = miset_dimension_starts(hdim, NDIMS, start_values);
  if( r!= MI_NOERROR )    TESTRPT("miset_dimension_starts",r);
  
  r = miset_dimension_separations(hdim, NDIMS, separations);
  if( r!= MI_NOERROR )    TESTRPT("miset_dimension_separations",r);
  
  r = micreate_volume(fname, NDIMS, hdim, MI_TYPE_FLOAT,
                      MI_CLASS_REAL, NULL, &hvol);
  if( r!= MI_NOERROR )    TESTRPT("micreate_volume",r);
  
  /* set slice scaling flag to true */
  r = miset_slice_scaling_flag(hvol, flag);
  if( r!= MI_NOERROR )    TESTRPT("miset_slice_scaling_flag",r);
  
  r = micreate_volume_image(hvol);
  if( r!= MI_NOERROR )    TESTRPT("micreate_volume_image",r);
  
  for (i = 0; i < CY*CX*CZ; i++) {
    buf[i] =  i * 0.001f;
  }
  
  start[0] = start[1] = start[2] = 0;
  count[0] = CY; count[1] = CX; count[2] = CZ;
  
  r = miset_voxel_value_hyperslab(hvol, MI_TYPE_FLOAT, start, count, buf);
  if( r!= MI_NOERROR )    TESTRPT("miset_voxel_value_hyperslab",r);
  /* Set random values to slice min and max for slice scaling*/
  start[0] =start[1]=start[2]=0;
  for (i=0; i < CY; i++) {
    start[0] = i;
    min += -0.1;
    max += 0.1;
    r = miset_slice_range(hvol,start,NDIMS , max, min);
    if( r!= MI_NOERROR )    TESTRPT("miset_slice_range",r);
  }
  
  r = miclose_volume(hvol);
  if( r!= MI_NOERROR )    TESTRPT("miclose_volume",r);
  free(buf);
  return error_cnt;
}

int main(int argc, char **argv)
{
  int r = 0;
  
  if(argc< 3)
  {
    fprintf(stderr,"Usage:%s <out int> <out float>\n",argv[0]);
    return 1;
  }
  
  printf("Creating 3D image REAL stored as INT w/ slice scaling!!\n");
  r +=create_real_as_int_image(argv[1]);
  printf("Creating 3D image REAL stored as FLOAT w/ slice scaling!!\n");
  r +=create_real_as_float_image(argv[2]);
  
  if (r != 0) {
    fprintf(stderr, "%d error%s reported\n",
            r, (r == 1) ? "" : "s");
  }
  else {
    fprintf(stderr, "\n No errors\n");
  }
  
  return (r);
}

/* kate: indent-mode cstyle; indent-width 2; replace-tabs on; */
