/** \file hyper.c
 * \brief MINC 2.0 Hyperslab Functions
 * \author Bert Vincent
 *
 * Functions to manipulate hyperslabs of volume image data.
 ************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H

#include <stdlib.h>

#include "minc2.h"
#include "minc2_private.h"

#define MIRW_OP_READ 1
#define MIRW_OP_WRITE 2

typedef unsigned long mioffset_t;

/** In-place array dimension restructuring.
 *
 * Based on Chris H.Q. Ding, "An Optimal Index Reshuffle Algorithm for
 * Multidimensional Arrays and its Applications for Parallel Architectures"
 * IEEE Transactions on Parallel and Distributed Systems, Vol.12, No.3,
 * March 2001, pp.306-315.
 *
 * I rewrote the algorithm in "C" an generalized it to N dimensions.
 *
 * Guaranteed to do the minimum number of memory moves, but requires
 * that we allocate a bitmap of nelem/8 bytes.  The paper suggests
 * ways to eliminate the bitmap - I'll work on it.
 */

/**
 * Map a set of array coordinates to a linear offset in the array memory.
 */
static mioffset_t
index_to_offset(int ndims,
                const unsigned long sizes[],
                const unsigned long index[])
{
  mioffset_t offset = index[0];
  int i;

  for (i = 1; i < ndims; i++) {
    offset *= sizes[i];
    offset += index[i];
  }
  return (offset);
}

/**
 * Map a linear offset to a set of coordinates in a multidimensional array.
 */
static void
offset_to_index(int ndims,
                const unsigned long sizes[],
                mioffset_t offset,
                unsigned long index[])
{
  int i;

  for (i = ndims - 1; i > 0; i--) {
    index[i] = offset % sizes[i];
    offset /= sizes[i];
  }
  index[0] = offset;
}

/* Trivial bitmap test & set.
 */
#define BIT_TST(bm, i) (bm[(i) / 8] & (1 << ((i) % 8)))
#define BIT_SET(bm, i) (bm[(i) / 8] |= (1 << ((i) % 8)))

/** The main restructuring code.
 */
MNCAPI void
restructure_array(int ndims,    /* Dimension count */
                  unsigned char *array, /* Raw data */
                  const unsigned long *lengths_perm, /* Permuted lengths */
                  int el_size,  /* Element size, in bytes */
                  const int *map, /* Mapping array */
                  const int *dir) /* Direction array, in permuted order */
{
  unsigned long index[MI2_MAX_VAR_DIMS]; /* Raw indices */
  unsigned long index_perm[MI2_MAX_VAR_DIMS]; /* Permuted indices */
  unsigned long lengths[MI2_MAX_VAR_DIMS]; /* Raw (unpermuted) lengths */
  unsigned char *temp;
  mioffset_t offset_start;
  mioffset_t offset_next;
  mioffset_t offset;
  unsigned char *bitmap;
  size_t total;
  int i;

  if ((temp = malloc(el_size)) == NULL) {
    //TODO: report MEMORY error somehow
    return;
  }

  /**
   * Permute the lengths from their "output" configuration back into
   * their "raw" or native order:
   **/
  for (i = 0; i < ndims; i++) {
    lengths[map[i]] = lengths_perm[i];
  }

  /**
   * Calculate the total size of the array, in elements.
   **/
  total = 1;
  for (i = 0; i < ndims; i++) {
    total *= lengths[i];
  }

  /**
   * Allocate a bitmap with enough space to hold one bit for each
   * element in the array.
   **/
  bitmap = calloc((total + 8 - 1) / 8, 1); /* bit array */
  if (bitmap == NULL) {
    //TODO: report MEMORY error somehow
    return;
  }

  for (offset_start = 0; offset_start < total; offset_start++) {

    /**
     * Look for an unset bit - that's where we start the next
     * cycle.
     **/

    if (!BIT_TST(bitmap, offset_start)) {

      /**
       * Found a cycle we have not yet performed.
       **/

      offset_next = -1;   /* Initialize. */

#ifdef DEBUG
      printf("%ld", offset_start);
#endif /* DEBUG */

      /**
       * Save the first element in this cycle.
       **/

      memcpy(temp, array + (offset_start * el_size), el_size);

      /**
       * We've touched this location.
       **/

      BIT_SET(bitmap, offset_start);

      offset = offset_start;

      /**
       * Do until the cycle repeats.
       **/

      while (offset_next != offset_start) {

        /**
         * Compute the index from the offset and permuted length.
         **/

        offset_to_index(ndims, lengths_perm, offset, index_perm);

        /**
         * Permute the index into the alternate arrangement.
         **/

        for (i = 0; i < ndims; i++) {
          if (dir[i] < 0) {
            index[map[i]] = lengths[map[i]] - index_perm[i] - 1;
          } else {
            index[map[i]] = index_perm[i];
          }
        }

        /**
         * Calculate the next offset from the permuted index.
         **/

        offset_next = index_to_offset(ndims, lengths, index);
#ifdef DEBUG
        if (offset_next >= total) {
          printf("Fatal - offset %ld out of bounds!\n", offset_next);
          printf("lengths %ld,%ld,%ld\n",
                 lengths[0],lengths[1],lengths[2]);
          printf("index %ld,%ld,%ld\n",
                 index_perm[0], index_perm[0], index_perm[2]);
          //TODO: report MEMORY error somehow
          exit(-1);
        }
#endif
        /**
         * If we are not at the end of the cycle...
         **/

        if (offset_next != offset_start) {

          /**
           * Note that we've touched a new location.
           **/

          BIT_SET(bitmap, offset_next);

#ifdef DEBUG
          printf(" - %ld", offset_next);
#endif /* DEBUG */

          /**
           * Move from old to new location.
           **/

          memcpy(array + (offset * el_size),
                 array + (offset_next * el_size),
                 el_size);

          /**
           * Advance offset to the next location in the cycle.
           **/

          offset = offset_next;
        }
      }

      /**
       * Store the first value in the cycle, which we saved in
       * 'tmp', into the last offset in the cycle.
       **/

      memcpy(array + (offset * el_size), temp, el_size);

#ifdef DEBUG
      printf("\n");
#endif /* DEBUG */
    }
  }

  free(bitmap);               /* Get rid of the bitmap. */
  free(temp);
}

/** Calculates and returns the number of bytes required to store the
 * hyperslab specified by the \a n_dimensions and the
 * \a count parameters, using hdf type id
 */
void 
miget_hyperslab_size_hdf(hid_t hdf_type_id, int n_dimensions, 
                                const unsigned long count[], 
                                misize_t *size_ptr)
{
  int voxel_size;
  misize_t temp;
  int i;
  voxel_size = H5Tget_size(hdf_type_id);

  temp = 1;
  for (i = 0; i < n_dimensions; i++) {
    temp *= count[i];
  }
  *size_ptr = (temp * voxel_size);
}


/** Calculates and returns the number of bytes required to store the
 * hyperslab specified by the \a n_dimensions and the
 * \a count parameters.
 */
int
miget_hyperslab_size(mitype_t volume_data_type,   /**< Data type of a voxel. */
                     int n_dimensions,            /**< Dimensionality */
                     const unsigned long count[], /**< Dimension lengths  */
                     misize_t *size_ptr)          /**< Returned byte count */
{
  hid_t type_id;

  type_id = mitype_to_hdftype(volume_data_type, TRUE);
  if (type_id < 0) {
    return (MI_ERROR);
  }
  
  miget_hyperslab_size_hdf(type_id,n_dimensions,count,size_ptr);

  H5Tclose(type_id);
  return (MI_NOERROR);
}

/** "semiprivate" function for translating coordinates.
 */
int
mitranslate_hyperslab_origin(mihandle_t volume,
                             const unsigned long start[],
                             const unsigned long count[],
                             hsize_t hdf_start[],
                             hsize_t hdf_count[],
                             int dir[]) /* direction vector in file order */
{
  int n_different = 0;
  int file_i;
  int ndims = volume->number_of_dims;
  int j;

  for(j=0; j<ndims; j++) {
    hdf_count[j]=0;
    hdf_start[j]=0;
  }
  for (file_i = 0; file_i < ndims; file_i++) {
    midimhandle_t hdim;
    int user_i;

    /* Set up the basic translations of dimensions, for
     * flipping directions and swapping indices.
     */
    if (volume->dim_indices != NULL) {
      /* Have to swap indices */
      user_i = volume->dim_indices[file_i];
      if (user_i != file_i) {
        n_different++;
      }
    } else {
      user_i = file_i;
    }

    hdim = volume->dim_handles[user_i];
    switch (hdim->flipping_order) {
    case MI_FILE_ORDER:
      hdf_start[user_i] = start[file_i];
      dir[file_i] = 1;    /* Set direction positive */
      break;

    case MI_COUNTER_FILE_ORDER:
      hdf_start[user_i] = hdim->length - start[file_i] - count[file_i];
      dir[file_i] = -1;   /* Set direction negative */
      break;

    case MI_POSITIVE:
      if (hdim->step > 0) { /* Positive? */
        hdf_start[user_i] = start[file_i];
        dir[file_i] = 1; /* Set direction positive */
      } else {
        hdf_start[user_i] = hdim->length - start[file_i] - count[file_i];
        dir[file_i] = -1; /* Set direction negative */
      }
      break;

    case MI_NEGATIVE:
      if (hdim->step < 0) { /* Negative? */
        hdf_start[user_i] = start[file_i];
        dir[file_i] = 1; /* Set direction positive */
      } else {
        hdf_start[user_i] = hdim->length - start[file_i] - count[file_i];
        dir[file_i] = -1; /* Set direction negative */
      }
      break;
    }
    hdf_count[user_i] = count[file_i];
  }
  return (n_different);
}

/** Read/write a hyperslab of data.  This is the simplified function
 * which performs no value conversion.  It is much more efficient than
 * mirw_hyperslab_icv()
 */
static int mirw_hyperslab_raw(int opcode,
                              mihandle_t volume,
                              mitype_t midatatype,
                              const unsigned long start[],
                              const unsigned long count[],
                              void *buffer)
{
  hid_t dset_id = -1;
  hid_t mspc_id = -1;
  hid_t fspc_id = -1;
  hid_t type_id = -1;
  int result = MI_ERROR;
  hsize_t hdf_start[MI2_MAX_VAR_DIMS];
  hsize_t hdf_count[MI2_MAX_VAR_DIMS];
  int dir[MI2_MAX_VAR_DIMS];  /* Direction vector in file order */
  int ndims;
  int n_different = 0;


  /* Disallow write operations to anything but the highest resolution.
   */
  if (opcode == MIRW_OP_WRITE && volume->selected_resolution != 0) {
    return (MI_ERROR);
  }

  dset_id = volume->image_id;
  if (dset_id < 0) {
    goto cleanup;
  }

  fspc_id = H5Dget_space(dset_id);
  if (fspc_id < 0) {
    goto cleanup;
  }

  if (midatatype == MI_TYPE_UNKNOWN) {
    type_id = H5Tcopy(volume->mtype_id);
  } else {
    type_id = mitype_to_hdftype(midatatype, TRUE);
  }

  ndims = volume->number_of_dims;

  if (ndims == 0) {
    /* A scalar volume is possible but extremely unlikely, not to
     * mention useless!
     */
    mspc_id = H5Screate(H5S_SCALAR);
  } else {

    n_different = mitranslate_hyperslab_origin(volume, start, count, hdf_start, hdf_count, dir);

    mspc_id = H5Screate_simple(ndims, hdf_count, NULL);
    if (mspc_id < 0) {
      goto cleanup;
    }
  }

  result = H5Sselect_hyperslab(fspc_id, H5S_SELECT_SET, hdf_start, NULL,
                               hdf_count, NULL);
  if (result < 0) {
    goto cleanup;
  }

  if (opcode == MIRW_OP_READ) {
    result = H5Dread(dset_id, type_id, mspc_id, fspc_id, H5P_DEFAULT,buffer);
    
    /* Restructure the array after reading the data in file orientation.
     */
    if (n_different != 0) {
      restructure_array(ndims, buffer, count, H5Tget_size(type_id),
                        volume->dim_indices, dir);
    }
  } else {

    volume->is_dirty = TRUE; /* Mark as modified. */

    /* Restructure array before writing to file.
     * TODO: use temporary buffer for that!
     */

    if (n_different != 0) {
      unsigned long icount[MI2_MAX_VAR_DIMS];
      int idir[MI2_MAX_VAR_DIMS];
      int imap[MI2_MAX_VAR_DIMS];
      int i;

      /* Invert before calling */
      for (i = 0; i < ndims; i++) {
        icount[volume->dim_indices[i]] = count[i];

        idir[volume->dim_indices[i]] = dir[i];

        // this one was correct the original way
        imap[volume->dim_indices[i]] = i;

      }

      restructure_array(ndims, buffer, icount, H5Tget_size(type_id),
                        imap, idir);
    }

    result = H5Dwrite(dset_id, type_id, mspc_id, fspc_id, H5P_DEFAULT,
                      buffer);
  }

cleanup:

  if (type_id >= 0) {
    H5Tclose(type_id);
  }
  if (mspc_id >= 0) {
    H5Sclose(mspc_id);
  }
  if (fspc_id >= 0) {
    H5Sclose(fspc_id);
  }
  return (result);
}

#define APPLY_DESCALING(type,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,voxel_min,voxel_max) \
  { \
    int _i,_j;\
    double voxel_range=voxel_max-voxel_min;\
    double voxel_offset=voxel_min;\
    type *_buffer=(type *)buffer;\
    for(_i=0;_i<total_number_of_slices;_i++)\
      for(_j=0;_j<image_slice_length;_j++)\
      {\
        double _temp;\
        _temp=*_buffer;\
        *_buffer =(type)( ((_temp - voxel_offset) / voxel_range)*(image_slice_max_buffer[_i]-image_slice_min_buffer[_i]) + image_slice_min_buffer[_i] ); \
        _buffer++;\
      }\
  }
  
#define APPLY_SCALING(type,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,voxel_min,voxel_max) \
  { \
    int _i,_j;\
    double voxel_range=voxel_max-voxel_min;\
    double voxel_offset=voxel_min;\
    type *_buffer=(type *)buffer;\
    for(_i=0;_i<total_number_of_slices;_i++)\
      for(_j=0;_j<image_slice_length;_j++)\
      {\
        double _temp;\
        _temp=*_buffer;\
        *_buffer = (type)(((_temp - image_slice_min_buffer[_i])/(image_slice_max_buffer[_i]-image_slice_min_buffer[_i]))*voxel_range + voxel_offset) ; \
        _buffer++;\
      }\
  }


/** Read/write a hyperslab of data, performing dimension remapping
 * and data rescaling as needed.
 */
static int mirw_hyperslab_icv(int opcode,
                              mihandle_t volume,
                              mitype_t buffer_data_type,
                              const unsigned long start[],
                              const unsigned long count[],
                              void *buffer)
{
  hid_t dset_id = -1;
  hid_t mspc_id = -1;
  hid_t fspc_id = -1;
  hid_t volume_type_id = -1;
  hid_t buffer_type_id = -1;
  int result = MI_ERROR;
  hsize_t hdf_start[MI2_MAX_VAR_DIMS];
  hsize_t hdf_count[MI2_MAX_VAR_DIMS];
  int dir[MI2_MAX_VAR_DIMS];  /* Direction vector in file order */
  int ndims;
  int slice_ndims;
  int n_different = 0;
  double volume_valid_min, volume_valid_max;
  misize_t buffer_size;
  void *temp_buffer=NULL;
  unsigned long icount[MI2_MAX_VAR_DIMS];
  int idir[MI2_MAX_VAR_DIMS];
  int imap[MI2_MAX_VAR_DIMS];
  double *image_slice_max_buffer=NULL;
  double *image_slice_min_buffer=NULL;
  
  int image_slice_start[MI2_MAX_VAR_DIMS];
  int image_slice_count[MI2_MAX_VAR_DIMS];
  int image_slice_length=-1;
  int total_number_of_slices=-1;
  int i;
  int j;

  /* Disallow write operations to anything but the highest resolution.
   */
  if (opcode == MIRW_OP_WRITE && volume->selected_resolution != 0) {
    /*TODO: report error that we are not dealing with the rihgt image here*/
    return (MI_ERROR);
  }

  dset_id = volume->image_id;
  if (dset_id < 0) {
    /*TODO: report the image was not opened*/
    goto cleanup;
  }

  fspc_id = H5Dget_space(dset_id);
  if (fspc_id < 0) {
    /*TODO: report can't get dataset*/
    goto cleanup;
  }

  volume_type_id = H5Tcopy(volume->mtype_id);
  if(volume_type_id<0)
  {
    fprintf(stderr,"H5Tcopy: Fail %s:%d\n",__FILE__,__LINE__);
    goto cleanup;
  }
  
  buffer_type_id = mitype_to_hdftype(buffer_data_type, TRUE);
  if(volume_type_id<0)
  {
    fprintf(stderr,"mitype_to_hdftype: Fail %s:%d\n",__FILE__,__LINE__);
    goto cleanup;
  }
  
  ndims = volume->number_of_dims;
  
  if (ndims == 0) {
    /* A scalar volume is possible but extremely unlikely, not to
     * mention useless!
     */
    mspc_id = H5Screate(H5S_SCALAR);
  } else {

    n_different = mitranslate_hyperslab_origin(volume,start,count, hdf_start,hdf_count,dir);

    mspc_id = H5Screate_simple(ndims, hdf_count, NULL);
    
    if (mspc_id < 0) {
      fprintf(stderr,"H5Screate_simple: Fail %s:%d\n",__FILE__,__LINE__);
      goto cleanup;
    }
  }
  
  miget_hyperslab_size_hdf(buffer_type_id,ndims,hdf_count,&buffer_size);

  result = H5Sselect_hyperslab(fspc_id, H5S_SELECT_SET, hdf_start, NULL,
                               hdf_count, NULL);
  if (result < 0) {
    fprintf(stderr,"H5Sselect_hyperslab: Fail %s:%d\n",__FILE__,__LINE__);
    goto cleanup;
  }

  miget_volume_valid_range( volume, &volume_valid_max, &volume_valid_min);

#ifdef _DEBUG
  printf("mirw_hyperslab_icv:Volume:%x valid_max:%f valid_min:%f scaling:%d\n",volume,volume_valid_max,volume_valid_min,volume->has_slice_scaling);
#endif  
  
  if(volume->has_slice_scaling)
  {
    hid_t image_max_fspc_id;
    hid_t image_min_fspc_id;
    hid_t scaling_mspc_id;
    int slice_buffer_size=1;

    image_max_fspc_id=H5Dget_space(volume->imax_id);
    image_min_fspc_id=H5Dget_space(volume->imin_id);

    if ( image_max_fspc_id < 0 ) {
      /*Report error that image-max is not found!*/
      return ( MI_ERROR );
    }

    slice_ndims = H5Sget_simple_extent_ndims ( image_max_fspc_id );
    if(slice_ndims<0)
    {
      /*TODO: report read error somehow*/
      fprintf(stderr,"H5Sget_simple_extent_ndims: Fail %s:%d\n",__FILE__,__LINE__);
      goto cleanup;
    }

    if ( slice_ndims > ndims ) { /*Can this really happen?*/
      slice_ndims = slice_ndims;
    }

    for ( i = 0; i < slice_ndims; i++ ) {
      image_slice_count[i] = hdf_count[i];
      image_slice_start[i] = hdf_start[i];
      
      if(hdf_count[i]>1) /*avoid zero sized dimensions?*/
        slice_buffer_size*=hdf_count[i];
    }
    total_number_of_slices=slice_buffer_size;
    image_slice_length=1;
    
    for (i = slice_ndims; i < ndims; i++ ) {
      if(hdf_count[i]>1) /*avoid zero sized dimensions?*/
        image_slice_length*=hdf_count[i];
      
      image_slice_count[i] = 0;
      image_slice_start[i] = 0;
    }
    image_slice_max_buffer=malloc(slice_buffer_size*sizeof(double));
    image_slice_min_buffer=malloc(slice_buffer_size*sizeof(double));
    /*TODO check for allocation failure ?*/
    
    scaling_mspc_id = H5Screate_simple(slice_ndims, image_slice_count, NULL);
    
    if( H5Sselect_hyperslab(image_max_fspc_id, H5S_SELECT_SET, image_slice_start, NULL, image_slice_count, NULL)>=0 )
    {
      if(H5Dread(volume->imax_id, H5T_NATIVE_DOUBLE, scaling_mspc_id, image_max_fspc_id, H5P_DEFAULT,image_slice_max_buffer)<0)
      {
        /*TODO: report read error somehow*/
        fprintf(stderr,"H5Dread: Fail %s:%d\n",__FILE__,__LINE__);
        goto cleanup;
      }
    } else {
      /*TODO: report read error somehow*/
      fprintf(stderr,"H5Sselect_hyperslab: Fail %s:%d\n",__FILE__,__LINE__);
      goto cleanup;
    }
    
    if( H5Sselect_hyperslab(image_min_fspc_id, H5S_SELECT_SET, image_slice_start, NULL, image_slice_count, NULL)>=0 )
    {
      if(H5Dread(volume->imin_id, H5T_NATIVE_DOUBLE, scaling_mspc_id, image_min_fspc_id, H5P_DEFAULT,image_slice_max_buffer)<0)
      {
        /*TODO: report read error somehow*/
        fprintf(stderr,"H5Dread: Fail %s:%d\n",__FILE__,__LINE__);
        goto cleanup;
      }
    } else {
      /*TODO: report read error somehow*/
      fprintf(stderr,"H5Sselect_hyperslab: Fail %s:%d\n",__FILE__,__LINE__);
      goto cleanup;
    }
    H5Sclose(scaling_mspc_id);
    H5Sclose(image_max_fspc_id);
  } else {
    slice_ndims=0;
    total_number_of_slices=1;
    image_slice_max_buffer=malloc(sizeof(double));
    image_slice_min_buffer=malloc(sizeof(double));
    miget_volume_range(volume,image_slice_max_buffer,image_slice_min_buffer);
    image_slice_length=1;
    
    for (i = 0; i < ndims; i++) {
      image_slice_length *= hdf_count[i];
    }
#ifdef _DEBUG    
    printf("mirw_hyperslab_icv:Real max:%f min:%f\n",*image_slice_max_buffer,*image_slice_min_buffer);
#endif    
  }
#ifdef _DEBUG  
  printf("mirw_hyperslab_icv:Slice_ndim:%d total_number_of_slices:%d image_slice_length:%d\n",slice_ndims,total_number_of_slices,image_slice_length);
#endif

  if (opcode == MIRW_OP_READ) 
  {
    result = H5Dread(dset_id, buffer_type_id, mspc_id, fspc_id, H5P_DEFAULT, buffer);
    if(result<0)
    {
      /*TODO: report read error somehow*/
      fprintf(stderr,"H5Dread: Fail %s:%d\n",__FILE__,__LINE__);
      goto cleanup;
    }
    
    /* TODO: apply interslice scaling here*/
    /* TODO: if output data type is not float or double - complain!*/
    /* Restructure the array after reading the data in file orientation.
    */
    if(volume->has_slice_scaling || buffer_data_type==MI_TYPE_FLOAT || buffer_data_type==MI_TYPE_DOUBLE)
    /*Figure out when scaling is actually needed!*/
    {
      
      switch(buffer_data_type)
      {
        case MI_TYPE_FLOAT:
          APPLY_DESCALING(float,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_DOUBLE:
          APPLY_DESCALING(double,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_INT:
          APPLY_DESCALING(int,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_UINT:
          APPLY_DESCALING(unsigned int,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_SHORT:
          APPLY_DESCALING(short,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_USHORT:
          APPLY_DESCALING(unsigned short,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_BYTE:
          APPLY_DESCALING(char,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_UBYTE:
          APPLY_DESCALING(unsigned char,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        default:
          /*TODO: report unsupported conversion*/
          result=MI_ERROR;
          goto cleanup;
      }
    }
    
    if (n_different != 0 ) {
      restructure_array(ndims, buffer, count, H5Tget_size(buffer_type_id),volume->dim_indices, dir);
      /*TODO: check if we managed to restructure the array*/
      result=0;
    }
  } else { /*opcode != MIRW_OP_READ*/

    volume->is_dirty = TRUE; /* Mark as modified. */
    
    if (n_different != 0 ) {
      /* Invert before calling */
      for (i = 0; i < ndims; i++) {
        icount[volume->dim_indices[i]] = count[i];
        idir[volume->dim_indices[i]] = dir[i];
        /* this one was correct the original way*/
        imap[volume->dim_indices[i]] = i;

      }
    }
    /* TODO: apply interslice scaling here*/
    /* TODO: if output data type is not float or double - complain!*/
    /* Restructure the array after reading the data in file orientation.
    */
    
    if (n_different != 0 ) 
    {
      /*TODO: don't touch the input ARRAY!*/
      /* Restructure array before writing to file.
      */
      restructure_array(ndims, buffer, icount, H5Tget_size(buffer_type_id), imap, idir);
    }

    if(volume->has_slice_scaling || buffer_data_type==MI_TYPE_FLOAT || buffer_data_type==MI_TYPE_DOUBLE)
    /*Figure out when scaling is actually needed!*/
    {
      switch(buffer_data_type)
      {
        case MI_TYPE_FLOAT:
          APPLY_SCALING(float,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_DOUBLE:
          APPLY_SCALING(double,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_INT:
          APPLY_SCALING(int,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_UINT:
          APPLY_SCALING(unsigned int,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_SHORT:
          APPLY_SCALING(short,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_USHORT:
          APPLY_SCALING(unsigned short,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_BYTE:
          APPLY_SCALING(char,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        case MI_TYPE_UBYTE:
          APPLY_SCALING(unsigned char,buffer,image_slice_length,total_number_of_slices,image_slice_min_buffer,image_slice_max_buffer,volume_valid_min,volume_valid_max);
          break;
        default:
          /*TODO: report unsupported conversion*/
          result=MI_ERROR;
          goto cleanup;
      }
    }
    result = H5Dwrite(dset_id, buffer_type_id, mspc_id, fspc_id, H5P_DEFAULT, buffer);
    if(result<0)
    {
      /*TODO: report write error somehow*/
      fprintf(stderr,"H5Dwrite: Fail %s:%d\n",__FILE__,__LINE__);
      goto cleanup;
    }
  }
      
cleanup:

  if (volume_type_id >= 0) {
    H5Tclose(volume_type_id);
  }
  if (buffer_type_id >= 0) {
    H5Tclose(buffer_type_id);
  }
  if (mspc_id >= 0) {
    H5Sclose(mspc_id);
  }
  if (fspc_id >= 0) {
    H5Sclose(fspc_id);
  }
  if(temp_buffer!=NULL)
  {
    free(temp_buffer);
  }
  if(image_slice_min_buffer!=NULL)
  {
    free(image_slice_min_buffer);
  }
  if(image_slice_max_buffer!=NULL)
  {
    free(image_slice_max_buffer);
  }
  return (result);
}

/** Reads the real values in the volume from the interval min through
 *  max, mapped to the maximum representable range for the requested
 *  data type. Float type is NOT an allowed data type.
 */
int miget_hyperslab_normalized(mihandle_t volume,
                               mitype_t buffer_data_type,
                               const unsigned long start[],
                               const unsigned long count[],
                               double min,
                               double max,
                               void *buffer)
{
  char path[MI2_MAX_PATH];
  hid_t dset_id;
  hid_t fspc_id;
  int icv;
  int result;
  int is_signed;

  if (min > max) {
    return (MI_ERROR);
  }

  sprintf(path, "/minc-2.0/image/%d/image", volume->selected_resolution);
  
  /* Open the dataset with the specified path
  */
  dset_id = H5Dopen1(volume->hdf_id, path);
  if (dset_id < 0) {
    return (MI_ERROR);
  }
  /* Get an Id to the copy of the dataspace */
  fspc_id = H5Dget_space(dset_id);
  
  if (fspc_id >= 0) 
  {
    if ( buffer_data_type == MI_TYPE_FLOAT || buffer_data_type == MI_TYPE_DOUBLE ) {
      H5Dclose(dset_id);
      return (MI_ERROR);
    }

    result=mirw_hyperslab_icv(MIRW_OP_READ, volume, buffer_data_type, start, count, buffer);
    
    H5Dclose(dset_id);
    
    return result;
  } else { 
    //TODO: add diagnostics here
    return (MI_ERROR);
  }

}

/** Get a hyperslab from the file, with the assistance of a MINC2 image
 * conversion variable (ICV).
 */
int miget_hyperslab_with_icv(mihandle_t volume,           /**< A MINC 2.0 volume handle */
                             mitype_t buffer_data_type,   /**< Output datatype */
                             const unsigned long start[], /**< Start coordinates  */
                             const unsigned long count[], /**< Lengths of edges  */
                             void *buffer)                /**< Output memory buffer */
{
  return mirw_hyperslab_icv(MIRW_OP_READ, volume, buffer_data_type, start, count,buffer);
}

/** Write a hyperslab to the file, with the assistance of a MINC image
 * conversion variable (ICV).
 */
int miset_hyperslab_with_icv(mihandle_t volume,        /**< A MINC 2.0 volume handle */
                         mitype_t buffer_data_type,    /**< Output datatype */
                         const unsigned long start[],  /**< Start coordinates  */
                         const unsigned long count[],  /**< Lengths of edges  */
                         void *buffer)                 /**< Output memory buffer */
{
  return  mirw_hyperslab_icv(MIRW_OP_WRITE,volume,buffer_data_type,start,count,buffer);
}

/** Read a hyperslab from the file into the preallocated buffer,
 *  converting from the stored "voxel" data range to the desired
 * "real" (float or double) data range.
 */
int miget_real_value_hyperslab(mihandle_t volume,       /**< A MINC 2.0 volume handle */
                           mitype_t buffer_data_type,   /**< Output datatype    */
                           const unsigned long start[], /**< Start coordinates  */
                           const unsigned long count[], /**< Lengths of edges   */
                           void *buffer)                /**< Output memory buffer */ 
{
  hid_t file_id;
  int var_id;
  int icv;
  int result;
  int is_signed;
  int nctype;
  int i;

  midimhandle_t hdim;

  //figure out whether we need to flip image    L.B May 18/2011
  for (i=0; i < volume->number_of_dims ; i++) {
    hdim = volume->dim_handles[i];
    switch (hdim->flipping_order) {
      // TODO: fix this
    case MI_FILE_ORDER:
//       mi2_icv_setint(icv, MI2_ICV_DO_DIM_CONV, FALSE);
      printf("Dim %d flip:File order is expected\n",i);
      break;
    case MI_COUNTER_FILE_ORDER:
      printf("Dim %d flip:MI_COUNTER_FILE_ORDER is expected\n",i);
      break;
    case MI_POSITIVE:
//       if (hdim->step < 0)
//         mi2_icv_setint(icv, MI2_ICV_DO_DIM_CONV, TRUE);
      printf("Dim %d flip:MI_POSITIVE is expected\n",i);
      
      break;
    case MI_NEGATIVE:
//       if (hdim->step > 0)
//         mi2_icv_setint(icv, MI2_ICV_DO_DIM_CONV, TRUE);
      printf("Dim flip:MI_NEGATIVE is expected\n",i);
      break;
    default:
      return;
    }
  }
  result = mirw_hyperslab_icv(MIRW_OP_READ,
                              volume,
                              buffer_data_type,
                              start,
                              count,
                              (void *) buffer);
  return (result);
}

/** Write a hyperslab to the file from the preallocated buffer,
 *  converting from the stored "voxel" data range to the desired
 * "real" (float or double) data range.
 */
int
miset_real_value_hyperslab(mihandle_t volume,
                           mitype_t buffer_data_type,
                           const unsigned long start[],
                           const unsigned long count[],
                           void *buffer)
{
  return mirw_hyperslab_icv(MIRW_OP_WRITE,
                                volume,
                                buffer_data_type,
                                start,
                                count,
                                (void *) buffer);
}

/** Read a hyperslab from the file into the preallocated buffer,
 * with no range conversions or normalization.  Type conversions will
 * be performed if necessary.
 */
int miget_voxel_value_hyperslab(mihandle_t volume,
                            mitype_t buffer_data_type,
                            const unsigned long start[],
                            const unsigned long count[],
                            void *buffer)
{
  return mirw_hyperslab_raw(MIRW_OP_READ, volume, buffer_data_type,
                            start, count, buffer);
}

/** Write a hyperslab to the file from the preallocated buffer,
 * with no range conversions or normalization.  Type conversions will
 * be performed if necessary.
 */
int miset_voxel_value_hyperslab(mihandle_t volume,
                            mitype_t buffer_data_type,
                            const unsigned long start[],
                            const unsigned long count[],
                            void *buffer)
{
  return mirw_hyperslab_raw(MIRW_OP_WRITE, volume, buffer_data_type,
                            start, count, (void *) buffer);
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 
