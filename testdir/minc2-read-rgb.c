#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minc2.h"

#define TESTRPT(msg, val) (error_cnt++, fprintf(stderr, \
                           "Error reported on line #%d, %s: %d\n", \
                           __LINE__, msg, val))

static int error_cnt = 0;


double calculate_mean_f(float *array,int length)
{
  double avg=0.0;
  int i;
  for(i=0;i<length;i++)
    avg+=(double)(array[i]);
  
  return avg/length;
}

double calculate_mean_s(short *array,int length)
{
  double avg=0.0;
  int i;
  for(i=0;i<length;i++)
    avg+=(double)(array[i]);
  
  return avg/length;
}

double calculate_mean_d(double *array,int length)
{
  double avg=0.0;
  int i;
  for(i=0;i<length;i++)
    avg+=array[i];
  
  return avg/length;
}


int main ( int argc, char **argv )
{
  mihandle_t vol;

  
  midimhandle_t *dim;
  int           ndim;
  unsigned int  *sizes;
  unsigned long *start;
  unsigned long *count;
  unsigned long *howfar;
  unsigned long *location;
  double        *origin;
  double        *step;
  miclass_t      volume_class;
  mitype_t       volume_type;
  int            vector_dimension_id=-1;
  int            vector_size=-1;
  
  float *f_buffer;
  double value;
  int r = 0;
  int i;
  
  if(argc<2)
  {
    fprintf(stderr,"Usage: %s <input.mnc> \n",argv[0]);
    return 1;
  }
  

  r = miopen_volume ( argv[1], MI2_OPEN_READ, &vol );

  if ( r < 0 ) {
    TESTRPT ( "failed to open image", r );
  }

  printf("Volume %s info: \n",argv[1]);
  
  r= miget_volume_dimension_count(vol,MI_DIMCLASS_ANY,MI_DIMATTR_ALL,&ndim);
  
  if ( r < 0 ) {
    TESTRPT ( "failed to get image dimension count", r );
  }
  
  r=miget_data_class(vol, &volume_class);
  
  if ( r < 0 ) {
    TESTRPT ( "failed to get volume class", r );
  }
  
  r=miget_data_type(vol, &volume_type);
  
  if ( r < 0 ) {
    TESTRPT ( "failed to get volume type", r );
  }
  
  
  printf("\tNumber of dimensions:%d Data class:%s Data Type: %s\n",ndim,
         volume_class==MI_CLASS_REAL?"Real":
         volume_class==MI_CLASS_INT?"Int":
         volume_class==MI_CLASS_LABEL?"Label":
         volume_class==MI_CLASS_COMPLEX?"Complex":"Other",
         volume_type==MI_TYPE_BYTE?"Byte":
         volume_type==MI_TYPE_SHORT?"Short":
         volume_type==MI_TYPE_INT?"Int":
         volume_type==MI_TYPE_FLOAT?"Float":
         volume_type==MI_TYPE_DOUBLE?"Double":
         volume_type==MI_TYPE_STRING?"String":
         volume_type==MI_TYPE_UBYTE?"Unsigned Byte":
         volume_type==MI_TYPE_USHORT?"Unsigned Short":
         volume_type==MI_TYPE_UINT?"Unsigned Int":"Other"
    );

  /*Allocate memory buffers*/
  dim=  malloc(sizeof(midimhandle_t)*ndim);
  sizes=malloc(sizeof(unsigned int)*ndim);
  start=malloc(sizeof(unsigned long)*ndim);
  count=malloc(sizeof(unsigned long)*ndim);
  howfar=malloc(sizeof(unsigned long)*ndim);
  location=malloc(sizeof(unsigned long)*ndim);
  origin=malloc(sizeof(double)*ndim);
  step=malloc(sizeof(double)*ndim);
  
  /* get the apparent dimensions and their sizes */
  r = miget_volume_dimensions ( vol, MI_DIMCLASS_ANY ,
                                MI_DIMATTR_ALL, MI_DIMORDER_FILE,
                                ndim, dim );
  if ( r <0 ) {
    TESTRPT("Error getting number of dimensions\n" ,r );
  }
  
  r = miget_dimension_sizes ( dim, ndim, sizes );
  if ( r <0 ) {
    TESTRPT("Error getting dimension sizes\n" ,r );
  }
  
  r = miget_dimension_separations(dim,MI_ORDER_FILE,ndim,step);
  if ( r <0 ) {
    TESTRPT("Error getting dimension steps\n" ,r );
  }
  
  printf("\tDimensions:\n");
  for(i=0;i<ndim;i++)
  {
    char *dname;
    midimclass_t dclass;
    miget_dimension_name(dim[i],&dname);
    miget_dimension_class(dim[i],&dclass);
    
    printf("\t\tName: %s Origin:%f Step:%f Size:%d class:%s\n",dname,origin[i],step[i],sizes[i],
            dclass==MI_DIMCLASS_SPATIAL?"Spatial":
            dclass==MI_DIMCLASS_TIME?"Time":
            dclass==MI_DIMCLASS_USER?"User":
            dclass==MI_DIMCLASS_RECORD?"Record":"Other"
          );
    
    if(!strcmp(dname,MIvector_dimension))
    {
      vector_dimension_id=i;
      vector_size=sizes[i];
    }
    free(dname);
  }
  
  if(vector_dimension_id>=0 && ndim==4 ) /*We have got a vector data*/
  {
    /*Now we are going to work with the volume using apparent dimension order*/
    midimhandle_t my_dim[4];
    static char *my_dimorder[] = {MIvector_dimension,MIxspace,MIyspace,MIzspace};
    unsigned int  my_sizes[4];
    unsigned long my_start[4];
    unsigned long my_count[4];

    float *f_coronal,*f_sagittal,*f_axial;/*floating point info*/
    short *s_coronal,*s_sagittal,*s_axial;/*floating point info*/
    /**/
    
    printf("Going to read coronal, axial and sagittal slices\n");
    r = miset_apparent_dimension_order_by_name ( vol, 4, my_dimorder );
    
    if ( r <0 ) {
      TESTRPT("Error setting apparent dimension order\n" ,r );
    }
    /* let's extract coronal, axial and sagittal slices*/
    
    /* get the apparent dimensions and their sizes */
    r  = miget_volume_dimensions ( vol, MI_DIMCLASS_ANY,
                                  MI_DIMATTR_ALL, MI_DIMORDER_APPARENT,/*MI_DIMORDER_FILE,*/
                                  4, my_dim );
    if ( r <0 ) {
      TESTRPT("Error in miget_volume_dimensions\n" ,r );
    }
    r = miget_dimension_sizes ( my_dim, 4, my_sizes );
    
    if ( r <0 ) {
      TESTRPT("Error in miget_dimension_sizes\n" ,r );
    }
    
    /*always extract all three vector components*/
    my_start[0]=0;
    my_count[0]=my_sizes[0];
    
    /*axial, z=const slice*/
    my_start[1]=0;my_count[1]=my_sizes[1];
    my_start[2]=0;my_count[2]=my_sizes[2];
    my_start[3]=0;my_count[3]=1;/*my_sizes[3]/2*/
    f_axial=malloc(sizeof(float)*my_count[0]*my_count[1]*my_count[2]*my_count[3]);

    
    printf("Reading Axial slice:%dx%dx%d float... ",my_count[1],my_count[2],my_count[3]);
    
    if ( (r=miget_real_value_hyperslab ( vol, MI_TYPE_FLOAT, my_start, my_count, f_axial )) < 0 ) {
      TESTRPT ( "Could not get float axial hyperslab.\n",r );
    }
    printf("mean=%f\n",calculate_mean_f(f_axial,my_count[0]*my_count[1]*my_count[2]*my_count[3]));
    
    /*sagittal, x=const slice*/
    my_start[1]=my_sizes[1]/2;my_count[1]=1;
    my_start[2]=0;my_count[2]=my_sizes[2];
    my_start[3]=0;my_count[3]=my_sizes[3];
    f_sagittal=malloc(sizeof(float)*my_count[0]*my_count[1]*my_count[2]*my_count[3]);
    
    printf("Reading Sagittal slice:%dx%dx%d float... ",my_count[1],my_count[2],my_count[3]);
    if ( (r=miget_real_value_hyperslab ( vol, MI_TYPE_FLOAT, my_start, my_count, f_sagittal )) < 0 ) {
      TESTRPT ( "Could not get float sagittal hyperslab.\n",r );
    }
    printf("mean=%f\n",calculate_mean_f(f_sagittal,my_count[0]*my_count[1]*my_count[2]*my_count[3]));
    
    /*coronal, y=const slice*/
    my_start[1]=0;my_count[1]=my_sizes[1];
    my_start[2]=my_sizes[2]/2;my_count[2]=1;
    my_start[3]=0;my_count[3]=my_sizes[3];
    f_coronal=malloc(sizeof(float)*my_count[0]*my_count[1]*my_count[2]*my_count[3]);
    
    printf("Reading Coronal slice:%dx%dx%d float... ",my_count[1],my_count[2],my_count[3]);
    if ( (r=miget_real_value_hyperslab ( vol, MI_TYPE_FLOAT, my_start, my_count, f_coronal )) < 0 ) {
      TESTRPT ( "Could not get float coronal hyperslab.\n",r );
    }
    printf("mean=%f\n",calculate_mean_f(f_coronal,my_count[0]*my_count[1]*my_count[2]*my_count[3]));
    
    /*TODO: do something with volumes*/
    free(f_axial);free(f_coronal);free(f_sagittal);
    
    /*axial, z=const slice*/
    my_start[1]=0;my_count[1]=my_sizes[1];
    my_start[2]=0;my_count[2]=my_sizes[2];
    my_start[3]=my_sizes[3]/2;my_count[3]=1;
    s_axial=malloc(sizeof(short)*my_count[0]*my_count[1]*my_count[2]*my_count[3]);
    
    printf("Reading Axial slice:%dx%dx%d short... ",my_count[1],my_count[2],my_count[3]);
    if ( (r=miget_real_value_hyperslab ( vol, MI_TYPE_SHORT, my_start, my_count, s_axial )) < 0 ) {
      TESTRPT ( "Could not get short axial hyperslab.\n",r );
    }
    printf("mean=%f\n",calculate_mean_s(s_axial,my_count[0]*my_count[1]*my_count[2]*my_count[3]));
    
    /*sagittal, x=const slice*/
    my_start[1]=my_sizes[1]/2;my_count[1]=1;
    my_start[2]=0;my_count[2]=my_sizes[2];
    my_start[3]=0;my_count[3]=my_sizes[3];
    s_sagittal=malloc(sizeof(short)*my_count[0]*my_count[1]*my_count[2]*my_count[3]);
    
    printf("Reading Sagittal slice:%dx%dx%d short... ",my_count[1],my_count[2],my_count[3]);
    if ( (r=miget_real_value_hyperslab ( vol, MI_TYPE_SHORT, my_start, my_count, s_sagittal )) < 0 ) {
      TESTRPT ( "Could not get short sagittal hyperslab.\n",r );
    }
    printf("mean=%f\n",calculate_mean_s(s_sagittal,my_count[0]*my_count[1]*my_count[2]*my_count[3]));
    
    /*coronal, y=const slice*/
    
    my_start[1]=0;my_count[1]=my_sizes[1];
    my_start[2]=my_sizes[2]/2;my_count[2]=1;
    my_start[3]=0;my_count[3]=my_sizes[3];
    s_coronal=malloc(sizeof(short)*my_count[0]*my_count[1]*my_count[2]*my_count[3]);
    
    printf("Reading Coronal slice:%dx%dx%d short... ",my_count[1],my_count[2],my_count[3]);    
    if ( (r=miget_real_value_hyperslab ( vol, MI_TYPE_SHORT, my_start, my_count, s_coronal )) < 0 ) {
      TESTRPT ( "Could not get short coronal hyperslab.\n",r );
    }
    printf("mean=%f\n",calculate_mean_s(s_coronal,my_count[0]*my_count[1]*my_count[2]*my_count[3]));
    
    /*TODO: do something with volumes*/
    free(s_axial);free(s_coronal);free(s_sagittal);
    
  } else  {
    fprintf(stderr,"Sorry, currently I can only process 4D volumes\n");
  }
  /* close volume*/
  miclose_volume ( vol );
  /*free(buffer);*/
  free(count);free(howfar);free(start);free(sizes);free(origin);free(step);

  if ( error_cnt != 0 ) {
    fprintf ( stderr, "%d error%s reported\n",
              error_cnt, ( error_cnt == 1 ) ? "" : "s" );
  } else {
    fprintf ( stderr, "\n No errors\n" );
  }

  return ( error_cnt );
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 
