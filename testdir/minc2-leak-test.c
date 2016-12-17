#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <ctype.h>
#include <minc2.h>
#include <minc2_private.h>
#include <math.h>
#include <stdint.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/time.h>
#include <sys/resource.h>

#define N_DIMS 3
#define CHUNK_LENGTH 10

/**
 * checks for maximum memory usage. units seem to be different between
 * os x and linux despite the documentation, but the code seems to work
 * either way.
 */
static int
check_high_water_mark(void)
{
  struct rusage usage;
  if (getrusage(RUSAGE_SELF, &usage) < 0)
    return 0;
  return usage.ru_maxrss;
}

static int
hdf5_check_objects(mihandle_t hvol)
{
  hid_t fid = hvol->hdf_id;
  ssize_t cnt;
  int howmany;
  int i;
  H5I_type_t ot;
  hid_t anobj;
  hid_t *objs;
  char name[1024];
  herr_t status;

  cnt = H5Fget_obj_count(fid, H5F_OBJ_ALL);

  if (cnt <= 0) return cnt;

  printf("%ld object(s) open\n", (long)cnt);

  objs = malloc(cnt * sizeof(hid_t));

  howmany = H5Fget_obj_ids(fid, H5F_OBJ_ALL, cnt, objs);

  printf("open objects:\n");

  for (i = 0; i < howmany; i++ ) {
    anobj = objs[i];
    ot = H5Iget_type(anobj);
    status = H5Iget_name(anobj, name, 1024);
    printf(" %d: type %d, name %s %d\n",i,ot,name, status);
  }

  free(objs);

  return howmany;
}

/* This function computes the magic function we use to determine the
 * weighting to apply to memory changes as the process nears
 * completion. Essentially what this means is that changes in memory
 * usage towards the end of the test are much more significant than
 * changes at the beginning.
 */
static double
magic(double fraction)
{
  return -0.5 + 1.0 / (1 + exp(-fraction));
}

static int
leak_loop(mihandle_t hvol, int n, int check_p)
{
  int i;
  misize_t count[N_DIMS];
  misize_t start[N_DIMS];
  double *buffer;    
  int result;
  int hwm = check_high_water_mark();
  int obj = H5Fget_obj_count(hvol->hdf_id, H5F_OBJ_ALL);
  int new_hwm;
  int new_obj;
  int j, k;
  size_t length;
  mitype_t datatype;
  void *attvalue;
  int n_voxels = 1;
  k = 1000;

  for (i = 0; i < N_DIMS; i++) {
    start[i] = 0;
    count[i] = CHUNK_LENGTH;
    n_voxels *= CHUNK_LENGTH;
  }

  buffer = (double *) malloc( n_voxels * sizeof(double));

  for (i = 0; i < n; i += k) {
    for (j = 0; j < k; j++) {
      result = miget_real_value_hyperslab(hvol, MI_TYPE_DOUBLE, start, count,
                                          buffer);
      if (result != MI_NOERROR) {
        fprintf(stderr, "ERROR while getting real hyperslab\n");
        return -1;
      }

      result = miget_voxel_value_hyperslab(hvol, MI_TYPE_DOUBLE, start, count,
                                           buffer);
      if (result != MI_NOERROR) {
        fprintf(stderr, "ERROR while getting raw hyperslab\n");
        return -1;
      }

      result = miget_attr_length(hvol, "", "ident", &length);
      if (result != MI_NOERROR) {
        fprintf(stderr, "ERROR while getting attribute length\n");
        return -1;
      }

      result = miget_attr_type(hvol, "", "ident", &datatype);
      if (result != MI_NOERROR) {
        fprintf(stderr, "ERROR while getting attribute type.\n");
        return -1;
      }

      attvalue = malloc(length + 1);
      result = miget_attr_values(hvol, datatype, "", "ident", length,
                                 attvalue);
      if (result != MI_NOERROR) {
        fprintf(stderr, "ERROR while getting attribute value.\n");
        return -1;
      }
      free(attvalue);
    }

    new_hwm = check_high_water_mark();
    new_obj = H5Fget_obj_count(hvol->hdf_id, H5F_OBJ_ALL);

    if (check_p && new_hwm > hwm) {
      double delta_hwm = new_hwm - hwm;

      /* For the memory leak detection, we use a "soft" check that
       * verifies that the delta does not increase in an unbounded
       * fashion.
       */
      if (delta_hwm * magic((double) i / n) > 100) {
        printf("MEMORY LEAK DETECTED: %f %d %d\n", delta_hwm, i, n);
        free(buffer);
        return 1;
      }
      hwm = new_hwm;
    }
    /* ANY change in the number of object objects reported is a 
     * FATAL ERROR.
     */
    if (check_p && new_obj > obj) {
      printf("HDF5 RESOURCE LEAK DETECTED: %d %d\n", obj, new_obj);
      obj = new_obj;
      hdf5_check_objects(hvol);
      free(buffer);
      return 1;
    }
  }
  free(buffer);
  return 0;
}

#define DIM_LENGTH (CHUNK_LENGTH*2)

static int
create_test_image ( const char *filename )
{
  int r;
  double starts[N_DIMS] = {-(DIM_LENGTH / 2), -(DIM_LENGTH / 2), -(DIM_LENGTH / 2)};
  double separations[N_DIMS] = {1, 1, 1};
  midimhandle_t hdim[N_DIMS];
  mihandle_t hvol;
  uint16_t *buf = NULL;
  int i;
  misize_t count[N_DIMS];
  misize_t start[N_DIMS];
  int nvoxels;
  const char *dimnames[N_DIMS] = {"zspace", "yspace", "xspace"};
  double min_real = -1.0;
  double max_real = +1.0;

  nvoxels = (int) pow(DIM_LENGTH, N_DIMS);

  for (i = 0; i < N_DIMS; i++) {
    r = micreate_dimension( dimnames[i], MI_DIMCLASS_SPATIAL,
                            MI_DIMATTR_REGULARLY_SAMPLED, DIM_LENGTH,
                            &hdim[i] );
    if (r < 0) 
      return r;
  }
  
  r = miset_dimension_starts( hdim, N_DIMS, starts );
  if (r < 0)
    return r;
  
  r = miset_dimension_separations( hdim, N_DIMS, separations );
  if (r < 0)
    return r;
  
  r = micreate_volume(filename, N_DIMS, hdim, MI_TYPE_USHORT,
                      MI_CLASS_REAL, NULL, &hvol );
  if (r < 0) 
    return r;
  
  r = miset_slice_scaling_flag ( hvol, 1 );
  if (r < 0) 
    return r;

  r = micreate_volume_image ( hvol );
  if (r < 0) 
    return r;

  buf = ( uint16_t * ) malloc ( nvoxels * sizeof ( uint16_t ) );
  if (buf == 0) {
    return -1;
  }
  for ( i = 0; i < nvoxels; i++ ) {
    buf[i] = ( uint16_t ) i;
  }

  start[0] = start[1] = start[2] = 0;
  count[0] = count[1] = count[2] = DIM_LENGTH;

  r = miset_voxel_value_hyperslab( hvol, MI_TYPE_USHORT, start, count, buf );
  if (r < 0) {
    return r;
  }
  
  start[0] = start[1] = start[2] = 0;
  for ( i = 0; i < DIM_LENGTH; i++ ) {
    start[0] = i;
    min_real -= 17.0;
    max_real += 17.0;
    r = miset_slice_range( hvol, start, N_DIMS, max_real, min_real );
    if (r < 0) 
      return r;
  }
  r = miclose_volume(hvol);
  free(buf);
  return r;
}

int
main(int argc, char *argv[])
{
  int result = -1;
  mihandle_t hvol;
  int ndims;
  midimhandle_t dimensions[N_DIMS];
  misize_t sizes[N_DIMS];
  char filename[1024];

  snprintf(filename, sizeof(filename), "minc2-leak-%d.mnc", getpid());

  if (create_test_image(filename) != 0) {
    fprintf(stderr, "ERROR creating example file.");
    return -1;
  }

  printf("Created test image '%s'\n", filename);

  result = miopen_volume(filename, MI2_OPEN_READ, &hvol);
  if (result != MI_NOERROR) {
    fprintf(stderr, "Error: opening the input file: %s\n", argv[1]);
    return -1;
  }

  // get some information about the volume
  result = miget_volume_dimension_count(hvol, MI_DIMCLASS_ANY, MI_DIMATTR_ALL, &ndims);
  if (result != MI_NOERROR) {
    fprintf(stderr, "ERROR getting volume dimension count.\n");
    return -1;
  }
  if (ndims > N_DIMS) {
    fprintf(stderr, "ERROR I can handle at most %d dimensions.\n", N_DIMS);
    return -1;
  }
  result = miget_volume_dimensions(hvol, MI_DIMCLASS_ANY, MI_DIMATTR_ALL, 
                                   MI_DIMORDER_FILE, ndims, dimensions);
  if (result != ndims) {
    fprintf(stderr, "ERROR getting volume dimensions (%d).\n", result);
    return -1;
  }
  result = miget_dimension_sizes(dimensions, ndims, sizes);
  if (result != MI_NOERROR) {
    fprintf(stderr, "ERROR getting dimension sizes.\n");
    return -1;
  }

  if (leak_loop(hvol, 100000, 1)) {
    result = 1;
  }
  printf("Done.\n");
  miclose_volume(hvol);
  unlink(filename);
  return result;
}
