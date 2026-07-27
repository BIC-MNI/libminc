/* nifti_readback.c
 *
 * Functional guard for the NIfTI name-mangling change: write a small NIfTI-1
 * volume via the (mangled) nifti_clib API, then read it back through libminc's
 * volume_io input_volume() path.  This links minc2 (so the nifti reader chain
 * is pulled in) and uses the nifti API directly (so it goes through the
 * mangled headers).  It must pass identically before and after mangling --
 * proving the prefix rename did not break NIfTI support.
 *
 * It also serves as the object the `nifti_mangle_symbols` test inspects with
 * nm: because it pulls the whole reader chain, its symbol table shows whether
 * libminc's nifti implementation is exported mangled (minc_nifti_*) or not.
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nifti1_io.h>
#include <volume_io.h>

#define NX 5
#define NY 6
#define NZ 7

static int write_test_nii(const char *fname)
{
  nifti_1_header hdr;
  nifti_image *nim;
  int i, nvox;

  memset(&hdr, 0, sizeof(hdr));
  hdr.sizeof_hdr = sizeof(hdr);
  hdr.regular = 'r';
  hdr.dim[0] = 3;
  hdr.dim[1] = NX;
  hdr.dim[2] = NY;
  hdr.dim[3] = NZ;
  hdr.dim[4] = 1;
  hdr.dim[5] = 1;
  hdr.dim[6] = 1;
  hdr.dim[7] = 1;
  hdr.datatype = DT_INT32;
  hdr.pixdim[0] = 1.0F;
  hdr.pixdim[1] = 1.0F;
  hdr.pixdim[2] = 1.0F;
  hdr.pixdim[3] = 1.0F;
  hdr.vox_offset = 352;             /* single-file .nii */
  hdr.sform_code = NIFTI_XFORM_SCANNER_ANAT;
  hdr.srow_x[0] = 1.0F; hdr.srow_x[3] = 0.0F;
  hdr.srow_y[1] = 1.0F; hdr.srow_y[3] = 0.0F;
  hdr.srow_z[2] = 1.0F; hdr.srow_z[3] = 0.0F;
  hdr.magic[0] = 'n'; hdr.magic[1] = '+'; hdr.magic[2] = '1'; hdr.magic[3] = '\0';
  {
    int nbyper, swapsize;
    nifti_datatype_sizes(hdr.datatype, &nbyper, &swapsize);
    hdr.bitpix = nbyper * 8;
  }

  nim = nifti_convert_nhdr2nim(hdr, fname);
  if (nim == NULL) { fprintf(stderr, "convert_nhdr2nim failed\n"); return 1; }

  nvox = nim->nx * nim->ny * nim->nz;
  nim->data = calloc((size_t)nvox, sizeof(int));
  if (nim->data == NULL) { fprintf(stderr, "calloc failed\n"); return 1; }
  for (i = 0; i < nvox; i++) ((int *)nim->data)[i] = i;

  if (nifti_set_filenames(nim, fname, 0, 0) != 0) {
    fprintf(stderr, "set_filenames failed\n"); return 1;
  }
  nifti_image_write(nim);
  nifti_image_free(nim);
  return 0;
}

int main(int argc, char **argv)
{
  const char *fname = (argc > 1) ? argv[1] : "nifti_readback_tmp.nii";
  VIO_Volume vol = NULL;
  VIO_Status status;
  int sizes[VIO_MAX_DIMENSIONS];

  if (write_test_nii(fname) != 0) {
    fprintf(stderr, "FAILED to write test .nii\n");
    return 1;
  }

  status = input_volume((VIO_STR)fname, 3, NULL,
                        MI_ORIGINAL_TYPE, FALSE, 0.0, 0.0,
                        TRUE, &vol, NULL);
  if (status != VIO_OK || vol == NULL) {
    fprintf(stderr, "FAILED: input_volume returned %d on %s\n", (int)status, fname);
    return 1;
  }

  get_volume_sizes(vol, sizes);
  /* libminc may permute axes; just require the voxel count to match. */
  if ((size_t)sizes[0] * sizes[1] * sizes[2] != (size_t)NX * NY * NZ) {
    fprintf(stderr, "FAILED: read sizes %d,%d,%d (voxels %d) != expected %d\n",
            sizes[0], sizes[1], sizes[2], sizes[0]*sizes[1]*sizes[2], NX*NY*NZ);
    delete_volume(vol);
    return 1;
  }

  delete_volume(vol);
  printf("nifti_readback OK: read %dx%dx%d NIfTI via libminc\n", sizes[0], sizes[1], sizes[2]);
  return 0;
}
