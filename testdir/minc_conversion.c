#define _GNU_SOURCE 1
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <minc.h>
#include <minc2.h>
#include <limits.h>
#include <float.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>

#define FUNC_ERROR(x) (fprintf(stderr, "On line %d, function %s failed unexpectedly\n", __LINE__, x), ++errors)

#define TST_X 0
#define TST_Y 1
#define TST_Z 2

static long errors = 0;

extern void icv_tests(void);

#define XSIZE 20
#define YSIZE 30
#define ZSIZE 40

#define YBOOST 10
#define ZBOOST 20

static struct dimdef {
  char * name;
  int length;
} dimtab1[3] = { 
  { MIxspace, XSIZE }, 
  { MIyspace, YSIZE },
  { MIzspace, ZSIZE }
};

struct testinfo {
  char *name;
  int fd;
  int maxid;
  int minid;
  int imgid;
  int dim[3];
  int test_group;
  int test_attribute;
};

/* Test case 1 - file creation & definition. 
 */
static int test1(struct testinfo *ip, struct dimdef *dims, int ndims)
{
  int varid;
  int stat;
  int i;

  printf("test1\n");
  /* Test case #1 - file creation 
   */
  ip->name = micreate_tempfile();
  if (ip->name == NULL) {
    FUNC_ERROR("micreate_tempfile\n");
  }

  printf("%s:%d\n",__FILE__,__LINE__);
  ip->fd = micreate(ip->name, NC_CLOBBER|MI2_CREATE_V1); /*Create MINC1 format*/
  if (ip->fd < 0) {
    FUNC_ERROR("micreate");
  }
  printf("%s:%d\n",__FILE__,__LINE__);
  /* Have to use ncdimdef() here since there is no MINC equivalent.  Sigh. 
   */
  printf("%s:%d\n",__FILE__,__LINE__);
  for (i = 0; i < ndims; i++) {

    /* Define the dimension 
     */
    ip->dim[i] = ncdimdef(ip->fd, dims[i].name, dims[i].length);
    if (ip->dim[i] < 0) {
      FUNC_ERROR("ncdimdef");
    }

    /* Create the dimension variable.
     */
    varid = micreate_std_variable(ip->fd, dims[i].name, NC_DOUBLE, 0, &ip->dim[i]);
    if (varid < 0) {
      FUNC_ERROR("micreate_std_variable");
    }
    stat = miattputdbl(ip->fd, varid, MIstep, 0.8);
    if (stat < 0) {
      FUNC_ERROR("miattputdbl");
    }
    stat = miattputdbl(ip->fd, varid, MIstart, 22.0);
    if (stat < 0) {
      FUNC_ERROR("miattputdbl");
    }
  }

  /* Create the image-max variable.
   */
  printf("%s:%d\n",__FILE__,__LINE__);
  ip->maxid = micreate_std_variable(ip->fd, (char*)MIimagemax, NC_DOUBLE, 0, NULL);
  if (ip->maxid < 0) {
    FUNC_ERROR("micreate_std_variable");
  }

  /* Create the image-min variable.
   */
  printf("%s:%d\n",__FILE__,__LINE__);
  ip->minid = micreate_std_variable(ip->fd, (char*)MIimagemin, NC_DOUBLE, 0, NULL);
  if (ip->minid < 0) {
    FUNC_ERROR("micreate_std_variable");
  }

  printf("%s:%d\n",__FILE__,__LINE__);
  ip->imgid = micreate_std_variable(ip->fd, (char*)MIimage, NC_FLOAT, ndims, ip->dim);
  if (ip->imgid < 0) {
    FUNC_ERROR("micreate_std_variable");
  }
  
  return (0);
}

static int test2(struct testinfo *ip, struct dimdef *dims, int ndims)
{
  int i, j, k;
  int stat;
  long coords[3];
  double flt;

  printf("test2\n");
  stat = miattputdbl(ip->fd, ip->imgid, MIvalid_max, (XSIZE * 10000.0));
  if (stat < 0) {
    FUNC_ERROR("miattputdbl");
  }

  stat = miattputdbl(ip->fd, ip->imgid, MIvalid_min, 0.0);
  if (stat < 0) {
    FUNC_ERROR("miattputdbl");
  }

  ncendef(ip->fd);		/* End definition mode. */

  coords[0] = 0;

  flt = 0.0;
  stat = mivarput1(ip->fd, ip->minid, coords, NC_DOUBLE, MI_SIGNED, &flt);
  if (stat < 0) {
    FUNC_ERROR("mivarput1");
  }
    
  flt = XSIZE * 10000.0;
  stat = mivarput1(ip->fd, ip->maxid, coords, NC_DOUBLE, MI_SIGNED, &flt);
  if (stat < 0) {
    FUNC_ERROR("mivarput1");
  }

  for (i = 0; i < dims[TST_X].length; i++) {
    for (j = 0; j < dims[TST_Y].length; j++) {
      for (k = 0; k < dims[TST_Z].length; k++) {
        float tmp = (i * 10000.0f) + (j * 100.0f) + k;

        coords[TST_X] = i;
        coords[TST_Y] = j;
        coords[TST_Z] = k;

        stat = mivarput1(ip->fd, ip->imgid, coords, NC_FLOAT, MI_SIGNED, &tmp);
        if (stat < 0) {
          fprintf(stderr, "At (%d,%d,%d), status %d: ", i,j,k,stat);
          FUNC_ERROR("mivarput1");
        }
      }
    }
  }
  
  return (0);
}

static int
test3(struct testinfo *ip, struct dimdef *dims, int ndims)
{
  /* Get the same variable again, but this time use an ICV to scale it.
   */
  size_t total;
  long coords[3];
  long lengths[3];
  double range[2];
  
  void *buf_ptr;
  float *flt_ptr;
  int i, j, k;
  int stat;
  int icv;

  printf("test3\n");

  total = 1;
  for (i = 0; i < ndims; i++) {
    total *= dims[i].length;
  }
  
  buf_ptr = malloc(total * sizeof (float));
  
  if (buf_ptr == NULL) {
    fprintf(stderr, "Oops, malloc failed\n");
    return (-1);
  }
  
  coords[TST_X] = 0;
  coords[TST_Y] = 0;
  coords[TST_Z] = 0;
  lengths[TST_X] = dims[TST_X].length;
  lengths[TST_Y] = dims[TST_Y].length;
  lengths[TST_Z] = dims[TST_Z].length;
  
  icv = miicv_create();
  if (icv < 0) {
    FUNC_ERROR("miicv_create");
  }
  
  stat = miicv_setint(icv, MI_ICV_TYPE, NC_FLOAT);
  if (stat < 0) {
    FUNC_ERROR("miicv_setint");
  }

  stat = miicv_setint(icv, MI_ICV_DO_NORM, 1);
  if (stat < 0) {
    FUNC_ERROR("miicv_setint");
  }
  
  stat = miicv_setint(icv, MI_ICV_USER_NORM, 1);
  if (stat < 0) {
    FUNC_ERROR("miicv_setint");
  }
  
  stat = miicv_attach(icv, ip->fd, ip->imgid);
  if (stat < 0) {
    FUNC_ERROR("miicv_attach");
  }
  
  stat = miicv_get(icv, coords, lengths, buf_ptr);
  if (stat < 0) {
    FUNC_ERROR("miicv_get");
  }
  
  stat = miget_image_range(ip->fd, range);
  if (stat < 0) {
    FUNC_ERROR("miget_image_range");
  }
  
  if (range[0] != 0 || range[1] != (XSIZE * 10000.0)) {
    fprintf(stderr, "miget_image_range: bad result\n");
    errors++;
  }
  
  stat = miget_valid_range(ip->fd, ip->imgid, range);
  if (stat < 0) {
    FUNC_ERROR("miget_valid_range");
  }
  
  if (range[0] != 0 || range[1] != (XSIZE * 10000.0)) {
    fprintf(stderr, "miget_valid_range: bad result\n");
    errors++;
  }
  
  flt_ptr = (float *) buf_ptr;
  for (i = 0; i < dims[TST_X].length; i++) {
    for (j = 0; j < dims[TST_Y].length; j++) {
      for (k = 0; k < dims[TST_Z].length; k++) {
        float tmp = (i * 10000.0f) + (j * 100.0f) + k;
        if (*flt_ptr != tmp ) {
          fprintf(stderr, "1. Data error at (%d,%d,%d) %f != %f\n", i,j,k, *flt_ptr, tmp);
          errors++;
        }
        flt_ptr++;
      }
    }
  }
  
  stat = miicv_detach(icv);
  if (stat < 0) {
    FUNC_ERROR("miicv_detach");
  }
  
  stat = miicv_free(icv);
  if (stat < 0) {
    FUNC_ERROR("miicv_free");
  }
  
  free(buf_ptr);
  
  return (0);
}


static void test4(struct testinfo *ip, struct dimdef *dims, int ndims)
{
  mihandle_t vol;  
  
  int           ndim;  
  
  int r;
  /*Now we are going to work with the volume using apparent dimension order*/
  midimhandle_t my_dim[3];
  static char *my_dimorder[] = {MIxspace,MIyspace,MIzspace};
  misize_t my_sizes[3];
  misize_t my_start[3];
  misize_t my_count[3];
  miclass_t      volume_class;
  mitype_t       volume_type;
  
  misize_t i,j,k;
  
  float *buffer;
  float *flt_ptr;
  /**/
  if( miopen_volume ( ip->name, MI2_OPEN_READ, &vol )<0)
    FUNC_ERROR("miopen_volume");
  
  if(miget_volume_dimension_count(vol,MI_DIMCLASS_ANY,MI_DIMATTR_ALL,&ndim)<0)
    FUNC_ERROR("miget_volume_dimension_count");
  
  
  if(ndim!=3)
    FUNC_ERROR("incorrect dimensions count");
  
  
  r=miget_data_class(vol, &volume_class);
  
  if ( r < 0 ) {
    FUNC_ERROR ( "failed to get volume class" );
  }
  
  r=miget_data_type(vol, &volume_type);
  
  if ( r < 0 ) {
    FUNC_ERROR ( "failed to get volume type" );
  }
  
 
  if(miset_apparent_dimension_order_by_name ( vol, 3, my_dimorder )<0)
    FUNC_ERROR("miset_apparent_dimension_order_by_name");
  
  /* get the apparent dimensions and their sizes */
  r  = miget_volume_dimensions ( vol, MI_DIMCLASS_ANY,
                                 MI_DIMATTR_ALL, MI_DIMORDER_APPARENT,
                                 3, my_dim );
  if ( r <0 ) {
    FUNC_ERROR("Error in miget_volume_dimensions\n" );
  }
  r = miget_dimension_sizes ( my_dim, 3, my_sizes );
  
  if ( r <0 ) {
    FUNC_ERROR("Error in miget_dimension_sizes\n" );
  }
  
  
  buffer=flt_ptr=malloc(sizeof(float)*my_sizes[0]*my_sizes[1]*my_sizes[2]);
  my_start[0]=my_start[1]=my_start[2]=0;
  
  my_count[0]=my_sizes[0];
  my_count[1]=my_sizes[1];
  my_count[2]=my_sizes[2];
  
  printf("Reading full volume %dx%dx%d float ... \n",(int)my_count[0],(int)my_count[1],(int)my_count[2]);
  
  if ( (r=miget_real_value_hyperslab ( vol, MI_TYPE_FLOAT, my_start, my_count, flt_ptr )) < 0 ) {
    FUNC_ERROR ( "Could not get float full volume." );
  }
  
  
  for (i = 0; i < my_count[0]; i++) {
    for (j = 0; j < my_count[1]; j++) {
      for (k = 0; k < my_count[2]; k++) {
        float tmp = (i * 10000) + (j * 100) + k;
        
        if (*flt_ptr != (float) tmp ) {
          fprintf(stderr, "2. Data error at (%llu,%llu,%llu) %f != %f\n", i,j,k, *flt_ptr, tmp);
          errors++;
        }
        flt_ptr++;
      }
    }
  }
  
  free(buffer);
  
  /* close volume*/
  miclose_volume ( vol );
  
}

/* Test MINC API's 
 */
int main(int argc, char **argv)
{
  struct testinfo info;

  test1(&info, dimtab1, 3);
  
  test2(&info, dimtab1, 3);

  test3(&info, dimtab1, 3);
  
  if (miclose(info.fd) != MI_NOERROR) {
    FUNC_ERROR("miclose");
  }
  
  /*now let's use MINC2 API*/
  test4(&info, dimtab1, 3);
  
  /*unlink(info.name);*/		/* Delete the temporary file. */

  free(info.name);		/* Free the temporary filename */
  
  if(!errors)
    printf("No errors detected!\n");
  else
    printf("Errors: %ld!\n",errors);
  
  return (errors);
}


