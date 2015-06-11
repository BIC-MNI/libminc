/**
 * \file Reader for MGH/MGZ (FreeSurfer) format files.
 */

#include <internal_volume_io.h>
#include <volume_io/basic.h>
#include <volume_io/volume.h>

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
                            volume_input_struct *in_ptr);


/**
 * Dispose of the resources used to read an MGH file.
 * \param in_ptr
 * \return Nothing.
 */
VIOAPI void
delete_mgh_format_input(
                        volume_input_struct   *in_ptr
                        );


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
                           );
