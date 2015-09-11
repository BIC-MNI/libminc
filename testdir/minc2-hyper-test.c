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
                      unsigned short stemp[CZ][CX][CY],
                      unsigned short stmp2[CX][CY][CZ])
{    
  mihandle_t hvol;
  int result;
  misize_t start[NDIMS];
  misize_t count[NDIMS];
  double dtemp[CX][CY][CZ];
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

  result = micreate_volume(name, NDIMS, hdims, MI_TYPE_UINT, 
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
        stmp2[i][j][k] = (short)((i*XP)+(j*YP)+(k*ZP));
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
  result = miset_voxel_value_hyperslab(hvol, MI_TYPE_SHORT, start, count, stmp2);
  if (result < 0) {
    TESTRPT("unable to set hyperslab", result);
    return error_cnt;
  }
    
  start[0] = start[1] = start[2] = 0;
  count[0] = CX;
  count[1] = CY;
  count[2] = CZ;
  result = miget_real_value_hyperslab(hvol, MI_TYPE_DOUBLE, start, count, dtemp);
        
  printf("miget_real_value_hyperslab()\n");

  if (result < 0) {
    TESTRPT("Unable to read real value hyperslab", result);
    return error_cnt;
  }
  else {
    for (i = 0; i < CX; i++) {
      for (j = 0; j < CY; j++) {
        for (k = 0; k < CZ; k++) {
          double r = stmp2[i][j][k];

          /* Use fixed known values for valid range, etc.
           */
          r -= VALID_MIN;
          r /= (VALID_MAX - VALID_MIN);
          r *= (REAL_MAX - REAL_MIN);
          r += REAL_MIN;

          /* Have to do approximate comparison, since conversion may
           * not be exact.
           */
          if (fabs(r - dtemp[i][j][k]) > 1.0e-15) {
            printf("%f != %f at %d,%d,%d ",r,dtemp[i][j][k],i,j,k);
            TESTRPT("Value error!", 0);
            break;
          }
        }
      }
    }
  }

  printf("miget_voxel_value_hyperslab, default dimensions\n");

  memset(stmp2, 0, sizeof(short)*CX*CY*CZ); /* Clear the array. */

  result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                       stmp2);
  if (result < 0) {
    TESTRPT("Unable to get raw hyperslab", result);
  }
  else {
    /* Verify all of the values.
     */
    for (i = 0; i < CX; i++) {
      for (j = 0; j < CY; j++) {
        for (k = 0; k < CZ; k++) {
          if (stmp2[i][j][k] != (i*XP)+(j*YP)+(k*ZP)) {
            TESTRPT("Value error", 0);
            break;
          }
        }
      }
    }
  }

  result = miset_apparent_dimension_order_by_name(hvol, NDIMS, dimnames);
  if (result < 0) {
    TESTRPT("unable to set dimension order", result);
  }

  printf("miget_voxel_value_hyperslab(), swapped dimensions\n");

  start[0] = start[1] = start[2] = 0;
  count[0] = CZ;
  count[1] = CX;
  count[2] = CY;
  result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                       stemp);
  if (result < 0) {
    TESTRPT("Error reading swapped hyperslab", result);
  }
  else {
    for (i = 0; i < CZ; i++) {
      for (j = 0; j < CX; j++) {
        for (k = 0; k < CY; k++) {
          if (stemp[i][j][k] != (j*XP)+(k*YP)+(i*ZP)) {
            printf("%d != %d: ", stemp[i][j][k], (j*XP)+(k*YP)+(i*ZP));
            TESTRPT("Value error", 0);
            break;
          }
        }
      }
    }
  }

  /********************************************************************
   * Read and validate the entire dataset.
   * This is done with:
   *  - Axes in (z,y,x) order
   *  - Z axis reversed
   */
  printf("miget_voxel_hyperslab, reversed Z axis\n");

  result = miset_dimension_apparent_voxel_order(hdims[2], 
                                                MI_COUNTER_FILE_ORDER);
  if (result < 0) {
    TESTRPT("unable to set voxel order", result);
  }

  start[0] = start[1] = start[2] = 0;
  count[0] = CZ;
  count[1] = CX;
  count[2] = CY;
  result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                       stemp);
  if (result < 0) {
    TESTRPT("Error reading swapped hyperslab", result);
  }
  else {
    for (i = 0; i < CZ; i++) {
      for (j = 0; j < CX; j++) {
        for (k = 0; k < CY; k++) {
          short t = (short)((j*XP)+(k*YP)+(((CZ-1)-i)*ZP));
          if (stemp[i][j][k] != t) {
            printf("%d != %d: @ %d,%d,%d", stemp[i][j][k], t,i,j,k);
            TESTRPT("Value error", 0);
            break;
          }
        }
      }
    }
  }

  /********************************************************************
   * Attempt to get a series of 3x2x2 matrices from the file.
   * This is done with:
   *  - Axes in (z,x,y) order
   *  - Z axis reversed
   */
  printf("Read 3x2x2 matrices of voxel values:\n");
  count[0] = 3;
  count[1] = 2;
  count[2] = 2;
  for (i = 0; (i + 3) < CZ; i += 3) {
    for (j = 0; (j + 2) < CX; j += 2) {
      for (k = 0; (k + 2) < CY; k += 2) {
        unsigned short stmp3[3][2][2];

        start[0] = i;
        start[1] = j;
        start[2] = k;

        result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT,
                                             start, count, stmp3);
        if (result < 0) {
          TESTRPT("Error reading swapped hyperslab", result);
        }
        else {
          int q,r,s;
          for (q = 0; q < 3; q++) {
            for (r = 0; r < 2; r++) {
              for (s = 0; s < 2; s++) {
                int x,y,z;
                short t;

                x = r + j;
                y = s + k;
                z = q + i;

                t = (short)((x*XP)+(y*YP)+(((CZ-1)-z)*ZP));

                if (stmp3[q][r][s] != t) {
                  printf("%d != %d: ", stmp3[q][r][s], t);
                  TESTRPT("Value error", 0);
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
  return error_cnt;
}

static int
test1()
{
    mihandle_t hvol;
    int result;
    misize_t start[NDIMS];
    misize_t count[NDIMS];
    unsigned short stmp2[CX][CY][CZ];
    unsigned short stemp[CZ][CX][CY];
    unsigned char btemp[CZ][CX][CY];
    int i,j,k;
    midimhandle_t hdims[NDIMS];
    int error_cnt = 0;

    error_cnt = create_and_test_image("tst-hyper.mnc", &hvol, hdims, stemp, stmp2);

    /*********************************************************************
     * Read the entire dataset, normalized.
     *
     * This is done with the axes in (z,y,x) order, but with the Z axis
     * restored to normal (file) order.
     */
    printf("miget_hyperslab_normalized()\n");

    result = miset_dimension_apparent_voxel_order(hdims[2], MI_FILE_ORDER);
    if (result < 0) {
        TESTRPT("unable to set voxel order", result);
    }

    start[0] = start[1] = start[2] = 0;
    count[0] = CZ;
    count[1] = CX;
    count[2] = CY;
    result = miget_hyperslab_normalized(hvol, MI_TYPE_UBYTE, start, count, NORM_MIN, NORM_MAX, btemp);
    if (result < 0) {
        TESTRPT("Can't read normalized hyperslab", result);
    }
    else {
        for (i = 0; i < CZ; i++) {
            for (j = 0; j < CX; j++) {
                for (k = 0; k < CY; k++) {
                    double r = stmp2[j][k][i];

                    /* Calculate what the normalized value ought to be
                     * in this case.  Use fixed known values for valid
                     * range, etc.
                     */
                    r -= VALID_MIN;
                    r /= (VALID_MAX - VALID_MIN);
                    r *= (REAL_MAX - REAL_MIN);
                    r += REAL_MIN;

                    /* r now contains the alleged "real" value for this
                     * voxel.  Now we have to map it to 0-255 for the
                     * unsigned byte type.
                     */

                    if (r > NORM_MAX) {
                        r = 255;
                    }
                    else if (r < NORM_MIN) {
                        r = 0;
                    }
                    else {
                        r = ((r - REAL_MIN) / (REAL_MAX - REAL_MIN)) * 255;
                    }


                    if (btemp[i][j][k] != (unsigned char)rint(r)) {
                        printf("%d != %d: @ %d,%d,%d ", (int)btemp[i][j][k], (int)rint(r), i,j,k);
                        TESTRPT("Value error!", 0);
                        break;
                    }
                }
            }
        }
    }

    /********************************************************************
     * Now read, modify, write, and repeat, performing an exclusive OR
     * operation on each voxel value.
     * Axes are still in (z,y,x) order.
     */
    printf("read and write entire hyperslab:\n");

    result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count, 
                                         stemp);
    if (result < 0) {
        TESTRPT("Can't read hyperslab", result);
    }
    else {
        for (i = 0; i < CZ; i++) {
            for (j = 0; j < CX; j++) {
                for (k = 0; k < CY; k++) {
                    short t = stemp[i][j][k];
                    if (t != (j*XP)+(k*YP)+(i*ZP)) {
                        printf("%d != %d: ", t, (j*XP)+(k*YP)+(i*ZP));
                        TESTRPT("Value error!", 0);
                        break;
                    }
                    stemp[i][j][k] ^= 0x5555;
                }
            }
        }
    }

    result = miset_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count, 
                                         stemp);
    /* stemp has now been modified by the operation! */

    result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                         stemp);
    if (result < 0) {
        TESTRPT("oops", result);
    }
    else {
        for (i = 0; i < CZ; i++) {
            for (j = 0; j < CX; j++) {
                for (k = 0; k < CY; k++) {
                    short t = stemp[i][j][k] ^ 0x5555;
                    if (t != (j*XP)+(k*YP)+(i*ZP)) {
                        printf("%d != %d: ", t, (j*XP)+(k*YP)+(i*ZP));
                        TESTRPT("Value error!", 0);
                        break;
                    }
                    stemp[i][j][k] = t;
                }
            }
        }
    }

    result = miset_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count, 
                                         stemp);
    if (result < 0) {
        TESTRPT("miset_voxel_value_hyperslab failed\n", result);
    }

    /* This call should have the effect of resetting the dimension order
     * to the "raw" file order.
     */
    miset_apparent_dimension_order_by_name(hvol, 0, NULL);

    /* Verify this.
     */
    printf("Read and verify hyperslab in original dimension order:\n");

    start[0] = start[1] = start[2];
    count[0] = CX;
    count[1] = CY;
    count[2] = CZ;
    result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                         stmp2);
    if (result < 0) {
        TESTRPT("Unable to read real value hyperslab", result);
    }
    else {
        for (i = 0; i < CX; i++) {
            for (j = 0; j < CY; j++) {
                for (k = 0; k < CZ; k++) {
                    short t = stmp2[i][j][k];
                    if (t != (i*XP)+(j*YP)+(k*ZP)) {
                        printf("%d != %d: ", t, (i*XP)+(j*YP)+(k*ZP));
                        TESTRPT("Value error!", 0);
                        break;
                    }
                }
            }
        }
    }

    miclose_volume(hvol);
    
    if (error_cnt == 0) {
        printf("No errors\n");
    }
    else {
        printf("**** %d error%s\n", error_cnt, (error_cnt == 1) ? "" : "s");
    }
    return (error_cnt);
}

static int
test2()
{
    mihandle_t hvol;
    int result;
    misize_t start[NDIMS];
    misize_t count[NDIMS];
    unsigned short stmp2[CX][CY][CZ];
    unsigned short stemp[CZ][CX][CY];
    int itemp[CZ][CX][CY];
    int i,j,k;
    midimhandle_t hdims[NDIMS];
    int error_cnt = 0;
    
    error_cnt = create_and_test_image("tst-hyper-s.mnc", &hvol, hdims, stemp, stmp2);

    /*********************************************************************
     * Read the entire dataset, normalized.
     *
     * This is done with the axes in (z,y,x) order, but with the Z axis
     * restored to normal (file) order.
     */
    printf("miget_hyperslab_normalized()\n");

    result = miset_dimension_apparent_voxel_order(hdims[2], MI_FILE_ORDER);
    if (result < 0) {
        TESTRPT("unable to set voxel order", result);
    }

    start[0] = start[1] = start[2] = 0;
    count[0] = CZ;
    count[1] = CX;
    count[2] = CY;
    result = miget_hyperslab_normalized(hvol, MI_TYPE_INT, start, count, REAL_MIN, REAL_MAX, itemp);
    if (result < 0) {
        TESTRPT("Can't read normalized hyperslab", result);
    }
    else {
        for (i = 0; i < CZ; i++) {
            for (j = 0; j < CX; j++) {
                for (k = 0; k < CY; k++) {
                    double r = stmp2[j][k][i];

                    /* Calculate what the normalized value ought to be
                     * in this case.  Use fixed known values for valid
                     * range, etc.
                     */
                    r -= VALID_MIN;
                    r /= (VALID_MAX - VALID_MIN);
                    r *= (REAL_MAX - REAL_MIN);
                    r += REAL_MIN;

                    /* r now contains the alleged "real" value for this
                     * voxel.  Now we have to map it back into the full
                     * range of the buffer type.
                     */

                    if (r > NORM_MAX) {
                      printf("clipping %g @ %d,%d,%d\n", r, i, j, k);
                        r = INT_MAX;
                    }
                    else if (r < NORM_MIN) {
                      printf("clipping %g @ %d,%d,%d\n", r, i, j, k);
                        r = INT_MIN;
                    }
                    else {
                      r = ((r - REAL_MIN) / (REAL_MAX - REAL_MIN)) * ((double)INT_MAX-(double)INT_MIN)+(double)INT_MIN;
                    }

                    if (itemp[i][j][k] != (int)rint(r)) {
                        printf("%d != %d: @ %d,%d,%d ", itemp[i][j][k], (int)rint(r), i,j,k);
                        TESTRPT("Value error!", 0);
                        break;
                    }
                }
            }
        }
    }
    
    printf("miset_hyperslab_normalized()\n");

    result = miset_hyperslab_normalized(hvol, MI_TYPE_INT, start, count,
                                        REAL_MIN, REAL_MAX, itemp);
    if (result < 0) {
        TESTRPT("Can't write hyperslab", result);
    }

    printf("miget_voxel_value_hyperslab()\n");

    start[0] = start[1] = start[2] = 0;
    count[0] = CZ;
    count[1] = CX;
    count[2] = CY;
    result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                         stemp);

    /* Verify that we haven't screwed up the values.
     */
    for (i = 0; i < CZ; i++) {
      for (j = 0; j < CX; j++) {
        for (k = 0; k < CY; k++) {
          unsigned short r = stmp2[j][k][i];
          if (r != stemp[i][j][k]) {
            printf("%d != %d, @ %d,%d,%d\n", r, stemp[i][j][k], i, j, k);
            TESTRPT("Value error: ", 0);
            break;
          }
        }
      }
    }

    miclose_volume(hvol);
    
    if (error_cnt == 0) {
        printf("No errors\n");
    }
    else {
        printf("**** %d error%s\n", error_cnt, (error_cnt == 1) ? "" : "s");
    }
    return (error_cnt);
}

static int
test3()
{
    mihandle_t hvol;
    mivolumeprops_t hprops;
    int result;
    misize_t start[NDIMS];
    misize_t count[NDIMS];
    double dtemp[CX][CY][CZ];
    unsigned short stmp2[CX][CY][CZ];
    unsigned short stemp[CZ][CX][CY];
    int itemp[CZ][CX][CY];
    int i,j,k;
    midimhandle_t hdims[NDIMS];
    char *dimnames[] = {"zspace", "xspace", "yspace"};
    int error_cnt = 0;

    printf("Testing hyperslab operations with multi-resolution image.\n");
    
    result = micreate_dimension("xspace", MI_DIMCLASS_SPATIAL, 
                                MI_DIMATTR_REGULARLY_SAMPLED, CX, &hdims[0]);

    result = micreate_dimension("yspace", MI_DIMCLASS_SPATIAL, 
                                MI_DIMATTR_REGULARLY_SAMPLED, CY, &hdims[1]);

    result = micreate_dimension("zspace", MI_DIMCLASS_SPATIAL, 
                                MI_DIMATTR_REGULARLY_SAMPLED, CZ, &hdims[2]);

    result = minew_volume_props(&hprops);
    miset_props_multi_resolution(hprops, TRUE, 2);

    result = micreate_volume("tst-hyper-m.mnc", NDIMS, hdims, MI_TYPE_INT,
                             MI_CLASS_REAL, hprops, &hvol);

    mifree_volume_props(hprops);

    if (result < 0) {
        TESTRPT("Unable to create test volume", result);
    }

    result = miget_volume_dimensions(hvol, MI_DIMCLASS_ANY, MI_DIMATTR_ALL, 
                                     MI_DIMORDER_FILE, NDIMS, hdims);
    if (result < 0) {
        TESTRPT("Unable to get volume dimensions", result);
    }

    micreate_volume_image(hvol);

    for (i = 0; i < CX; i++) {
        for (j = 0; j < CY; j++) {
            for (k = 0; k < CZ; k++) {
                stmp2[i][j][k] = (unsigned short)((i*XP)+(j*YP)+(k*ZP));
            }
        }
    }

    result = miset_volume_valid_range(hvol, VALID_MAX, VALID_MIN);
    if (result < 0) {
        TESTRPT("error setting valid range", result);
    }

    result = miset_volume_range(hvol, REAL_MAX, REAL_MIN);
    if (result < 0) {
        TESTRPT("error setting real range", result);
    }

    start[0] = start[1] = start[2] = 0;
    count[0] = CX;
    count[1] = CY;
    count[2] = CZ;
    result = miset_voxel_value_hyperslab(hvol, MI_TYPE_SHORT, start, count, stmp2);
    if (result < 0) {
        TESTRPT("unable to set hyperslab", result);
    }
    
    
    start[0] = start[1] = start[2] = 0;
    count[0] = CX;
    count[1] = CY;
    count[2] = CZ;
    result = miget_real_value_hyperslab(hvol, MI_TYPE_DOUBLE, start, count, dtemp);
        
    if (result < 0) {
        TESTRPT("Unable to read real value hyperslab", result);
    }
    else {
        for (i = 0; i < CX; i++) {
            for (j = 0; j < CY; j++) {
                for (k = 0; k < CZ; k++) {
                    double r = stmp2[i][j][k];

                    /* Use fixed known values for valid range, etc.
                     */
                    r -= VALID_MIN;
                    r /= (VALID_MAX - VALID_MIN);
                    r *= (REAL_MAX - REAL_MIN);
                    r += REAL_MIN;

                    /* Have to do approximate comparison, since conversion may
                     * not be exact.
                     */
                    if (fabs(r - dtemp[i][j][k]) > 1.0e-15) {
                        printf("%f != %f at %d,%d,%d ",r,dtemp[i][j][k],i,j,k);
                        TESTRPT("Value error!", 0);
                        break;
                    }
                }
            }
        }
    }

    memset(stmp2, 0, sizeof(short)*CX*CY*CZ); /* Clear the array. */

    result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                         stmp2);
    if (result < 0) {
        TESTRPT("Unable to get raw hyperslab", result);
    }
    else {
        /* Verify all of the values.
         */
        for (i = 0; i < CX; i++) {
            for (j = 0; j < CY; j++) {
                for (k = 0; k < CZ; k++) {
                    if (stmp2[i][j][k] != (i*XP)+(j*YP)+(k*ZP)) {
                        TESTRPT("Value error", 0);
                        break;
                    }
                }
            }
        }
    }

    result = miset_apparent_dimension_order_by_name(hvol, NDIMS, dimnames);
    if (result < 0) {
        TESTRPT("unable to set dimension order", result);
    }

    start[0] = start[1] = start[2] = 0;
    count[0] = CZ;
    count[1] = CX;
    count[2] = CY;
    result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                         stemp);
    if (result < 0) {
        TESTRPT("Error reading swapped hyperslab", result);
    }
    else {
        for (i = 0; i < CZ; i++) {
            for (j = 0; j < CX; j++) {
                for (k = 0; k < CY; k++) {
                    if (stemp[i][j][k] != (j*XP)+(k*YP)+(i*ZP)) {
                        printf("%d != %d: ", stemp[i][j][k], (j*XP)+(k*YP)+(i*ZP));
                        TESTRPT("Value error", 0);
                        break;
                    }
                }
            }
        }
    }

    /********************************************************************
     * Read and validate the entire dataset.
     * This is done with:
     *  - Axes in (z,y,x) order
     *  - Z axis reversed
     */

    result = miset_dimension_apparent_voxel_order(hdims[2], 
                                                  MI_COUNTER_FILE_ORDER);
    if (result < 0) {
        TESTRPT("unable to set voxel order", result);
    }

    start[0] = start[1] = start[2] = 0;
    count[0] = CZ;
    count[1] = CX;
    count[2] = CY;
    result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                         stemp);
    if (result < 0) {
        TESTRPT("Error reading swapped hyperslab", result);
    }
    else {
        for (i = 0; i < CZ; i++) {
            for (j = 0; j < CX; j++) {
                for (k = 0; k < CY; k++) {
                    short t = (short)((j*XP)+(k*YP)+(((CZ-1)-i)*ZP));
                    if (stemp[i][j][k] != t) {
                        printf("%d != %d: @ %d,%d,%d", stemp[i][j][k], t,i,j,k);
                        TESTRPT("Value error", 0);
                        break;
                    }
                }
            }
        }
    }

    /********************************************************************
     * Attempt to get a series of 3x2x2 matrices from the file.
     * This is done with:
     *  - Axes in (z,x,y) order
     *  - Z axis reversed
     */
    count[0] = 3;
    count[1] = 2;
    count[2] = 2;
    for (i = 0; (i + 3) < CZ; i += 3) {
        for (j = 0; (j + 2) < CX; j += 2) {
            for (k = 0; (k + 2) < CY; k += 2) {
                unsigned short stmp3[3][2][2];

                start[0] = i;
                start[1] = j;
                start[2] = k;

                result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT,
                                                     start, count, stmp3);
                if (result < 0) {
                    TESTRPT("Error reading swapped hyperslab", result);
                }
                else {
                    int q,r,s;
                    for (q = 0; q < 3; q++) {
                        for (r = 0; r < 2; r++) {
                            for (s = 0; s < 2; s++) {
                                int x,y,z;
                                short t;

                                x = r + j;
                                y = s + k;
                                z = q + i;

                                t = (short)((x*XP)+(y*YP)+(((CZ-1)-z)*ZP));

                                if (stmp3[q][r][s] != t) {
                                    printf("%d != %d: ", stmp3[q][r][s], t);
                                    TESTRPT("Value error", 0);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /*********************************************************************
     * Read the entire dataset, normalized.
     *
     * This is done with the axes in (z,y,x) order, but with the Z axis
     * restored to normal (file) order.
     */
    result = miset_dimension_apparent_voxel_order(hdims[2], MI_FILE_ORDER);
    if (result < 0) {
        TESTRPT("unable to set voxel order", result);
    }

    start[0] = start[1] = start[2] = 0;
    count[0] = CZ;
    count[1] = CX;
    count[2] = CY;
    result = miget_hyperslab_normalized(hvol, MI_TYPE_INT, start, count, REAL_MIN, REAL_MAX, itemp);
    if (result < 0) {
        TESTRPT("Can't read normalized hyperslab", result);
    }
    else {
        for (i = 0; i < CZ; i++) {
            for (j = 0; j < CX; j++) {
                for (k = 0; k < CY; k++) {
                    double r = stmp2[j][k][i];

                    /* Calculate what the normalized value ought to be
                     * in this case.  Use fixed known values for valid
                     * range, etc.
                     */
                    r -= VALID_MIN;
                    r /= (VALID_MAX - VALID_MIN);
                    r *= (REAL_MAX - REAL_MIN);
                    r += REAL_MIN;

                    /* r now contains the alleged "real" value for this
                     * voxel.  Now we have to map it back into the full
                     * range of the buffer type.
                     */

                    if (r > NORM_MAX) {
                      printf("clipping %g @ %d,%d,%d\n", r, i, j, k);
                        r = INT_MAX;
                    }
                    else if (r < NORM_MIN) {
                      printf("clipping %g @ %d,%d,%d\n", r, i, j, k);
                        r = INT_MIN;
                    }
                    else {
                      r = ((r - REAL_MIN) / (REAL_MAX - REAL_MIN)) * ((double)INT_MAX-(double)INT_MIN)+(double)INT_MIN;
                    }

                    if (itemp[i][j][k] != (int)rint(r)) {
                        printf("%d != %d: @ %d,%d,%d ", itemp[i][j][k], (int)rint(r), i,j,k);
                        TESTRPT("Value error!", 0);
                        break;
                    }
                }
            }
        }
    }
    
    result = miset_hyperslab_normalized(hvol, MI_TYPE_INT, start, count,
                                        REAL_MIN, REAL_MAX, itemp);
    if (result < 0) {
        TESTRPT("Can't write hyperslab", result);
    }

    start[0] = start[1] = start[2] = 0;
    count[0] = CZ;
    count[1] = CX;
    count[2] = CY;
    result = miget_voxel_value_hyperslab(hvol, MI_TYPE_USHORT, start, count,
                                         stemp);

    /* Verify that we haven't screwed up the values.
     */
    for (i = 0; i < CZ; i++) {
      for (j = 0; j < CX; j++) {
        for (k = 0; k < CY; k++) {
          unsigned short r = stmp2[j][k][i];
          if (r != stemp[i][j][k]) {
            printf("%d != %d, @ %d,%d,%d\n", r, stemp[i][j][k], i, j, k);
            TESTRPT("Value error: ", 0);
            break;
          }
        }
      }
    }

    result = miselect_resolution(hvol, 2);
    if (result != MI_NOERROR)
    {
      TESTRPT("Can't select resolution depth 2.", result);
    }
    
    result = miselect_resolution(hvol, 1);
    if (result != MI_NOERROR)
    {
      TESTRPT("Can't select resolution depth 1.", result);
    }
    

    result = miselect_resolution(hvol, 0);
    if (result != MI_NOERROR)
    {
      TESTRPT("Can't select resolution depth 0.", result);
    }
    
    result = miselect_resolution(hvol, 21);
    if (result != MI_ERROR)
    {
      TESTRPT("Selected resolution depth 21.", result);
    }
    
    miclose_volume(hvol);
    
    if (error_cnt == 0) {
        printf("No errors\n");
    }
    else {
        printf("**** %d error%s\n", error_cnt, (error_cnt == 1) ? "" : "s");
    }
    return (error_cnt);
}

int 
main(void)
{
  int error_count = 0;
  error_count += test1();
  error_count += test2();
  error_count += test3();
  return (error_count);
}
