/**
 * \file Reader for NIfTI-1 format files.
 */

#include <internal_volume_io.h>
#include <volume_io/basic.h>
#include <volume_io/volume.h>

/**
 * Initializes loading a NIfTI-1 format file by reading the header.
 * This function assumes that volume->filename has been assigned.
 *
 * \param filename
 * \param volume
 * \param in_ptr
 * \return VIO_OK if successful.
 */
VIOAPI  VIO_Status
initialize_nifti_format_input(VIO_STR             filename,
                              VIO_Volume          volume,
                              volume_input_struct *in_ptr);


/**
 * Dispose of the resources used to read a NIfTI-1 file.
 * \param in_ptr
 * \return Nothing.
 */
VIOAPI void
delete_nifti_format_input(
                          volume_input_struct   *in_ptr
                          );


/**
 * Read the next slice of an NIfTI-1 format file.
 * \param volume
 * \param in_ptr
 * \param fraction_done
 * \return TRUE if successful.
 */
VIOAPI  VIO_BOOL
input_more_nifti_format_file(
                             VIO_Volume          volume,
                             volume_input_struct *in_ptr,
                             VIO_Real            *fraction_done
                             );
