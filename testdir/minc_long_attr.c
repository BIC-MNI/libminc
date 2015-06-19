#define _GNU_SOURCE 1
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <minc.h>
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
  char *large_attribute;
  int attribute_size;
};

/* Test case 1 - file creation & definition. 
 */
static int test1(struct testinfo *ip, struct dimdef *dims, int ndims)
{
  int varid;
  int stat;
  int i;

  /* Test case #1 - file creation 
   */
  ip->name = micreate_tempfile();
  if (ip->name == NULL) {
    FUNC_ERROR("micreate_tempfile\n");
  }

  ip->fd = micreate(ip->name, NC_CLOBBER);
  if (ip->fd < 0) {
    FUNC_ERROR("micreate");
  }

  /* Have to use ncdimdef() here since there is no MINC equivalent.  Sigh. 
   */
  for (i = 0; i < ndims; i++) {

    /* Define the dimension 
     */
    ip->dim[i] = ncdimdef(ip->fd, dims[i].name, dims[i].length);
    if (ip->dim[i] < 0) {
      FUNC_ERROR("ncdimdef");
    }

    /* Create the dimension variable.
     */
    varid = micreate_std_variable(ip->fd, dims[i].name, NC_DOUBLE, 0, 
				  &ip->dim[i]);
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
  ip->maxid = micreate_std_variable(ip->fd, MIimagemax, NC_FLOAT, 0, NULL);
  if (ip->maxid < 0) {
    FUNC_ERROR("micreate_std_variable");
  }

  /* Create the image-min variable.
   */
  ip->minid = micreate_std_variable(ip->fd, MIimagemin, NC_FLOAT, 0, NULL);
  if (ip->minid < 0) {
    FUNC_ERROR("micreate_std_variable");
  }

  ip->imgid = micreate_std_variable(ip->fd, MIimage, NC_INT, ndims, ip->dim);
  if (ip->imgid < 0) {
    FUNC_ERROR("micreate_std_variable");
  }
  
  ip->test_group = ncvardef(ip->fd,(char*)"test",NC_INT,0,0);/* micreate_group_variable(ip->fd,(char*)"test");*/
  if(ip->test_group<0)
  {
    FUNC_ERROR("micreate_group_variable");
  }
  
  ip->large_attribute=calloc(ip->attribute_size,sizeof(char));
  memset(ip->large_attribute,'X',ip->attribute_size-1);
  
  ip->test_attribute = ncattput(ip->fd, ip->test_group, "test", NC_CHAR, ip->attribute_size, ip->large_attribute);
  
  return (0);
}

static int test2(struct testinfo *ip, struct dimdef *dims, int ndims)
{
  int i, j, k;
  int stat;
  long coords[3];
  float flt;

  stat = miattputdbl(ip->fd, ip->imgid, MIvalid_max, (XSIZE * 10000.0));
  if (stat < 0) {
    FUNC_ERROR("miattputdbl");
  }

  stat = miattputdbl(ip->fd, ip->imgid, MIvalid_min, -(XSIZE * 10000.0));
  if (stat < 0) {
    FUNC_ERROR("miattputdbl");
  }

  ncendef(ip->fd);		/* End definition mode. */

  coords[0] = 0;

  flt = -(XSIZE * 100000.0);
  stat = mivarput1(ip->fd, ip->minid, coords, NC_FLOAT, MI_SIGNED, &flt);
  if (stat < 0) {
    FUNC_ERROR("mivarput1");
  }
    
  flt = XSIZE * 100000.0;
  stat = mivarput1(ip->fd, ip->maxid, coords, NC_FLOAT, MI_SIGNED, &flt);
  if (stat < 0) {
    FUNC_ERROR("mivarput1");
  }

  for (i = 0; i < dims[TST_X].length; i++) {
    for (j = 0; j < dims[TST_Y].length; j++) {
      for (k = 0; k < dims[TST_Z].length; k++) {
        int tmp = (i * 10000) + (j * 100) + k;

        coords[TST_X] = i;
        coords[TST_Y] = j;
        coords[TST_Z] = k;

        stat = mivarput1(ip->fd, ip->imgid, coords, NC_INT, MI_SIGNED, &tmp);
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
  /* Try to read the data back. */
  size_t total;
  long coords[3];
  long lengths[3];
  void *buf_ptr;
  int *int_ptr;
  int i, j, k;
  int stat;
  
  int varid;
  int att_length;
  nc_type att_datatype;
  char *att;

  total = 1;
  for (i = 0; i < ndims; i++) {
    total *= dims[i].length;
  }

  buf_ptr = malloc(total * sizeof (int));

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

  stat = mivarget(ip->fd, ip->imgid, coords, lengths, NC_INT, MI_SIGNED, 
                  buf_ptr);
  if (stat < 0) {
    FUNC_ERROR("mivarget");
  }

  int_ptr = (int *) buf_ptr;
  for (i = 0; i < dims[TST_X].length; i++) {
    for (j = 0; j < dims[TST_Y].length; j++) {
      for (k = 0; k < dims[TST_Z].length; k++) {
        int tmp = (i * 10000) + (j * 100) + k;
        if (*int_ptr != tmp) {
          fprintf(stderr, "1. Data error at (%d,%d,%d)\n", i,j,k);
          errors++;
        }
        int_ptr++;
      }
    }
  }
  free(buf_ptr);

  
  varid = ncvarid(ip->fd, "test");
  if(varid<0)
    FUNC_ERROR("ncvarid");

  if ((ncattinq(ip->fd, varid, (char *)"test", &att_datatype, &att_length) == MI_ERROR) ||
        (att_datatype != NC_CHAR))
    FUNC_ERROR("ncattinq");

  if(att_length!=ip->attribute_size)
    FUNC_ERROR("Wrong attribute length!");

  att=malloc(att_length);

  if(miattgetstr(ip->fd, varid, (char *)"test", att_length, att)==NULL)
    FUNC_ERROR("miattgetstr");

  if(memcmp(att,ip->large_attribute,att_length)!=0)
    FUNC_ERROR("Attribute contents mismath!");

  free(att);
  
  return (0);
}


/* Test MINC API's 
 */
int main(int argc, char **argv)
{
  struct testinfo info;
  if(argc>1)
    info.attribute_size=atoi(argv[1]);
  else
    info.attribute_size=100000;

  printf("Running test with attribute size=%d\n",info.attribute_size);

  test1(&info, dimtab1, 3);

  test2(&info, dimtab1, 3);

  test3(&info, dimtab1, 3);

  if (miclose(info.fd) != MI_NOERROR) {
    FUNC_ERROR("miclose");
  }

  unlink(info.name);		/* Delete the temporary file. */

  free(info.name);		/* Free the temporary filename */

  return (errors);
}


