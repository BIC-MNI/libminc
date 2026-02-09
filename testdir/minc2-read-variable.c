#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minc2.h"

#define TESTRPT(msg, val) (error_cnt++, fprintf(stderr, \
                           "Error reported on line #%d, %s: %d\n", \
                           __LINE__, msg, val))

static int error_cnt = 0;


static const char * get_type_name(mitype_t volume_type)
{
  return volume_type==MI_TYPE_BYTE?"Byte":
         volume_type==MI_TYPE_SHORT?"Short":
         volume_type==MI_TYPE_INT?"Int":
         volume_type==MI_TYPE_FLOAT?"Float":
         volume_type==MI_TYPE_DOUBLE?"Double":
         volume_type==MI_TYPE_STRING?"String":
         volume_type==MI_TYPE_UBYTE?"Unsigned Byte":
         volume_type==MI_TYPE_USHORT?"Unsigned Short":
         volume_type==MI_TYPE_UINT?"Unsigned Int":"Other";
}


int main ( int argc, char **argv )
{
  mihandle_t vol;
 /*
 Test dataset:
  make_phantom  -width 1 1 1 -byte -voxel_range 0 1 -center 0 0 0 -nelements 3 3 3 -step 1 1 1 -start -1-1 -1 dot.mnc
  mincconcat -concat_dimension time -coordlist 0,1,10,100,200 -widthlist 0.25,0.5,1,2,4 dot.mnc dot.mnc dot.mnc dot.mnc dot.mnc 4d_dot.mnc
 */
  
  int           ndim;
  miclass_t      volume_class;
  mitype_t       volume_type;
  mitype_t       time_var_type;
  double         *time_steps = NULL;
  hsize_t        dim_sizes[MI2_MAX_VAR_DIMS];
  hsize_t        starts[]={0};
  hsize_t        counts[]={5};
  int r = 0;
  
  if(argc<2)
  {
    fprintf(stderr,"Usage: %s <input.mnc> \n",argv[0]);
    return 1;
  }
  
  r = miopen_volume ( argv[1], MI2_OPEN_READ, &vol );

  if ( r < 0 ) {
    TESTRPT ( "failed to open image", r );
    /*nothing else to do here*/
    error_cnt++;
    return ( error_cnt );
  }

  if ( r < 0 ) {
    error_cnt++;
    TESTRPT ( "failed to get image dimension count", r );
  }

  /* check the time dimensions variable*/
  ndim = miget_variable_ndims(vol,  "dimensions", "time");
  if ( ndim < 0 ) {
    error_cnt++;
    TESTRPT ( "failed to get variable dimensions type", r );
  }
  
  if(ndim!=1)
  {
    error_cnt++;
    TESTRPT ( "unexpected number of dimensions for time variable", ndim );
  }

  r = miget_variable_dims(vol,  "dimensions", "time", dim_sizes);
  if ( r < 0 ) {
    error_cnt++;
    TESTRPT ( "failed to get time variable sizes", r );
  }

  if(dim_sizes[0]!=5)
  {
      error_cnt++;
      TESTRPT ( "unexpected size for time variable", (int)dim_sizes[0] );
  }

  r=miget_variable_type(vol, "dimensions", "time", &time_var_type);
  if ( r < 0 ) {
      error_cnt++;
      TESTRPT ( "failed to get time variable type", r );
  }

  if(time_var_type!=MI_TYPE_DOUBLE)
  {
      error_cnt++;
      TESTRPT ( "unexpected type for time variable", time_var_type );
  }

  time_steps=malloc(dim_sizes[0]*sizeof(double));
  if(!time_steps)
  {
      error_cnt++;
      TESTRPT ( "failed to allocate memory for time steps", (int)(dim_sizes[0]*sizeof(double)) );
  }   

  r=miget_variable_raw(vol, "dimensions", "time", MI_TYPE_DOUBLE, starts, counts, time_steps);
  if ( r < 0 ) {
      error_cnt++;
      TESTRPT ( "failed to get time variable values", r );
  }

  printf("\tTime steps: ");
  {
      int i;
      for(i=0;i<dim_sizes[0];i++)
          printf("%f ",time_steps[i]);
      printf("\n");
  }

  if(time_steps[0]!=0.0 || time_steps[1]!=1.0 || time_steps[2]!=10.0 ||
     time_steps[3]!=100.0 || time_steps[4]!=200.0)
  {
      error_cnt++;
      TESTRPT ( "unexpected values for time variable", 0 );
  }

  free(time_steps);
  time_steps=NULL;

  /* Now deal with time widths */

  /* check the time dimensions variable*/
  ndim = miget_variable_ndims(vol,  "info", "time-width");
  if ( ndim < 0 ) {
    error_cnt++;
    TESTRPT ( "failed to get time-width dimensions number", r );
  }
  
  if(ndim!=1)
  {
    error_cnt++;
    TESTRPT ( "unexpected number of dimensions for time-width variable", ndim );
  }

  r = miget_variable_dims(vol,  "info", "time-width", dim_sizes);
  if ( r < 0 ) {
    error_cnt++;
    TESTRPT ( "failed to get time-width variable sizes", r );
  }

  if(dim_sizes[0]!=5)
  {
      error_cnt++;
      TESTRPT ( "unexpected size for time-width variable", (int)dim_sizes[0] );
  }

  r=miget_variable_type(vol, "info", "time-width", &time_var_type);
  if ( r < 0 ) {
      error_cnt++;
      TESTRPT ( "failed to get time-width variable type", r );
  }

  if(time_var_type != MI_TYPE_DOUBLE)
  {
      error_cnt++;
      TESTRPT ( "unexpected type for time-width variable", time_var_type );
  }

  time_steps=malloc(dim_sizes[0]*sizeof(double));
  if(!time_steps)
  {
      error_cnt++;
      TESTRPT ( "failed to allocate memory for time steps", (int)(dim_sizes[0]*sizeof(double)) );
  }   

  r=miget_variable_raw(vol,  "info", "time-width", MI_TYPE_DOUBLE, starts, counts, time_steps);
  if ( r < 0 ) {
      error_cnt++;
      TESTRPT ( "failed to get time-width variable values", r );
  }

  printf("\tTime-widths: ");
  {
      int i;
      for(i=0;i<dim_sizes[0];i++)
          printf("%f ",time_steps[i]);
      printf("\n");
  }

  if(time_steps[0]!=0.25 || time_steps[1]!=0.5 || time_steps[2]!=1.0 ||
     time_steps[3]!=2.0 || time_steps[4]!=4.0)
  {
      error_cnt++;
      TESTRPT ( "unexpected values for time-width variable", 0 );
  }

  free(time_steps);
  time_steps=NULL;

  /* close volume*/
  miclose_volume ( vol );
  /*free(buffer);*/
  if(time_steps)
    free(time_steps);

  if ( error_cnt != 0 ) {
    fprintf ( stderr, "%d error%s reported\n",
              error_cnt, ( error_cnt == 1 ) ? "" : "s" );
  } else {
    fprintf ( stderr, "\n No errors\n" );
  }

  return ( error_cnt );
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 
