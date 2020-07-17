#include <stdio.h>
#include <string.h>
#include <math.h>
#include "minc2.h"
#include "config.h"

#define NDIMS 3
#define CX 11
#define CY 12
#define CZ 9


#ifdef _MSC_VER
double rint(double v); /*hack: defined in m2util.c*/
#endif

#define TESTRPT(msg, val) (error_cnt++, printf(\
                                  "Error reported on line #%d, %s: %d\n", \
                                  __LINE__, msg, val))
#define XP 101
#define YP 17
#define ZP 1

#define VALID_MAX (((CX-1)*XP)+((CY-1)*YP)+((CZ-1)*ZP))
#define VALID_MIN (0.0)
#define REAL_MAX (1.0)
#define REAL_MIN (-1.0)
#define NORM_MAX (1.0)
#define NORM_MIN (-1.0)

static int
create_and_test_image(const char *name, 
                      mihandle_t *hvol_ptr, 
                      midimhandle_t hdims[],
                      double outval)
{    
  mihandle_t hvol;
  int result;
  misize_t start[NDIMS];
  misize_t count[NDIMS];
  double stmp2[CX][CY][CZ];
  double dtemp;
  int i,j,k;
  char *dimnames[] = {"zspace", "xspace", "yspace"};
  int error_cnt = 0;

  result = micreate_dimension("xspace", MI_DIMCLASS_SPATIAL, 
                              MI_DIMATTR_REGULARLY_SAMPLED, CX, &hdims[0]);
  if (result < 0) {
    TESTRPT("Unable to create test volume", result);
    return error_cnt;
  }

  result = micreate_dimension("yspace", MI_DIMCLASS_SPATIAL, 
                              MI_DIMATTR_REGULARLY_SAMPLED, CY, &hdims[1]);
  if (result < 0) {
    TESTRPT("Unable to create test volume", result);
    return error_cnt;
  }

  result = micreate_dimension("zspace", MI_DIMCLASS_SPATIAL, 
                              MI_DIMATTR_REGULARLY_SAMPLED, CZ, &hdims[2]);
  if (result < 0) {
    TESTRPT("Unable to create test volume", result);
    return error_cnt;
  }

  result = micreate_volume(name, NDIMS, hdims, MI_TYPE_DOUBLE, 
                           MI_CLASS_REAL, NULL, &hvol);
  if (result < 0) {
    TESTRPT("Unable to create test volume", result);
    return error_cnt;
  }

  result = miget_volume_dimensions(hvol, MI_DIMCLASS_ANY, MI_DIMATTR_ALL, 
                                   MI_DIMORDER_FILE, NDIMS, hdims);
  if (result < 0) {
    TESTRPT("Unable to get volume dimensions", result);
    return error_cnt;
  }

  micreate_volume_image(hvol);

  *hvol_ptr = hvol;

  for (i = 0; i < CX; i++) {
    for (j = 0; j < CY; j++) {
      for (k = 0; k < CZ; k++) {
        stmp2[i][j][k] = (double)((i*XP)+(j*YP)+(k*ZP));
      }
    }
  }

  result = miset_volume_valid_range(hvol, VALID_MAX, VALID_MIN);
  if (result < 0) {
    TESTRPT("error setting valid range", result);
    return error_cnt;
  }

  result = miset_volume_range(hvol, REAL_MAX, REAL_MIN);
  if (result < 0) {
    TESTRPT("error setting real range", result);
    return error_cnt;
  }

  start[0] = start[1] = start[2] = 0;
  count[0] = CX;
  count[1] = CY;
  count[2] = CZ;
  result = miset_real_value_hyperslab(hvol, MI_TYPE_DOUBLE, start, count, stmp2);
  if (result < 0) {
    TESTRPT("unable to set hyperslab", result);
    return error_cnt;
  }

  
  misize_t test_vox[] = { (misize_t) CX/2, (misize_t) CY/2, (misize_t) CZ/2 };
  
  result = miget_real_value(hvol, test_vox, NDIMS, &outval);
  if (result < 0) {
    TESTRPT("unable to read value", result);
    return error_cnt;
  }

  // printf("outval: %f \n", (double) outval);
  // printf("smtp2: %f \n", stmp2[test_vox[0]][test_vox[1]][test_vox[2]]);
  // printf("predicted: %f \n", (double) (test_vox[0] * XP + test_vox[1] * YP + test_vox[2] * ZP));
  if (fabs(outval - stmp2[test_vox[0]][test_vox[1]][test_vox[2]]) > .000001){
    printf("Voxel difference: %f \n", outval -  stmp2[test_vox[0]][test_vox[1]][test_vox[2]]);
    TESTRPT("scaling incorrect for miget_real_value", -1);
    return error_cnt;
  }

  return error_cnt;
}

int
main(void){
  mihandle_t hvol;
  midimhandle_t hdims[NDIMS];
  double outval;
  
  double error_count = create_and_test_image("test-dbl.mnc", &hvol, hdims, outval);
  return(error_count);
}
