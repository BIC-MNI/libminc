#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minc2.h"

#define TESTRPT(msg, val) (error_cnt++, fprintf(stderr, \
                           "Error reported on line #%d, %s: %d\n", \
                           __LINE__, msg, val))

static int error_cnt = 0;


double calculate_mean_f(float *array,misize_t length)
{
  double avg=0.0;
  misize_t i;
  for(i=0;i<length;i++)
  {
    avg+=(double)(array[i]);
  }
  
  return avg/length;
}

double calculate_mean_s(short *array,misize_t length)
{
  double avg=0.0;
  int i;
  for(i=0;i<length;i++)
    avg+=(double)(array[i]);
  
  return avg/length;
}

double calculate_mean_d(double *array,misize_t length)
{
  double avg=0.0;
  misize_t i;
  for(i=0;i<length;i++)
    avg+=array[i];
  
  return avg/length;
}


int main ( int argc, char **argv )
{
  mihandle_t vol;

  
  midimhandle_t *dim;
  int           ndim;
  misize_t *sizes;
  misize_t *start;
  misize_t *count;
  misize_t *howfar;
  misize_t *location;
  double        *origin;
  double        *step;
  miclass_t      volume_class;
  mitype_t       volume_type;
  int            vector_dimension_id=-1;
  int            vector_size=-1;
  milisthandle_t grplist;
  milisthandle_t attlist;
  char group_name[256];
  char attr_name[256];
  int group_id,attr_id;
  
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
    /*nothing else to do here*/
    return ( error_cnt );
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

  /* go over metadata*/

  if ( milist_start(vol, "/", 0, &grplist) == MI_NOERROR )
    {
      printf("Iterating through root groups\n");
      while( milist_grp_next(grplist, group_name, sizeof(group_name)) == MI_NOERROR )
      {
        printf("Group:%s\n",group_name);
        char path[256];
        char attribute[256];
        
         if(milist_start(vol, group_name, 1 , &attlist) == MI_NOERROR)
         {
           while( milist_attr_next(vol,attlist,path,sizeof(path),attribute,sizeof(attribute)) == MI_NOERROR )
           {
             printf("Attribute %s %s\n",path,attribute);
           }
           milist_finish(attlist);
        }
      }
    }
  milist_finish(grplist);
  
  /* close volume*/
  miclose_volume ( vol );
  /*free(buffer);*/

  if ( error_cnt != 0 ) {
    fprintf ( stderr, "%d error%s reported\n",
              error_cnt, ( error_cnt == 1 ) ? "" : "s" );
  } else {
    fprintf ( stderr, "\n No errors\n" );
  }

  return ( error_cnt );
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 
