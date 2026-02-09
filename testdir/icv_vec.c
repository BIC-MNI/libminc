#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>

#include <math.h>
#include <minc.h>

#include <string.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

static const int NZ = 9;                    /* NZ,NY, and NX should be <= 10! */
static const int NY = 8;
static const int NX = 10;
static const int NV = 6;

/* define the dimension ordering we use - z,y,x,vector_dimension */
enum dimension_index { IZ, IY, IX, IV, N_DIM };

#define TVAL(z,y,x,v)  ((z)*1000+(y)*100+(x)*10+(v))
#define TINDV(z,y,x,v) ((v) + ((x)*NV)+((y)*NV*NX)+((z)*NV*NX*NY))
#define TIND(z,y,x)    ((x)+((y)*NX)+((z)*NX*NY))
#define TINDVS(z,y,x,v) ((v) + ((x)*NV)+((y)*NV*NX)+((z)*NV*NX*(NY+2)))
#define TINDS(z,y,x)    ((x)+((y)*NX)+((z)*NX*(NY+2)))

#define IMAGE_MIN -1.0
#define IMAGE_MAX 1.0
#define VOXEL_MIN 0.0
#define VOXEL_MAX TVAL(NZ-1,NY-1,NX-1,NV-1)
#define N_ELEMENTS (NZ*(NY+2)*NX*NV) /* NY+2 to accommodate stretching test */

static int
test_icv_vector(int cflag, nc_type voxel_type)
{
  int icv, cdfid, img, max, min, dimvar;
  int dim[N_DIM];
  struct {
    long len;
    const char *name;
  } diminfo[N_DIM];
  int numdims = N_DIM;
  long coord[N_DIM];
  long count[N_DIM];
  short int *ivalue = malloc(N_ELEMENTS * sizeof(short int));
  double *dvalue = malloc(N_ELEMENTS * sizeof(double));
  int i, j, k, v;
  char filename[256];
  int return_val;
  int error_count = 0;

  printf("Testing with voxel type=%d\n", voxel_type);

  if (ivalue == NULL || dvalue == NULL) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    return 1;
  }

  diminfo[IZ].len = NZ;
  diminfo[IZ].name = MIzspace;

  diminfo[IY].len = NY;
  diminfo[IY].name = MIyspace;

  diminfo[IX].len = NX;
  diminfo[IX].name = MIxspace;

  diminfo[IV].len = NV;
  diminfo[IV].name = MIvector_dimension;

  coord[IZ] = coord[IY] = coord[IX] = coord[IV] = 0;
  count[IZ] = NZ;
  count[IY] = NY;
  count[IX] = NX;
  count[IV] = NV;

  snprintf(filename, sizeof(filename), "test_icv_vec-%d.mnc", getpid());
  cdfid = micreate(filename, NC_CLOBBER | cflag);
  for (i = 0; i < numdims; i++) {
    dim[i] = ncdimdef(cdfid, diminfo[i].name, diminfo[i].len);
    if (i != IV) {
      dimvar = micreate_std_variable(cdfid, diminfo[i].name, NC_DOUBLE, 0, &dim[i]);
      miattputdbl(cdfid, dimvar, MIstep, 0.8);
      miattputdbl(cdfid, dimvar, MIstart, 22.0);
    }
  }

  img = micreate_std_variable(cdfid, MIimage, voxel_type, numdims, dim);
  miattputdbl(cdfid, img, MIvalid_max, VOXEL_MAX);
  miattputdbl(cdfid, img, MIvalid_min, VOXEL_MIN);
  max = micreate_std_variable(cdfid, MIimagemax, NC_DOUBLE, 1, dim);
  min = micreate_std_variable(cdfid, MIimagemin, NC_DOUBLE, 1, dim);
  ncendef(cdfid);
  for (i = 0; i < diminfo[0].len; i++) {
    double temp = 1.0;
    coord[IZ] = i;
    ncvarput1(cdfid, max, coord, &temp);
    temp = -temp;
    ncvarput1(cdfid, min, coord, &temp);
  }
  for (i = 0; i < NZ; i++) {
    for (j = 0; j < NY; j++) {
      for (k = 0; k < NX; k++) {
        for (v = 0; v < NV; v++) {
          int c = TINDV(i, j, k, v);
          ivalue[c] = TVAL(i, j, k, v);
        }
      }
    }
  }

  coord[IZ] = coord[IY] = coord[IX] = coord[IV] = 0;
  mivarput(cdfid, img, coord, count, NC_SHORT, MI_UNSIGNED, ivalue);

  icv = miicv_create();
  return_val = miicv_setint(icv, MI_ICV_DO_NORM, TRUE);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  return_val = miicv_setint(icv, MI_ICV_TYPE, NC_DOUBLE);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  miicv_setint(icv, MI_ICV_DO_DIM_CONV, TRUE);
  return_val = miicv_attach(icv, cdfid, img);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  for (i = 0; i < N_ELEMENTS; i++)
    dvalue[i] = 0;
  return_val = miicv_get(icv, coord, count, dvalue);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  for (i = 0; i < NZ; i++) {
    for (j = 0; j < NY; j++) {
      for (k = 0; k < NX; k++) {
        double iv = 0.0;
        for (v = 0; v < NV; v++) {
          iv += TVAL(i,j,k,v);
        }
        iv /= NV;
        if (voxel_type != NC_DOUBLE && voxel_type != NC_FLOAT) {
          iv = (iv + VOXEL_MIN) / (VOXEL_MAX - VOXEL_MIN);
          iv = iv * (IMAGE_MAX - IMAGE_MIN) + IMAGE_MIN;
        }
        if (fabs(iv - dvalue[TIND(i, j, k)]) > 1e-8) {
          fprintf(stdout, "Error on line %d, (%d,%d,%d): %8.7f %8.7f\n",
                  __LINE__, i, j, k, dvalue[TIND(i,j,k)], iv);
        }
      }
    }
  }
  return_val = miicv_detach(icv);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  return_val = miicv_setint(icv, MI_ICV_DO_SCALAR, FALSE);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  return_val = miicv_attach(icv, cdfid, img);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }

  for (i=0; i < N_ELEMENTS; i++)
    dvalue[i] = 0;
  return_val = miicv_get(icv, coord, count, dvalue);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code %d at line %d!\n",
            return_val, __LINE__);
    error_count++;
  }
  for (i = 0; i < NZ; i++) {
    for (j = 0; j < NY; j++) {
      for (k = 0; k < NX; k++) {
        for (v = 0; v < NV; v++) {
          double iv = TVAL(i,j,k,v);
          if (voxel_type != NC_DOUBLE && voxel_type != NC_FLOAT) {
            iv = (iv + VOXEL_MIN) / (VOXEL_MAX - VOXEL_MIN);
            iv = iv * (IMAGE_MAX - IMAGE_MIN) + IMAGE_MIN;
          }
          if (fabs(iv - dvalue[TINDV(i,j,k,v)]) > 1e-8) {
            fprintf(stdout, "ERROR on line %d, (%d,%d,%d,%d): %8.7f %8.7f\n",
                    __LINE__, i, j, k, v, dvalue[TINDV(i,j,k,v)], iv);
            error_count++;
            break;
          }
        }
      }
    }
  }

  return_val = miicv_detach(icv);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  return_val = miicv_setint(icv, MI_ICV_DO_SCALAR, TRUE);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  return_val = miicv_setint(icv, MI_ICV_KEEP_ASPECT, FALSE);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  /* Should increase Y dimension size by one. */
  return_val = miicv_setint(icv, MI_ICV_BDIM_SIZE, NY+2);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  return_val = miicv_setint(icv, MI_ICV_DO_RANGE, FALSE);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  return_val = miicv_setint(icv, MI_ICV_DO_NORM, FALSE);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }

  return_val = miicv_setint(icv, MI_ICV_TYPE, NC_DOUBLE);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  return_val = miicv_setint(icv, MI_ICV_DO_DIM_CONV, TRUE);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  return_val = miicv_attach(icv, cdfid, img);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }

  for (i = 0; i < N_ELEMENTS; i++)
    dvalue[i] = 0;

  count[IY] = NY+2;
  return_val = miicv_get(icv, coord, count, dvalue);
  if (return_val != MI_NOERROR) {
    fprintf(stdout, "ERROR bad return code at line %d!\n", __LINE__);
    error_count++;
  }
  for (i = 0; i < NZ; i++) {
    for (j = 0; j < NY+2; j++) {
      for (k = 0; k < NX; k++) {
        double iv = 0.0;
        if (j >= 1 && j < NY+1) {
          for (v = 0; v < NV; v++) {
            iv += TVAL(i,j-1,k,v);
          }
          iv /= NV;
        }
        if (fabs(iv - dvalue[TINDS(i,j,k)]) > 1e-8) {
          fprintf(stdout, "ERROR on line %d, (%d,%d,%d): %8.7f %8.7f\n",
                  __LINE__, i, j, k, dvalue[TINDS(i,j,k)], iv);
          error_count++;
          break;
        }
      }
    }
  }
  miclose(cdfid);
  miicv_free(icv);
  free(ivalue);
  free(dvalue);
  unlink(filename);
  return (error_count);
}

#define N_TYPES 4

int main(int argc, char **argv)
{
  int output_type[N_TYPES] = {NC_SHORT, NC_INT, NC_FLOAT, NC_DOUBLE};
  int i;
  int error_count = 0;
  int cflag = 0;

  ncopts &= ~(NC_FATAL | NC_VERBOSE);

  printf("Testing ICV with vector dimensions.\n");

#if MINC2
  if (argc == 2 && !strcmp(argv[1], "-2")) {
    cflag = MI2_CREATE_V2;
  }
#endif /* MINC2 */

  for (i = 0; i < N_TYPES; i++) {
    error_count += test_icv_vector(cflag, output_type[i]);
  }
  printf("Exiting with error count %d\n", error_count);
  return error_count;
}


