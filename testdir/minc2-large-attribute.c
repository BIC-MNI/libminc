#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minc2.h"

/* This test will attempt to create a few different test images
 * which can be tested with minc2.0
*/

static int error_cnt=0;

#define TESTRPT(msg, val) (error_cnt++, fprintf(stderr, \
                           "Error reported on line #%d, %s: %d\n", \
                           __LINE__, msg, val))

#define CZ 142
#define CY 245
#define CX 10
#define CU 5
#define NDIMS 3


static int create_3D_image ( size_t attribute_size,char *test_file )
{
  int r;
  double start_values[NDIMS] = { -6.96, -12.453,  -9.48};
  double separations[NDIMS] = {0.09, 0.09, 0.09};
  midimhandle_t hdim[NDIMS];
  mihandle_t hvol;
  unsigned short *buf = NULL;
  int i;
  misize_t count[NDIMS];
  misize_t start[NDIMS];
  miboolean_t flag = 1;
  char *attribute;

  double min = -1.0;
  double max =  1.0;
  r = micreate_dimension ( "yspace", MI_DIMCLASS_SPATIAL,
                           MI_DIMATTR_REGULARLY_SAMPLED, CY, &hdim[0] );
  if(r<0) return r;
  
  r = micreate_dimension ( "xspace", MI_DIMCLASS_SPATIAL,
                           MI_DIMATTR_REGULARLY_SAMPLED, CX, &hdim[1] );
  if(r<0) return r;
  
  r = micreate_dimension ( "zspace", MI_DIMCLASS_SPATIAL,
                           MI_DIMATTR_REGULARLY_SAMPLED, CZ, &hdim[2] );
  if(r<0) return r;
  
  r = miset_dimension_starts ( hdim, NDIMS, start_values );
  if(r<0) return r;
  
  r = miset_dimension_separations ( hdim, NDIMS, separations );
  if(r<0) return r;
  
  r = micreate_volume ( test_file, NDIMS, hdim, MI_TYPE_USHORT,
                        MI_CLASS_REAL, NULL, &hvol );
  if(r<0) return r;
  
  /* set slice scaling flag to true */
  r = miset_slice_scaling_flag ( hvol, flag );
  if(r<0) return r;
  

  r = micreate_volume_image ( hvol );
  if(r<0) return r;
  

  buf = ( unsigned short * ) malloc ( CX * CY * CZ * sizeof ( unsigned short ) );
  for ( i = 0; i < CY * CX * CZ; i++ ) {
    buf[i] = ( unsigned short ) (i * 0.001);
  }

  start[0] = start[1] = start[2] = 0;
  count[0] = CY; count[1] = CX; count[2] = CZ;

  r = miset_voxel_value_hyperslab ( hvol, MI_TYPE_USHORT, start, count, buf );
  if(r<0) return r;
  
  /* Set random values to slice min and max for slice scaling*/
  start[0] = start[1] = start[2] = 0;
  for ( i = 0; i < CY; i++ ) {
    start[0] = i;
    min += 0.1;
    max += 0.1;
    r = miset_slice_range ( hvol, start, NDIMS , max, min );
    if(r<0) return r;
  }
  attribute=malloc(attribute_size);
  memset(attribute,'Z',attribute_size-1);
  attribute[attribute_size-1]=0;
  
  miset_attr_values(hvol,MI_TYPE_STRING,"test", "test",  attribute_size, attribute);
  miset_attr_values(hvol,MI_TYPE_STRING,"test", "test2", 6, "test2");
  
  r = miclose_volume ( hvol );
  free(buf);
  free(attribute);
  return r;
}


static int test_3D_image ( size_t attribute_size,char *test_file )
{
  int r;
  mihandle_t hvol;
  unsigned char *buf = NULL;
  size_t length;
  int wrong_value=0;
  mitype_t att_type;
  r = miopen_volume ( test_file, MI2_OPEN_READ, &hvol );

  if ( r < 0 ) {
    TESTRPT ( "failed to open image", r );
    /*nothing else to do here*/
    return ( error_cnt );
  }

  r = miget_attr_type(hvol, "test", "test", &att_type);
  if (r < 0) {
    TESTRPT("miget_attr_type failed", r);
  }

  if(att_type != MI_TYPE_STRING)
    TESTRPT("Attribute has unexpected type ", r);

  r = miget_attr_length(hvol, "test", "test", &length);
  if (r < 0) {
    TESTRPT("miget_attr_length failed", r);
  }

  if(length != attribute_size)
    TESTRPT("Attribute has unexpected length ", r);

  buf = calloc(length, sizeof(char));
  r = miget_attr_values(hvol, MI_TYPE_STRING,  "test", "test", length, buf);
  if (r < 0) {
    TESTRPT("miget_attr_values failed", r);
  }
  if(buf[length-1]!=0)
    TESTRPT("Attribute is not null terminated", r);
  
  for(size_t i=0;i<(length-1);++i)
  {
    if(buf[i]!='Z')
      wrong_value++;
  }

  if(wrong_value>0)
    TESTRPT("Attribute has wrong values", wrong_value);

  free(buf);
  r = miclose_volume ( hvol );
  return r;
}



int main ( int argc, char **argv )
{
  int attribute_size=100000;
  char *test_file="3D_image_a.mnc";
  if(argc>1)
    attribute_size=atoi(argv[1]);
  if(argc>2)
    test_file=argv[2];
  
  printf ( "Creating 3D image with attribute %d ! (3D_image_a.mnc)\n", attribute_size );
  if( create_3D_image(attribute_size, test_file)<0)
    TESTRPT("create_3D_image",0);

  if( test_3D_image(attribute_size, test_file)<0)
    TESTRPT("test_3D_image",0);
  
  if ( error_cnt != 0 ) {
    fprintf ( stderr, "%d error%s reported\n",
              error_cnt, ( error_cnt == 1 ) ? "" : "s" );
  } else {
    fprintf ( stderr, "\n No errors\n" );
  }

  return ( error_cnt );
}

