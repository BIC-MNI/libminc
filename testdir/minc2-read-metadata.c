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

static int print_metadata(mihandle_t vol, const char * path,int ident)
{
  milisthandle_t grplist;
  char group_name[256];
  milisthandle_t attlist;
  int r=MI_NOERROR;
  //printf("Printing groups path:%s %d\n",path,ident);
  if ( (r=milist_start(vol, path, 0, &grplist)) == MI_NOERROR )
    {
      while( milist_grp_next(grplist, group_name, sizeof(group_name)) == MI_NOERROR )
      {
        char add_path[256];

        printf("%*s %s:\n",ident,"",group_name);
        strcpy(add_path,path);strcat(add_path,group_name);

        print_metadata(vol,add_path,ident+2);

      }
     milist_finish(grplist);
    }
  else return r;

  //printf("Printing attributes path:%s %d\n",path,ident);
  if((r=milist_start(vol, path, 1 , &attlist)) == MI_NOERROR)
  {
    char int_path[256];
    char attribute[256];

    while( milist_attr_next(vol,attlist,int_path,sizeof(int_path),attribute,sizeof(attribute)) == MI_NOERROR )
    {
      mitype_t att_data_type;
      size_t att_length;
      if(miget_attr_type(vol,int_path,attribute,&att_data_type) == MI_NOERROR &&
         miget_attr_length(vol,int_path,attribute,&att_length) == MI_NOERROR )
      {
        printf("%*s %s:%s type:%s length:%d\n",ident,"",int_path,attribute,get_type_name(att_data_type),(int)att_length);

        switch(att_data_type)
        {
          case MI_TYPE_STRING:
          {
            char *tmp=(char*)malloc(att_length+1);
            if(miget_attr_values(vol,att_data_type,int_path,attribute,att_length+1,tmp) == MI_NOERROR )
              printf("%*s %s\n",ident,"",tmp);
            free(tmp);
          }
          break;
          case MI_TYPE_FLOAT:
          {
            float *tmp=(float*)malloc(att_length*sizeof(float));
            if(miget_attr_values(vol,att_data_type,int_path,attribute,att_length,tmp) == MI_NOERROR )
            {
              size_t i;
              printf("%*s ",ident,"");
              for(i=0;i<att_length;i++)
                printf("%f ",(double)tmp[i]);
              printf("\n");
            }
            free(tmp);
          }
          break;
          case MI_TYPE_DOUBLE:
          {
            double *tmp=(double*)malloc(att_length*sizeof(double));
            if(miget_attr_values(vol,att_data_type,int_path,attribute,att_length,tmp) == MI_NOERROR )
            {
              size_t i;
              printf("%*s ",ident,"");
              for(i=0;i<att_length;i++)
                printf("%f ",tmp[i]);
              printf("\n");
            }
            free(tmp);
          }
          break;
          case MI_TYPE_INT:
          {
            int *tmp=(int*)malloc(att_length*sizeof(int));
            if(miget_attr_values(vol,att_data_type,int_path,attribute,att_length,tmp) == MI_NOERROR )
            {
              size_t i;
              printf("%*s ",ident,"");
              for(i=0;i<att_length;i++)
                printf("%d ",tmp[i]);
              printf("\n");
            }
            free(tmp);
          }
          break;
          default:
            printf("%*sUnsupported type\n",ident,"");
        }
      } else {
        printf("%*s A:%s %s Can't get type or length\n",ident,"",int_path,attribute);
      }
    }
    milist_finish(attlist);
  }

  return r;
}

int main ( int argc, char **argv )
{
  mihandle_t vol;


  int           ndim;
  miclass_t      volume_class;
  mitype_t       volume_type;

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
         get_type_name(volume_type)
    );

  /* go over metadata*/
  print_metadata(vol,"",0);

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
