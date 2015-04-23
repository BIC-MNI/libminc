/**
 * \file Reader for MGH/MGZ (FreeSurfer) format files.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include  <internal_volume_io.h>

#include <arpa/inet.h> /* for ntohl and ntohs */

#define NUM_BYTE_VALUES      (UCHAR_MAX + 1)

#define MGH_MAX_DIMS 4          /* Maximum number of dimensions */
#define MGH_N_SPATIAL VIO_N_DIMENSIONS /* Number of spatial dimensions */
#define MGH_N_COMPONENTS 4      /* Number of transform components. */
#define MGH_N_XFORM (MGH_N_COMPONENTS * MGH_N_SPATIAL)

#define MGH_HEADER_SIZE 284 /* Total number of bytes in the header. */

#define MGH_EXTRA_SIZE 194   /* Number of "unused" bytes in the header. */

#define MGH_TYPE_UCHAR 0  /**< Voxels are 1-byte unsigned integers. */
#define MGH_TYPE_INT 1    /**< Voxels are 4-byte signed integers. */
#define MGH_TYPE_LONG 2   /**< Unsupported here.  */
#define MGH_TYPE_FLOAT 3  /**< Voxels are 4-byte floating point. */
#define MGH_TYPE_SHORT 4  /**< Voxels are 2-byte signed integers. */
#define MGH_TYPE_BITMAP 5 /**< Unsupported here. */
#define MGH_TYPE_TENSOR 6 /**< Unsupported here. */

/**
 * Information in the MGH/MGZ file header.
 */
struct mgh_header {
  int version;                  /**< Must be 0x00000001. */
  int sizes[MGH_MAX_DIMS];      /**< Dimension sizes, fastest-varying FIRST. */
  int type;                     /**< One of the MGH_TYPE_xxx values. */
  int dof;                      /**< Degrees of freedom, if used. */
  short goodRASflag;            /**< True if spacing and dircos valid. */
  float spacing[MGH_N_SPATIAL]; /**< Dimension spacing.  */
  float dircos[MGH_N_COMPONENTS][MGH_N_SPATIAL]; /**< Dimension transform. */
};

/**
 * Trailer information found immediately AFTER the hyperslab of data.
 */
struct mgh_trailer {
  float TR;
  float FlipAngle;
  float TE;
  float TI;
  float FoV;
};

/**
 * Trivial function to swap floats if necessary.
 * \param f The big-endian 4-byte float value.
 * \return The float value in host byte order.
 */
static float
swapFloat(float f)
{
  union {
    float f;
    int i;
  } sl;

  sl.f = f;

  sl.i = ntohl(sl.i);

  return sl.f;
}

/**
 * Reads the next slice from the MGH volume. As a side effect, it advances
 * the slice_index value in the volume input structure if successful.
 *
 * \param in_ptr The volume input information.
 * \return VIO_OK if success
 */
static VIO_Status
input_next_slice(
                 volume_input_struct *in_ptr
                 )
{
  int n_voxels_in_slice;
  int n_bytes_per_voxel;

  if (in_ptr->slice_index >= in_ptr->sizes_in_file[2])
  {
    fprintf(stderr, "Read past final slice.\n");
    return VIO_ERROR;
  }

  n_bytes_per_voxel = get_type_size(in_ptr->file_data_type);
  n_voxels_in_slice = (in_ptr->sizes_in_file[0] *
                       in_ptr->sizes_in_file[1]);
  if (fread(in_ptr->byte_slice_buffer,
            n_bytes_per_voxel,
            n_voxels_in_slice,
            in_ptr->volume_file) != n_voxels_in_slice)
  {
    fprintf(stderr, "fread error %d\n", errno);
    return VIO_ERROR;
  }

  ++in_ptr->slice_index;
  return VIO_OK;
}

/**
 * Converts a MGH file header into a general linear transform for the
 * Volume IO library.
 *
 * \param hdr_ptr A pointer to the MGH header structure.
 * \param in_ptr A pointer to the input information for this volume.
 * \param linear_xform_ptr A pointer to the output transform
 * \returns void
 */
static void
mgh_header_to_linear_transform(const struct mgh_header *hdr_ptr,
                               const volume_input_struct *in_ptr,
                               struct VIO_General_transform *linear_xform_ptr)
{
  int           i, j;
  VIO_Transform mnc_xform;
  VIO_Real      mgh_xform[MGH_N_SPATIAL][MGH_N_COMPONENTS];
  
  make_identity_transform(&mnc_xform);

  /*
   * Use the MGH header to generate a linear transform.
   */
  for (i = 0; i < MGH_N_SPATIAL; i++) {
    for (j = 0; j < MGH_N_SPATIAL; j++) {
      mgh_xform[j][i] = hdr_ptr->dircos[i][j] * hdr_ptr->spacing[i];
    }
  }

  for (i = 0; i < MGH_N_SPATIAL; i++) {
    double temp = 0.0;
    for (j = 0; j < MGH_N_SPATIAL; j++) {
      temp += mgh_xform[i][j] * (hdr_ptr->sizes[j] / 2.0);
    }
    mgh_xform[i][MGH_N_COMPONENTS - 1] = hdr_ptr->dircos[MGH_N_COMPONENTS - 1][i] - temp;
  }

  printf("mgh_xform:\n");       /* DEBUG */
  for (i = 0; i < MGH_N_SPATIAL; i++) {
    for (j = 0; j < MGH_N_COMPONENTS; j++) {
      printf("%.4f ", mgh_xform[i][j]);
    }
    printf("\n");
  }

  /* Convert MGH transform to our format. The only difference is that
   * our transform is always written in XYZ (RAS) order, so we have to
   * swap the axes as needed.
   */
  for (i = 0; i < MGH_N_SPATIAL; i++) {
    int volume_axis = in_ptr->axis_index_from_file[i];
    for (j = 0; j < MGH_N_COMPONENTS; j++) {
      Transform_elem(mnc_xform, i, j) = mgh_xform[volume_axis][j];
    }
  }
  create_linear_transform(linear_xform_ptr, &mnc_xform);
}

/**
 * Skips over the MGH header, if, for example, we are re-reading
 * the file.
 * 
 * \param fp The open file stream (might be a pipe).
 * \returns TRUE if successful.
 */
static VIO_BOOL
mgh_header_skip(FILE *fp)
{
  char dummy[MGH_HEADER_SIZE];
  return fread(dummy, 1, MGH_HEADER_SIZE, fp) == MGH_HEADER_SIZE;
}

/**
 * Re-open an MGH file so that we can re-scan the whole thing.
 * We do this because we use "popen()" to open compressed files.
 * This requires that we simply close and reopen the file, rather
 * than trying to seek on a pipe.
 *
 * \param in_ptr The volume input information.
 * \param command The command to issue to read the file.
 * \returns TRUE if successful.
 */
static VIO_BOOL
mgh_reopen(volume_input_struct *in_ptr, const char *command)
{
  pclose(in_ptr->volume_file);

  if ((in_ptr->volume_file = popen(command, "r")) == NULL)
  {
    handle_internal_error("Failed to re-open MGH file for second scan.");
    return FALSE;
  }
  if (!mgh_header_skip(in_ptr->volume_file))
  {
    handle_internal_error("Failed to read past MGH file header.");
    return FALSE;
  }

  in_ptr->slice_index = 0;   /* Reset the slice index. */
  return TRUE;
}

/**
 * Read an MGH header from an open file stream.
 * \param fp The open file (may be a pipe).
 * \param hdr_ptr A pointer to the header structure to fill in.
 * \returns TRUE if successful.
 */
static VIO_BOOL
mgh_header_from_file(FILE *fp, struct mgh_header *hdr_ptr)
{
  int i, j;
  char dummy[MGH_HEADER_SIZE];

  /* Read in the header. We do this piecemeal because the mgh_header
   * struct will not be properly aligned on most systems, so a single
   * fread() WILL NOT WORK.
   */
  if (fread(&hdr_ptr->version, sizeof(int), 1, fp) != 1 ||
      fread(hdr_ptr->sizes, sizeof(int), MGH_MAX_DIMS, fp) != MGH_MAX_DIMS ||
      fread(&hdr_ptr->type, sizeof(int), 1, fp) != 1 ||
      fread(&hdr_ptr->dof, sizeof(int), 1, fp) != 1 || 
      fread(&hdr_ptr->goodRASflag, sizeof(short), 1, fp) != 1 ||
      /* The rest of the fields are optional, but we can safely read them
       * now and check goodRASflag later to see if we should really trust
       * them.
       */
      fread(hdr_ptr->spacing, sizeof(float), MGH_N_SPATIAL, fp) != MGH_N_SPATIAL ||
      fread(hdr_ptr->dircos, sizeof(float), MGH_N_XFORM, fp) != MGH_N_XFORM ||
      fread(dummy, 1, MGH_EXTRA_SIZE, fp) != MGH_EXTRA_SIZE)
  {
    print_error("Problem reading MGH file header.");
    return FALSE;
  }

  /* Successfully read all of the data. Now we have to convert it to the
   * machine's byte ordering.
   */
  hdr_ptr->version = ntohl(hdr_ptr->version);
  for (i = 0; i < MGH_MAX_DIMS; i++)
  {
    hdr_ptr->sizes[i] = ntohl(hdr_ptr->sizes[i]);
  }
  hdr_ptr->type = ntohl(hdr_ptr->type);
  hdr_ptr->dof = ntohl(hdr_ptr->dof);
  hdr_ptr->goodRASflag = ntohs(hdr_ptr->goodRASflag);

  if (hdr_ptr->version != 1)
  {
    print_error("Must be MGH version 1.\n");
    return FALSE;
  }

  if (hdr_ptr->goodRASflag)
  {
    for (i = 0; i < MGH_N_SPATIAL; i++)
    {
      hdr_ptr->spacing[i] = swapFloat(hdr_ptr->spacing[i]);
      for (j = 0; j < MGH_N_COMPONENTS; j++)
      {
        hdr_ptr->dircos[j][i] = swapFloat(hdr_ptr->dircos[j][i]);
      }
    }
  }
  else
  {
    /* Flag is zero, so just use the defaults.
     */
    for (i = 0; i < MGH_N_SPATIAL; i++)
    {
      /* Default spacing is 1.0.
       */
      hdr_ptr->spacing[i] = 1.0;

      /* Assume coronal orientation.
       */
      for (j = 0; j < MGH_N_COMPONENTS; j++) {
        hdr_ptr->dircos[j][i] = 0.0;
      }
      hdr_ptr->dircos[0][0] = -1.0;
      hdr_ptr->dircos[1][2] = -1.0;
      hdr_ptr->dircos[2][1] = 1.0;
    }
  }
  return TRUE;
}

static VIO_BOOL
mgh_scan_for_voxel_range(volume_input_struct *in_ptr, 
                         int n_voxels_in_slice,
                         float *min_value_ptr,
                         float *max_value_ptr)
{
  float min_value = FLT_MAX;
  float max_value = -FLT_MAX;
  int slice;
  float value;
  int i;

  for_less( slice, 0, in_ptr->sizes_in_file[2] )
  {
    input_next_slice( in_ptr );

    switch (in_ptr->file_data_type)
    {
    case VIO_UNSIGNED_BYTE:
      {
        unsigned char *byte_buffer_ptr = in_ptr->byte_slice_buffer;
        for_less(i, 0, n_voxels_in_slice)
        {
          value = *byte_buffer_ptr++;
          if (value < min_value )
            min_value = value;
          if (value > max_value )
            max_value = value;
        }
      }
      break;

    case VIO_SIGNED_SHORT:
      {
        short *short_buffer_ptr = (short *) in_ptr->byte_slice_buffer;
        for_less(i, 0, n_voxels_in_slice)
        {
          value = ntohs(*short_buffer_ptr++);
          if (value < min_value)
            min_value = value;
          if (value > max_value)
            max_value = value;
        }
      }
      break;

    case VIO_SIGNED_INT:
      {
        int *int_buffer_ptr = (int *) in_ptr->byte_slice_buffer;
        for_less(i, 0, n_voxels_in_slice)
        {
          value = ntohl(*int_buffer_ptr++);
          if (value < min_value)
            min_value = value;
          if (value > max_value)
            max_value = value;
        }
      }
      break;

    case VIO_FLOAT:
      {
        float *float_buffer_ptr = (float *) in_ptr->byte_slice_buffer;
        for_less(i, 0, n_voxels_in_slice)
        {
          value = swapFloat(*float_buffer_ptr++);
          if (value < min_value)
            min_value = value;
          if (value > max_value)
            max_value = value;
        }
      }
      break;
    }
  }

  printf("global min %f max %f\n", min_value, max_value); /* DEBUG */

  *min_value_ptr = min_value;
  *max_value_ptr = max_value;
  return TRUE;
}

/**
 * Initializes loading a MGH format file by reading the header.
 * This function assumes that volume->filename has been assigned.
 *
 * \param filename
 * \param volume
 * \param in_ptr
 * \return VIO_OK if successful.
 */
VIOAPI  VIO_Status
initialize_mgh_format_input(VIO_STR             filename,
                            VIO_Volume          volume,
                            volume_input_struct *in_ptr)
{
  VIO_Status        status;
  int               sizes[VIO_MAX_DIMENSIONS];
  int               i, j;
  int               n_voxels_in_slice;
  int               n_bytes_per_voxel;
  nc_type           desired_data_type;
  FILE              *fp;
  int               axis;
  char              command[VIO_EXTREMELY_LARGE_STRING_SIZE];
  struct mgh_header hdr;
  VIO_General_transform mnc_linear_xform;
  VIO_Real          mnc_dircos[VIO_N_DIMENSIONS][VIO_N_DIMENSIONS];
  VIO_Real          mnc_steps[VIO_MAX_DIMENSIONS];
  VIO_Real          mnc_starts[VIO_MAX_DIMENSIONS];
  int               n_dimensions;

  status = VIO_OK;

  if (string_ends_in(filename, "mgz"))
  {
    sprintf(command, "gzip -c -d %s", filename);
  }
  else
  {
    sprintf(command, "cat %s", filename);
  }

  if ((fp = popen(command, "r")) == NULL)
  {
    print_error("Unable to open file %s, errno %d.\n", filename, errno);
    return VIO_ERROR;
  }

  if (!mgh_header_from_file(fp, &hdr))
  {
    pclose(fp);
    return VIO_ERROR;
  }

  /* Translate from MGH to VIO types.
   */
  switch (hdr.type)
  {
  case MGH_TYPE_UCHAR:
    in_ptr->file_data_type = VIO_UNSIGNED_BYTE;
    break;
  case MGH_TYPE_INT:
    in_ptr->file_data_type = VIO_SIGNED_INT;
    break;
  case MGH_TYPE_FLOAT:
    in_ptr->file_data_type = VIO_FLOAT;
    break;
  case MGH_TYPE_SHORT:
    in_ptr->file_data_type = VIO_SIGNED_SHORT;
    break;
  default:
    print_error("Unknown MGH data type.\n");
    pclose(fp);
    return VIO_ERROR;
  }

  /* Decide how to store data in memory. */

  if ( get_volume_data_type(volume) == VIO_NO_DATA_TYPE )
    desired_data_type = NC_BYTE;
  else
    desired_data_type = volume->nc_data_type;

  if( volume->spatial_axes[VIO_X] < 0 ||
      volume->spatial_axes[VIO_Y] < 0 ||
      volume->spatial_axes[VIO_Z] < 0 )
  {
    print_error("warning: setting MGH spatial axes to XYZ.\n");
    volume->spatial_axes[VIO_X] = 0;
    volume->spatial_axes[VIO_Y] = 1;
    volume->spatial_axes[VIO_Z] = 2;
  }

  n_dimensions = 0;
  for_less( axis, 0, MGH_MAX_DIMS )
  {
    in_ptr->sizes_in_file[axis] = hdr.sizes[axis];
    if (hdr.sizes[axis] > 1)
    {
      n_dimensions++;
    }
  }

  if (!set_volume_n_dimensions(volume, n_dimensions))
  {
    printf("Problem setting number of dimensions to %d\n", n_dimensions);
  }

  /* Figure out which of the logical axes corresponds to the typical
   * world spatial dimensions.
   */
  for_less( axis, 0, VIO_N_DIMENSIONS )
  {
    int major_axis = VIO_X;

    if (fabs(hdr.dircos[axis][1]) > fabs(hdr.dircos[axis][0]) &&
        fabs(hdr.dircos[axis][1]) > fabs(hdr.dircos[axis][2]))
    {
      major_axis = VIO_Y;
    }
    if (fabs(hdr.dircos[axis][2]) > fabs(hdr.dircos[axis][0]) &&
        fabs(hdr.dircos[axis][2]) > fabs(hdr.dircos[axis][1]))
    {
      major_axis = VIO_Z;
    }
    in_ptr->axis_index_from_file[axis] = major_axis;
  }

  mgh_header_to_linear_transform(&hdr, in_ptr, &mnc_linear_xform);

  convert_transform_to_starts_and_steps(&mnc_linear_xform,
                                        VIO_N_DIMENSIONS,
                                        NULL,
                                        volume->spatial_axes,
                                        mnc_starts,
                                        mnc_steps,
                                        mnc_dircos);
  for_less( axis, 0, VIO_N_DIMENSIONS)
  {
    int volume_axis = in_ptr->axis_index_from_file[axis];

    sizes[volume_axis] = in_ptr->sizes_in_file[axis];

    /* DEBUG */
    printf("%d %d size:%4d step:%6.3f start:%9.4f dc:[%7.4f %7.4f %7.4f]\n", 
           axis,
           volume_axis,
           sizes[volume_axis],
           mnc_steps[volume_axis],
           mnc_starts[volume_axis],
           mnc_dircos[volume_axis][0], 
           mnc_dircos[volume_axis][1], 
           mnc_dircos[volume_axis][2]);

    set_volume_direction_cosine(volume, volume_axis, mnc_dircos[axis]);
  }

  set_volume_separations( volume, mnc_steps );
  set_volume_starts( volume, mnc_starts );

  /* If we are a 4D image, we need to copy the size here.
   */
  sizes[3] = in_ptr->sizes_in_file[3];

  set_volume_type( volume, desired_data_type, FALSE, 0.0, 0.0 );
  set_volume_sizes( volume, sizes );

  n_bytes_per_voxel = get_type_size( in_ptr->file_data_type );

  n_voxels_in_slice = (in_ptr->sizes_in_file[0] *
                       in_ptr->sizes_in_file[1]);

  /* Allocate the slice buffer. */

  ALLOC(in_ptr->byte_slice_buffer, n_voxels_in_slice * n_bytes_per_voxel);

  in_ptr->volume_file = fp;

  in_ptr->slice_index = 0;

  /* If the data must be converted to byte, read the entire image file simply
   * to find the max and min values. This allows us to set the value_scale and
   * value_translation properly when we read the file.
   */
  if (get_volume_data_type(volume) != in_ptr->file_data_type )
  {
    float min_value, max_value;

    mgh_scan_for_voxel_range(in_ptr, n_voxels_in_slice, &min_value, &max_value);
    set_volume_voxel_range(volume, min_value, max_value);

    /* Reinitialize the whole process, by closing the existing pipe
     * and re-opening it, so we can scan through it again.
     */
    if (!mgh_reopen(in_ptr, command))
    {
      return VIO_ERROR;
    }
  }

  return VIO_OK;
}

/**
 * Dispose of the resources used to read an MGH file.
 * \param in_ptr
 * \return Nothing.
 */
VIOAPI void
delete_mgh_format_input(
                        volume_input_struct   *in_ptr
                        )
{
  FREE( in_ptr->byte_slice_buffer );

  pclose( in_ptr->volume_file );
}

/**
 * Read the next slice of an MGH (MGZ) format file.
 * \param volume
 * \param in_ptr
 * \param fraction_done
 * \return TRUE if successful.
 */
VIOAPI  VIO_BOOL
input_more_mgh_format_file(
                           VIO_Volume          volume,
                           volume_input_struct *in_ptr,
                           VIO_Real            *fraction_done
                           )
{
  int        i;
  VIO_Real   value;
  VIO_Status status;
  VIO_Real   value_translation, value_scale;
  VIO_Real   original_min_voxel, original_max_voxel;
  int        *inner_index, indices[VIO_MAX_DIMENSIONS];

  if ( in_ptr->slice_index < in_ptr->sizes_in_file[2] )
  {
    /* If the memory for the volume has not been allocated yet,
     * initialize that memory now.
     */
    if (!volume_is_alloced(volume))
    {
      alloc_volume_data(volume);
      if (!volume_is_alloced(volume))
      {
        print_error("Failed to allocate volume.\n");
        return FALSE;
      }
    }

    status = input_next_slice( in_ptr );

    /* See if we need to apply scaling to this slice. This is only
     * needed if the volume voxel type is not the same as the file
     * voxel type. THIS IS ONLY REALLY LEGAL FOR BYTE VOLUME TYPES.
     */
    if (get_volume_data_type(volume) != in_ptr->file_data_type)
    {
      get_volume_voxel_range(volume, &original_min_voxel, &original_max_voxel);
      value_translation = original_min_voxel;
      value_scale = (original_max_voxel - original_min_voxel) /
        (VIO_Real) (NUM_BYTE_VALUES - 1);
    }
    else
    {
      /* Just do the trivial scaling. */
      value_translation = 0.0;
      value_scale = 1.0;
    }

    /* Set up the indices.
     */
    inner_index = &indices[in_ptr->axis_index_from_file[0]];

    indices[in_ptr->axis_index_from_file[2]] = in_ptr->slice_index - 1;

    if ( status == VIO_OK )
    {
      switch ( in_ptr->file_data_type )
      {
      case VIO_UNSIGNED_BYTE:
        {
          unsigned char *byte_buffer_ptr = in_ptr->byte_slice_buffer;
          for_less( i, 0, in_ptr->sizes_in_file[1] )
          {
            indices[in_ptr->axis_index_from_file[1]] = i;
            for_less( *inner_index, 0, in_ptr->sizes_in_file[0] )
            {
              unsigned char v = *byte_buffer_ptr++;
              value = ((VIO_Real) v - value_translation) / value_scale;
              if ( value < 0.0 )
                value = 0.0;
              else if ( value > 255.0 )
                value = 255.0;

              set_volume_voxel_value( volume,
                                      indices[VIO_X],
                                      indices[VIO_Y],
                                      indices[VIO_Z],
                                      0,
                                      0,
                                      value);

            }
          }
        }
        break;

      case VIO_SIGNED_SHORT:
        {
          short *short_buffer_ptr = (short *) in_ptr->byte_slice_buffer;
          for_less( i, 0, in_ptr->sizes_in_file[1] )
          {
            indices[in_ptr->axis_index_from_file[1]] = i;
            for_less( *inner_index, 0, in_ptr->sizes_in_file[0])
            {
              short v = ntohs(*short_buffer_ptr++);
              value = ((VIO_Real) v - value_translation) / value_scale;

              set_volume_voxel_value( volume,
                                      indices[VIO_X],
                                      indices[VIO_Y],
                                      indices[VIO_Z],
                                      0,
                                      0,
                                      value );

            }
          }
        }
        break;

      case VIO_SIGNED_INT:
        {
          int *int_buffer_ptr = (int *) in_ptr->byte_slice_buffer;
          for_less( i, 0, in_ptr->sizes_in_file[1] )
          {
            indices[in_ptr->axis_index_from_file[1]] = i;
            for_less( *inner_index, 0, in_ptr->sizes_in_file[0])
            {
              int v = ntohl(*int_buffer_ptr++);
              value = ((VIO_Real) v - value_translation) / value_scale;
              set_volume_voxel_value( volume,
                                      indices[VIO_X],
                                      indices[VIO_Y],
                                      indices[VIO_Z],
                                      0,
                                      0,
                                      value );
            }
          }
        }
        break;
      case VIO_FLOAT:
        {
          float *float_buffer_ptr = (float *) in_ptr->byte_slice_buffer;
          for_less( i, 0, in_ptr->sizes_in_file[1] )
          {
            indices[in_ptr->axis_index_from_file[1]] = i;
            for_less( *inner_index, 0, in_ptr->sizes_in_file[0])
            {
              float v = swapFloat(*float_buffer_ptr++);
              value = ((VIO_Real) v - value_translation) / value_scale;
              set_volume_voxel_value(volume,
                                     indices[VIO_X],
                                     indices[VIO_Y],
                                     indices[VIO_Z],
                                     0,
                                     0,
                                     value );

            }
          }
        }
        break;

      default:
        handle_internal_error( "input_more_mgh_format_file" );
        break;
      }
    }
  }

  *fraction_done = (VIO_Real) in_ptr->slice_index / in_ptr->sizes_in_file[2];

  /* See if we are all done. If so, we need to perform a final check
   * of the volume to set the ranges appropriately.
   */
  if (in_ptr->slice_index == in_ptr->sizes_in_file[2])
  {
    int      x, y, z;                   /* Loop counters. */
    int      sizes[VIO_MAX_DIMENSIONS]; /* Sizes. */
    VIO_Real min_value, max_value;      /* For range checking. */

    get_volume_sizes( volume, sizes );

    min_value = get_volume_voxel_value( volume, 0, 0, 0, 0, 0 );
    max_value = min_value;
    for_less( x, 0, sizes[VIO_X] )
    {
      for_less( y, 0, sizes[VIO_Y] )
      {
        for_less( z, 0, sizes[VIO_Z] )
        {
          value = get_volume_voxel_value(volume, x, y, z, 0, 0);
          if (value < min_value)
            min_value = value;
          if (value > max_value)
            max_value = value;
        }
      }
    }

    set_volume_voxel_range( volume, min_value, max_value );

    /* Make sure we scale the data up to the original real range,
     * if appropriate.
     */
    if (get_volume_data_type(volume) != in_ptr->file_data_type)
    {
      set_volume_real_range(volume, original_min_voxel, original_max_voxel);
    }

    return FALSE;
  }
  else
  {
    return TRUE;                /* More work to do. */
  }
}
