/* ----------------------------- MNI Header -----------------------------------
@NAME       : minc2_icv.c
@DESCRIPTION: File of functions to manipulate image conversion variables
              (icv). These variables allow conversion of netcdf variables
              (the MINC image variable, in particular) to a form more
              convenient for a program.
@METHOD     : Routines included in this file :
              public :
                 mi2_icv_create
                 mi2_icv_free
                 mi2_icv_setdbl
                 mi2_icv_setint
                 mi2_icv_setlong
                 mi2_icv_setstr
                 mi2_icv_inqdbl
                 mi2_icv_inqint
                 mi2_icv_inqlong
                 mi2_icv_inqstr
                 mi2_icv_ndattach
                 mi2_icv_detach
                 mi2_icv_get
                 mi2_icv_put
              semiprivate :
                 MI2_icv_chkid
              private :
                 MI2_icv_get_type
                 MI2_icv_get_vrange
                 MI2_get_default_range
                 MI2_icv_get_norm
                 MI2_icv_access
                 MI2_icv_zero_buffer
                 MI2_icv_coords_tovar
                 MI2_icv_calc_scale
@CREATED    : July 27, 1992. (Peter Neelin, Montreal Neurological Institute)
@MODIFIED   : 
 * $Log: image_conversion.c,v $
 * Revision 6.17  2010-03-02 12:23:14  rotor
 *  * ported HDF calls to 1.8.x
 *  * Makefile.am: updated for minccmp
 *
 * Revision 6.16  2008/01/17 02:33:02  rotor
 *  * removed all rcsids
 *  * removed a bunch of ^L's that somehow crept in
 *  * removed old (and outdated) BUGS file
 *
 * Revision 6.15  2008/01/12 19:08:14  stever
 * Add __attribute__ ((unused)) to all rcsid variables.
 *
 * Revision 6.14  2007/12/12 20:55:26  rotor
 *  * added a bunch of bug fixes from Claude.
 *
 * Revision 6.13  2004/12/14 23:53:46  bert
 * Get rid of compilation warnings
 *
 * Revision 6.12  2004/10/15 13:45:28  bert
 * Minor changes for Windows compatibility
 *
 * Revision 6.11  2004/04/27 15:40:22  bert
 * Revised logging/error handling
 *
 * Revision 6.10  2003/09/18 16:17:00  bert
 * Correctly cast double to nc_type
 *
 * Revision 6.9  2001/11/28 15:38:07  neelin
 * Removed limit on number of icvs that can exist at one time.
 *
 * Revision 6.8  2001/11/13 21:00:24  neelin
 * Modified icv scaling calculations for no normalization. When the icv
 * type is double, normalization is always done, regardless of the
 * normalization setting. When the external type is floating point,
 * normalization to the slice real range is done (essentially a valid
 * range scaling, but where the valid range for a float is the slice real
 * range).
 *
 * Revision 6.7  2001/11/13 14:15:17  neelin
 * Added functions miget_image_range and mivar_exists
 *
 * Revision 6.6  2001/08/20 13:16:53  neelin
 * Removed extraneous variables from MI2_icv_get_vrange.
 *
 * Revision 6.5  2001/08/16 19:24:11  neelin
 * Fixes to the code handling valid_range values.
 *
 * Revision 6.4  2001/08/16 16:41:31  neelin
 * Added library functions to handle reading of datatype, sign and valid range,
 * plus writing of valid range and setting of default ranges. These functions
 * properly handle differences between valid_range type and image type. Such
 * difference can cause valid data to appear as invalid when double to float
 * conversion causes rounding in the wrong direction (out of range).
 * Modified voxel_loop, volume_io and programs to use these functions.
 *
 * Revision 6.3  2001/08/16 13:32:18  neelin
 * Partial fix for valid_range of different type from image (problems
 * arising from double to float conversion/rounding). NOT COMPLETE.
 *
 * Revision 6.2  2001/04/17 18:40:12  neelin
 * Modifications to work with NetCDF 3.x
 * In particular, changed NC_LONG to NC_INT (and corresponding longs to ints).
 * Changed NC_UNSPECIFIED to NC_NAT.
 * A few fixes to the configure script.
 *
 * Revision 6.1  1999/10/19 14:45:07  neelin
 * Fixed Log subsitutions for CVS
 *
 * Revision 6.0  1997/09/12 13:24:54  neelin
 * Release of minc version 0.6
 *
 * Revision 5.0  1997/08/21  13:25:53  neelin
 * Release of minc version 0.5
 *
 * Revision 4.0  1997/05/07  20:07:52  neelin
 * Release of minc version 0.4
 *
 * Revision 3.3  1997/04/21  17:32:04  neelin
 * Fixed calculation of scale for icv so that values are not re-scaled
 * from real values to file floating-point values.
 *
 * Revision 3.2  1997/04/10  19:22:18  neelin
 * Removed redefinition of NULL and added pointer casts in appropriate places.
 *
 * Revision 3.1  1997/04/10  18:14:50  neelin
 * Fixed handling of invalid data when icv scale is zero.
 *
 * Revision 3.0  1995/05/15  19:33:12  neelin
 * Release of minc version 0.3
 *
 * Revision 2.3  1995/02/08  19:14:44  neelin
 * More changes for irix 5 lint.
 *
 * Revision 2.2  1995/02/08  19:01:06  neelin
 * Moved private function declarations from minc_routines.h to appropriate file.
 *
 * Revision 2.1  1994/12/09  09:12:30  neelin
 * Added test in miicv_detach to make sure that icv is attached before
 * detaching it.
 *
 * Revision 2.0  94/09/28  10:37:55  neelin
 * Release of minc version 0.2
 * 
 * Revision 1.18  94/09/28  10:37:06  neelin
 * Pre-release
 * 
 * Revision 1.17  93/08/11  12:59:31  neelin
 * We need only increment the chunk pointer (see previous fix) if we are
 * not doing dimension conversion (dimension conversion handles the 
 * offsets itself).
 * 
 * Revision 1.16  93/08/11  11:49:36  neelin
 * Added RCS logging in source.
 * Fixed bug in MI2_icv_access so that pointer to values buffer is incremented
 * as we loop through the chunks. This affected calls to miicv_get/put that
 * had MIimagemax/min varying over the values read in one call (ie. reading
 * or writing a volume with MIimagemax/min varying over slices will give
 * incorrect results if the volume is read with one call).
 * 
              January 22, 1993 (P.N.)
                 - Modified handling of icv properties with miicv_set<type>.
                   Removed routine miicv_set. Use routines miicv_setdbl,
                   miicv_setint, miicv_setlong, miicv_setstr instead (this
                   gives type checking at compile time).
@COPYRIGHT  :
              Copyright 1993 Peter Neelin, McConnell Brain Imaging Centre, 
              Montreal Neurological Institute, McGill University.
              Permission to use, copy, modify, and distribute this
              software and its documentation for any purpose and without
              fee is hereby granted, provided that the above copyright
              notice appear in all copies.  The author and McGill University
              make no representations about the suitability of this
              software for any purpose.  It is provided "as is" without
              express or implied warranty.
---------------------------------------------------------------------------- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H

#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include "minc2.h"

/* --------- memory allocation macros -------------------------- */

#define  MALLOC( n_items, type ) \
         ( (type *) malloc( (size_t) (n_items) * sizeof(type) ) )

#define  CALLOC( n_items, type ) \
         ( (type *) calloc( (size_t) (n_items), sizeof(type) ) )

#define  REALLOC( ptr, n_items, type ) \
         ( (type *) realloc( (void *) ptr, (size_t) (n_items) * sizeof(type) ) )

#define  FREE( ptr ) \
         free( (void *) ptr )


#define MI2_FROM_DOUBLE(dvalue, type, sign, ptr) \
   switch (type) { \
   case MI_TYPE_BYTE : \
   case MI_TYPE_UBYTE : \
      switch (sign) { \
      case MI2_PRIV_UNSIGNED : \
         dvalue = MAX(0, dvalue); \
         dvalue = MIN(UCHAR_MAX, dvalue); \
         *((unsigned char *) ptr) = ROUND(dvalue); \
         break; \
      case MI2_PRIV_SIGNED : \
         dvalue = MAX(SCHAR_MIN, dvalue); \
         dvalue = MIN(SCHAR_MAX, dvalue); \
         *((signed char *) ptr) = ROUND(dvalue); \
         break; \
      } \
      break; \
   case MI_TYPE_SHORT : \
      switch (sign) { \
      case MI2_PRIV_UNSIGNED : \
         dvalue = MAX(0, dvalue); \
         dvalue = MIN(USHRT_MAX, dvalue); \
         *((unsigned short *) ptr) = ROUND(dvalue); \
         break; \
      case MI2_PRIV_SIGNED : \
         dvalue = MAX(SHRT_MIN, dvalue); \
         dvalue = MIN(SHRT_MAX, dvalue); \
         *((signed short *) ptr) = ROUND(dvalue); \
         break; \
      } \
      break; \
   case MI_TYPE_INT : \
      switch (sign) { \
      case MI2_PRIV_UNSIGNED : \
         dvalue = MAX(0, dvalue); \
         dvalue = MIN(UINT_MAX, dvalue); \
         *((unsigned int *) ptr) = ROUND(dvalue); \
         break; \
      case MI2_PRIV_SIGNED : \
         dvalue = MAX(INT_MIN, dvalue); \
         dvalue = MIN(INT_MAX, dvalue); \
         *((signed int *) ptr) = ROUND(dvalue); \
         break; \
      } \
      break; \
   case MI_TYPE_FLOAT : \
      dvalue = MAX(-FLT_MAX,dvalue); \
      *((float *) ptr) = MIN(FLT_MAX,dvalue); \
      break; \
   case MI_TYPE_DOUBLE : \
      *((double *) ptr) = dvalue; \
      break; \
   case MI_TYPE_UNKNOWN : \
      MI_LOG_PKG_ERROR2(MI_ERR_NONNUMERIC, \
         "Attempt to convert to MI_TYPE_UNKNOWN from double"); \
      dvalue = 0; \
      break; \
   }

/**/
#define _(x) x      /* For future gettext */
         
         

/* Private functions */
static int MI2_icv_get_type(mi2_icv_type *icvp, int cdfid, int varid);
static int MI2_icv_get_vrange(mi2_icv_type *icvp, int cdfid, int varid);
static double MI2_get_default_range(char *what, mitype_t datatype, int sign);
static int MI2_icv_get_norm(mi2_icv_type *icvp, int cdfid, int varid);
static int MI2_icv_access(int operation, mi2_icv_type *icvp, long start[], 
                          long count[], void *values);
static int MI2_icv_zero_buffer(mi2_icv_type *icvp, long count[], void *values);
static int MI2_icv_coords_tovar(mi2_icv_type *icvp, 
                                long icv_start[], long icv_count[],
                                long var_start[], long var_count[]);
static int MI2_icv_calc_scale(int operation, mi2_icv_type *icvp, long coords[]);

static mi2_icv_type *MI2_icv_chkid(int icvid);

/* Array of pointers to image conversion structures */
static int minc_icv_list_nalloc = 0;
static mi2_icv_type **minc_icv_list = NULL;

/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_create
@INPUT      : (none)
@OUTPUT     : (none)
@RETURNS    : icv id or MI_ERROR when an error occurs
@DESCRIPTION: Creates an image conversion variable (icv) and returns
              a handle to it.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : August 7, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_create()
{
   int new_icv;       /* Id of newly created icv */
   mi2_icv_type *icvp;  /* Pointer to new icv structure */
   int idim;
   int new_nalloc;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_create");

   /* Look for free slot */
   for (new_icv=0; new_icv<minc_icv_list_nalloc; new_icv++)
      if (minc_icv_list[new_icv]==NULL) break;

   /* If none, then extend the list */
   if (new_icv>=minc_icv_list_nalloc) {

      /* How much space will be needed? */
      new_nalloc = minc_icv_list_nalloc + MI2_MAX_NUM_ICV;

      /* Check for first allocation */
      if (minc_icv_list_nalloc == 0) {
         minc_icv_list = MALLOC(new_nalloc, mi2_icv_type *);
      }
      else {
         minc_icv_list = REALLOC(minc_icv_list, new_nalloc, mi2_icv_type *);
      }

      /* Check that the allocation was successful */
      if (minc_icv_list == NULL) {
         MI2_LOG_SYS_ERROR1("mi2_icv_create");
         MI2_RETURN(MI_ERROR);
      }
      /* Put in NULL pointers */
      for (new_icv=minc_icv_list_nalloc; new_icv<new_nalloc; new_icv++)
         minc_icv_list[new_icv] = NULL;

      /* Use the first free slot and update the list length */
      new_icv = minc_icv_list_nalloc;
      minc_icv_list_nalloc = new_nalloc;

   }

   /* Allocate a new structure */
   if ((minc_icv_list[new_icv]=MALLOC(1, mi2_icv_type))==NULL) {
      MI2_LOG_SYS_ERROR1("mi2_icv_create");
      MI2_RETURN(MI_ERROR);
   }
   icvp=minc_icv_list[new_icv];

   /* Fill in defaults */

   /* Stuff for calling MI2_varaccess */
   icvp->do_scale = FALSE;
   icvp->do_dimconvert = FALSE;
   icvp->do_fillvalue = FALSE;
   icvp->fill_valid_min = -DBL_MAX;
   icvp->fill_valid_max = DBL_MAX;

   /* User defaults */
   icvp->user_type = MI_TYPE_SHORT;
   icvp->user_typelen = nctypelen(icvp->user_type);
   icvp->user_sign = MI2_PRIV_SIGNED;
   icvp->user_do_range = TRUE;
   icvp->user_vmax = MI2_get_default_range(MIvalid_max, icvp->user_type,
                                            icvp->user_sign);
   icvp->user_vmin = MI2_get_default_range(MIvalid_min, icvp->user_type,
                                            icvp->user_sign);
   icvp->user_do_norm = FALSE;
   icvp->user_user_norm = FALSE;
   icvp->user_maxvar = strdup(MIimagemax);
   icvp->user_minvar = strdup(MIimagemin);
   icvp->user_imgmax = MI2_DEFAULT_MAX;
   icvp->user_imgmin = MI2_DEFAULT_MIN;
   icvp->user_do_dimconv = FALSE;
   icvp->user_do_scalar = TRUE;
   icvp->user_xdim_dir = MI2_ICV_POSITIVE;
   icvp->user_ydim_dir = MI2_ICV_POSITIVE;
   icvp->user_zdim_dir = MI2_ICV_POSITIVE;
   icvp->user_num_imgdims = 2;
   icvp->user_keep_aspect = TRUE;
   icvp->user_do_fillvalue = FALSE;
   icvp->user_fillvalue = -DBL_MAX;
   for (idim=0; idim<MI2_MAX_IMGDIMS; idim++) {
      icvp->user_dim_size[idim]=MI2_ICV_ANYSIZE;
   }

   /* Variable values */
   icvp->cdfid = MI_ERROR;            /* Set so that we can recognise an */
   icvp->varid = MI_ERROR;            /* unattached icv */

   /* Values that can be read by user */
   icvp->derv_imgmax = MI2_DEFAULT_MAX;
   icvp->derv_imgmin = MI2_DEFAULT_MIN;
   for (idim=0; idim<MI2_MAX_IMGDIMS; idim++) {
      icvp->derv_dim_step[idim] = 0.0;
      icvp->derv_dim_start[idim] = 0.0;
   }

   MI2_RETURN(new_icv);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_free
@INPUT      : icvid
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Frees the image conversion variable (icv)
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : August 7, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_free(int icvid)
{
   mi2_icv_type *icvp;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_free");

   /* Check icv id */
   if ((icvp=MI2_icv_chkid(icvid)) == NULL) MI2_RETURN(MI_ERROR);

   /* Detach the icv if it is attached */
   if (icvp->cdfid != MI_ERROR) {
       if (mi2_icv_detach(icvid) < 0) {
           MI2_RETURN(MI_ERROR);
       }
   }

   /* Free anything allocated at creation time */
   FREE(icvp->user_maxvar);
   FREE(icvp->user_minvar);

   /* Free the structure */
   FREE(icvp);
   minc_icv_list[icvid]=NULL;

   /* Delete entire structure if no longer in use. */
   int new_icv;
   for (new_icv=0; new_icv<minc_icv_list_nalloc; new_icv++)
      if (minc_icv_list[new_icv]!=NULL) break;

   if (new_icv>=minc_icv_list_nalloc) {
      FREE(minc_icv_list);
      minc_icv_list_nalloc=0;
   }

   MI2_RETURN(MI_NOERROR);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_setdbl
@INPUT      : icvid        - icv id
              icv_property - property of icv to set
              value        - value to set it to
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Sets a property of an icv to a given double value
              Properties cannot be modified while the icv is attached to a 
              cdf file and variable (see mi2_icv_attach and mi2_icv_detach).
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : August 7, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_setdbl(int icvid, int icv_property, double value)
{
   int ival, idim;
   mi2_icv_type *icvp;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_setdbl");

   /* Check icv id */
   if ((icvp=MI2_icv_chkid(icvid)) == NULL) MI2_RETURN(MI_ERROR);

   /* Check that the icv is not attached to a file */
   if (icvp->cdfid != MI_ERROR) {
       milog_message(MI2_MSG_ICVATTACHED);
       MI2_RETURN(MI_ERROR);
   }

   /* Set the property */
   switch (icv_property) {
   case MI2_ICV_TYPE:
      icvp->user_type   = (mitype_t) value;
      icvp->user_typelen= nctypelen(icvp->user_type);
      icvp->user_vmax   = MI2_get_default_range(MIvalid_max, icvp->user_type,
                                               icvp->user_sign);
      icvp->user_vmin   = MI2_get_default_range(MIvalid_min, icvp->user_type,
                                               icvp->user_sign);
      break;
   case MI2_ICV_DO_RANGE:
      icvp->user_do_range = value; break;
   case MI2_ICV_VALID_MAX:
      icvp->user_vmax   = value; break;
   case MI2_ICV_VALID_MIN:
      icvp->user_vmin   = value; break;
   case MI2_ICV_DO_NORM:
      icvp->user_do_norm = value; break;
   case MI2_ICV_USER_NORM:
      icvp->user_user_norm = value; break;
   case MI2_ICV_IMAGE_MAX:
      icvp->user_imgmax = value; break;
   case MI2_ICV_IMAGE_MIN:
      icvp->user_imgmin = value; break;
   case MI2_ICV_DO_FILLVALUE:
      icvp->user_do_fillvalue = value; break;
   case MI2_ICV_FILLVALUE:
      icvp->user_fillvalue = value; break;
   case MI2_ICV_DO_DIM_CONV:
      icvp->user_do_dimconv = value; break;
   case MI2_ICV_DO_SCALAR:
      icvp->user_do_scalar = value; break;
   case MI2_ICV_XDIM_DIR: 
      ival = value;
      icvp->user_xdim_dir = ((ival==MI2_ICV_POSITIVE) || 
                             (ival==MI2_ICV_NEGATIVE)) ? ival : MI2_ICV_ANYDIR;
      break;
   case MI2_ICV_YDIM_DIR:
      ival = value;
      icvp->user_ydim_dir = ((ival==MI2_ICV_POSITIVE) || 
                             (ival==MI2_ICV_NEGATIVE)) ? ival : MI2_ICV_ANYDIR;
      break;
   case MI2_ICV_ZDIM_DIR:
      ival = value;
      icvp->user_zdim_dir = ((ival==MI2_ICV_POSITIVE) || 
                             (ival==MI2_ICV_NEGATIVE)) ? ival : MI2_ICV_ANYDIR;
      break;
   case MI2_ICV_NUM_IMGDIMS:
      ival = value;
      if ((ival<0) || (ival>MI2_MAX_IMGDIMS)) {
          milog_message(MI2_MSG_BADPROP, _("MI2_ICV_NUM_IMGDIMS out of range"));
         MI2_RETURN(MI_ERROR);
      }
      icvp->user_num_imgdims = ival;
      break;
   case MI2_ICV_ADIM_SIZE:
      icvp->user_dim_size[0] = value; break;
   case MI2_ICV_BDIM_SIZE:
      icvp->user_dim_size[1] = value; break;
   case MI2_ICV_KEEP_ASPECT:
      icvp->user_keep_aspect = value; break;
   case MI2_ICV_SIGN:
   case MI2_ICV_MAXVAR:
   case MI2_ICV_MINVAR:
       milog_message(MI2_MSG_BADPROP, 
                     _("Can't store a number in a string value"));
       MI2_RETURN(MI_ERROR);
       break;
   default:
      /* Check for image dimension properties */
      if ((icv_property>=MI2_ICV_DIM_SIZE) && 
          (icv_property<MI2_ICV_DIM_SIZE+MI2_MAX_IMGDIMS)) {
         idim = icv_property - MI2_ICV_DIM_SIZE;
         icvp->user_dim_size[idim] = value;
      }
      else {
          milog_message(MI2_MSG_BADPROP, "Unknown code");
          MI2_RETURN(MI_ERROR);
      }
      break;
   }

   MI2_RETURN(MI_NOERROR);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_setint
@INPUT      : icvid        - icv id
              icv_property - property of icv to set
              value        - value to set it to
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Sets a property of an icv to a given integer value.
              Properties cannot be modified while the icv is attached to a 
              cdf file and variable (see mi2_icv_attach and mi2_icv_detach).
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : August 7, 1992 (Peter Neelin)
@MODIFIED   : January 22, 1993 (P.N.)
                 - modified handling of icv properties
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_setint(int icvid, int icv_property, int value)
{

   MI2_SAVE_ROUTINE_NAME("mi2_icv_setint");

   if (mi2_icv_setdbl(icvid, icv_property, (double) value) < 0) {
       MI2_RETURN(MI_ERROR);
   }

   MI2_RETURN(MI_NOERROR);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_setlong
@INPUT      : icvid        - icv id
              icv_property - property of icv to set
              value        - value to set it to
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Sets a property of an icv to a given long integer value.
              Properties cannot be modified while the icv is attached to a 
              cdf file and variable (see mi2_icv_attach and mi2_icv_detach).
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : January 22, 1993 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_setlong(int icvid, int icv_property, long value)
{

   MI2_SAVE_ROUTINE_NAME("mi2_icv_setlong");

   if (mi2_icv_setdbl(icvid, icv_property, (double) value) < 0) {
       MI2_RETURN(MI_ERROR);
   }

   MI2_RETURN(MI_NOERROR);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_setstr
@INPUT      : icvid        - icv id
              icv_property - property of icv to set
              value        - value to set it to
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Sets a property of an icv to a given string value. 
              Properties cannot be modified while the icv is attached to a 
              cdf file and variable (see mi2_icv_attach and mi2_icv_detach).
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : January 22, 1993 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_setstr(int icvid, int icv_property, char *value)
{
   mi2_icv_type *icvp;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_setstr");

   /* Check icv id */
   if ((icvp=MI2_icv_chkid(icvid)) == NULL) MI2_RETURN(MI_ERROR);

   /* Check that the icv is not attached to a file */
   if (icvp->cdfid != MI_ERROR) {
       milog_message(MI2_MSG_ICVATTACHED);
       MI2_RETURN(MI_ERROR);
   }

   /* Set the property */
   switch (icv_property) {
   case MI2_ICV_SIGN:
      icvp->user_sign   = MI2_get_sign_from_string(icvp->user_type, value);
      icvp->user_vmax   = MI2_get_default_range(MIvalid_max, icvp->user_type,
                                               icvp->user_sign);
      icvp->user_vmin   = MI2_get_default_range(MIvalid_min, icvp->user_type,
                                               icvp->user_sign);
      break;
   case MI2_ICV_MAXVAR:
      if (value!=NULL) {
         FREE(icvp->user_maxvar);
         icvp->user_maxvar = strdup(value);
      }
      break;
   case MI2_ICV_MINVAR:
      if (value!=NULL) {
         FREE(icvp->user_minvar);
         icvp->user_minvar = strdup(value);
      }
      break;
   case MI2_ICV_TYPE:
   case MI2_ICV_DO_RANGE:
   case MI2_ICV_VALID_MAX:
   case MI2_ICV_VALID_MIN:
   case MI2_ICV_DO_NORM:
   case MI2_ICV_USER_NORM:
   case MI2_ICV_IMAGE_MAX:
   case MI2_ICV_IMAGE_MIN:
   case MI2_ICV_DO_DIM_CONV:
   case MI2_ICV_DO_SCALAR:
   case MI2_ICV_XDIM_DIR: 
   case MI2_ICV_YDIM_DIR:
   case MI2_ICV_ZDIM_DIR:
   case MI2_ICV_NUM_IMGDIMS:
   case MI2_ICV_ADIM_SIZE:
   case MI2_ICV_BDIM_SIZE:
   case MI2_ICV_KEEP_ASPECT:
       milog_message(MI2_MSG_BADPROP, "Can't store a string in a numeric property");
       MI2_RETURN(MI_ERROR);
      break;
   default:
      /* Check for image dimension properties */
      if ((icv_property>=MI2_ICV_DIM_SIZE) && 
          (icv_property<MI2_ICV_DIM_SIZE+MI2_MAX_IMGDIMS)) {
          milog_message(MI2_MSG_BADPROP, "Can't store a string in a numeric property");
          MI2_RETURN(MI_ERROR);
      }
      else {
          milog_message(MI2_MSG_BADPROP, "Unknown code");
          MI2_RETURN(MI_ERROR);
      }
      break;
   }

   MI2_RETURN(MI_NOERROR);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_inqdbl
@INPUT      : icvid        - icv id
              icv_property - icv property to get
@OUTPUT     : value        - value returned
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets the value of an icv property
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : January 22, 1993 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_inqdbl(int icvid, int icv_property, double *value)
{
   int idim;
   mi2_icv_type *icvp;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_inqdbl");

   /* Check icv id */
   if ((icvp=MI2_icv_chkid(icvid)) == NULL) MI2_RETURN(MI_ERROR);

   /* Set the property */
   switch (icv_property) {
   case MI2_ICV_TYPE:
      *value = icvp->user_type; break;
   case MI2_ICV_DO_RANGE:
      *value = icvp->user_do_range; break;
   case MI2_ICV_VALID_MAX:
      *value = icvp->user_vmax; break;
   case MI2_ICV_VALID_MIN:
      *value = icvp->user_vmin; break;
   case MI2_ICV_DO_NORM:
      *value = icvp->user_do_norm; break;
   case MI2_ICV_USER_NORM:
      *value = icvp->user_user_norm; break;
   case MI2_ICV_IMAGE_MAX:
      *value = icvp->user_imgmax; break;
   case MI2_ICV_IMAGE_MIN:
      *value = icvp->user_imgmin; break;
   case MI2_ICV_NORM_MAX:
      *value = icvp->derv_imgmax; break;
   case MI2_ICV_NORM_MIN:
      *value = icvp->derv_imgmin; break;
   case MI2_ICV_DO_FILLVALUE:
      *value = icvp->user_do_fillvalue; break;
   case MI2_ICV_FILLVALUE:
      *value = icvp->user_fillvalue; break;
   case MI2_ICV_DO_DIM_CONV:
      *value = icvp->user_do_dimconv; break;
   case MI2_ICV_DO_SCALAR:
      *value = icvp->user_do_scalar; break;
   case MI2_ICV_XDIM_DIR: 
      *value = icvp->user_xdim_dir; break;
   case MI2_ICV_YDIM_DIR:
      *value = icvp->user_ydim_dir; break;
   case MI2_ICV_ZDIM_DIR:
      *value = icvp->user_zdim_dir; break;
   case MI2_ICV_NUM_IMGDIMS:
      *value = icvp->user_num_imgdims; break;
   case MI2_ICV_NUM_DIMS:
      *value = icvp->var_ndims;
      if (icvp->var_is_vector && icvp->user_do_scalar) (*value)--;
      break;
   case MI2_ICV_CDFID:
      *value = icvp->cdfid; break;
   case MI2_ICV_VARID:
      *value = icvp->varid; break;
   case MI2_ICV_ADIM_SIZE:
      *value = icvp->user_dim_size[0]; break;
   case MI2_ICV_BDIM_SIZE:
      *value = icvp->user_dim_size[1]; break;
   case MI2_ICV_ADIM_STEP:
      *value = icvp->derv_dim_step[0]; break;
   case MI2_ICV_BDIM_STEP:
      *value = icvp->derv_dim_step[1]; break;
   case MI2_ICV_ADIM_START:
      *value = icvp->derv_dim_start[0]; break;
   case MI2_ICV_BDIM_START:
      *value = icvp->derv_dim_start[1]; break;
   case MI2_ICV_KEEP_ASPECT:
      *value = icvp->user_keep_aspect; break;
   case MI2_ICV_SIGN:
   case MI2_ICV_MAXVAR:
   case MI2_ICV_MINVAR:
       milog_message(MI2_MSG_BADPROP,
                     _("Tried to get icv string property as a number"));
      MI2_RETURN(MI_ERROR);
      break;
   default:
      /* Check for image dimension properties */
      if ((icv_property>=MI2_ICV_DIM_SIZE) && 
          (icv_property<MI2_ICV_DIM_SIZE+MI2_MAX_IMGDIMS)) {
         idim = icv_property - MI2_ICV_DIM_SIZE;
         *value = icvp->user_dim_size[idim];
      }
      else if ((icv_property>=MI2_ICV_DIM_STEP) && 
               (icv_property<MI2_ICV_DIM_STEP+MI2_MAX_IMGDIMS)) {
         idim = icv_property - MI2_ICV_DIM_STEP;
         *value = icvp->derv_dim_step[idim];
      }
      else if ((icv_property>=MI2_ICV_DIM_START) && 
               (icv_property<MI2_ICV_DIM_START+MI2_MAX_IMGDIMS)) {
         idim = icv_property - MI2_ICV_DIM_START;
         *value = icvp->derv_dim_start[idim];
      }
      else {
         milog_message(MI2_MSG_BADPROP, _("Tried to get unknown icv property"));
         MI2_RETURN(MI_ERROR);
      }
      break;
   }

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_inqint
@INPUT      : icvid        - icv id
              icv_property - icv property to get
@OUTPUT     : value        - value returned
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets the value of an icv property
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : January 22, 1993 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_inqint(int icvid, int icv_property, int *value)
{
   double dvalue;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_inqint");

   if (mi2_icv_inqdbl(icvid, icv_property, &dvalue) < 0) {
       MI2_RETURN(MI_ERROR);
   }

   *value = dvalue;

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_inqlong
@INPUT      : icvid        - icv id
              icv_property - icv property to get
@OUTPUT     : value        - value returned
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets the value of an icv property
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : January 22, 1993 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_inqlong(int icvid, int icv_property, long *value)
{
   double dvalue;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_inqlong");

   if (mi2_icv_inqdbl(icvid, icv_property, &dvalue) < 0) {
       MI2_RETURN(MI_ERROR);
   }

   *value = dvalue;

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_inqstr
@INPUT      : icvid        - icv id
              icv_property - icv property to get
@OUTPUT     : value        - value returned. Caller must allocate enough 
                 space for return string.
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets the value of an icv property
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_inqstr(int icvid, int icv_property, char *value)
{
   mi2_icv_type *icvp;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_inqstr");

   /* Check icv id */
   if ((icvp=MI2_icv_chkid(icvid)) == NULL) MI2_RETURN(MI_ERROR);

   /* Set the property */
   switch (icv_property) {
   case MI2_ICV_SIGN:
      if (icvp->user_sign==MI2_PRIV_SIGNED)
         (void) strcpy(value, MI_SIGNED);
      else if (icvp->user_sign==MI2_PRIV_UNSIGNED)
         (void) strcpy(value, MI_UNSIGNED);
      else
         (void) strcpy(value, MI_EMPTY_STRING);
      break;
   case MI2_ICV_MAXVAR:
      (void) strcpy(value, icvp->user_maxvar);
      break;
   case MI2_ICV_MINVAR:
      (void) strcpy(value, icvp->user_minvar);
      break;
   case MI2_ICV_TYPE:
   case MI2_ICV_DO_RANGE:
   case MI2_ICV_VALID_MAX:
   case MI2_ICV_VALID_MIN:
   case MI2_ICV_DO_NORM:
   case MI2_ICV_USER_NORM:
   case MI2_ICV_IMAGE_MAX:
   case MI2_ICV_IMAGE_MIN:
   case MI2_ICV_NORM_MAX:
   case MI2_ICV_NORM_MIN:
   case MI2_ICV_DO_DIM_CONV:
   case MI2_ICV_DO_SCALAR:
   case MI2_ICV_XDIM_DIR: 
   case MI2_ICV_YDIM_DIR:
   case MI2_ICV_ZDIM_DIR:
   case MI2_ICV_NUM_IMGDIMS:
   case MI2_ICV_ADIM_SIZE:
   case MI2_ICV_BDIM_SIZE:
   case MI2_ICV_ADIM_STEP:
   case MI2_ICV_BDIM_STEP:
   case MI2_ICV_ADIM_START:
   case MI2_ICV_BDIM_START:
   case MI2_ICV_KEEP_ASPECT:
   case MI2_ICV_NUM_DIMS:
   case MI2_ICV_CDFID:
   case MI2_ICV_VARID:
       milog_message(MI2_MSG_BADPROP, 
                     _("Tried to get icv numeric property as a string"));
      MI2_RETURN(MI_ERROR);
      break;
   default:
      /* Check for image dimension properties */
      if (((icv_property>=MI2_ICV_DIM_SIZE) && 
           (icv_property<MI2_ICV_DIM_SIZE+MI2_MAX_IMGDIMS)) ||
          ((icv_property>=MI2_ICV_DIM_STEP) && 
           (icv_property<MI2_ICV_DIM_STEP+MI2_MAX_IMGDIMS)) ||
          ((icv_property>=MI2_ICV_DIM_START) && 
           (icv_property<MI2_ICV_DIM_START+MI2_MAX_IMGDIMS))) {
         milog_message(MI2_MSG_BADPROP,
                       _("Tried to get icv numeric property as a string"));
         MI2_RETURN(MI_ERROR);
      }
      else {
         milog_message(MI2_MSG_BADPROP,
                       _("Tried to get unknown icv property"));
         MI2_RETURN(MI_ERROR);
      }
      break;
   }

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_ndattach
@INPUT      : icvid - icv id
              cdfid - cdf file id
              varid - cdf variable id
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Attaches an open cdf file and variable to an image conversion
              variable for subsequent access through miicvget and miicvput.
              File must be in data mode. This routine differs from 
              mi2_icv_attach in that no dimension conversions will be made
              on the variable (avoids linking in a significant amount
              of code).
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : September 9, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_ndattach(int icvid, int cdfid, int varid)
{
   mi2_icv_type *icvp;         /* Pointer to icv structure */
   int idim;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_ndattach");

   /* Check icv id */
   if ((icvp=MI2_icv_chkid(icvid)) == NULL) MI2_RETURN(MI_ERROR);

   /* If the icv is attached, then detach it */
   if (icvp->cdfid != MI_ERROR) {
       if (mi2_icv_detach(icvid) < 0) {
           MI2_RETURN(MI_ERROR);
       }
   }

   /* Inquire about the variable's type, sign and number of dimensions */
   if (MI2_icv_get_type(icvp, cdfid, varid) < 0) {
       MI2_RETURN(MI_ERROR);
   }

   /* If not doing range calculations, just set derv_firstdim for
      MI2_icv_access, otherwise, call routines to calculate range and 
      normalization */
   if (!icvp->user_do_range) {
      icvp->derv_firstdim = -1;
   }
   else {
      /* Get valid range */
       if (MI2_icv_get_vrange(icvp, cdfid, varid) < 0) {
           MI2_RETURN(MI_ERROR);
       }
          
      /* Get normalization info */
       if (MI2_icv_get_norm(icvp, cdfid, varid) < 0) {
           MI2_RETURN(MI_ERROR);
       }
   }

   /* Set other fields to defaults */
   icvp->var_is_vector = FALSE;
   icvp->var_vector_size = 1;
   icvp->derv_do_zero = FALSE;
   icvp->derv_do_bufsize_step = FALSE;
   icvp->derv_var_pix_off = NULL;
   icvp->derv_usr_pix_off = NULL;
   for (idim=0; idim<icvp->user_num_imgdims; idim++) {
      icvp->derv_dim_flip[idim] = FALSE;
      icvp->derv_dim_grow[idim] = TRUE;
      icvp->derv_dim_scale[idim] = 1;
      icvp->derv_dim_off[idim] = 0;
      icvp->derv_dim_step[idim] = 0.0;
      icvp->derv_dim_start[idim] = 0.0;
   }

   /* Set the do_scale and do_dimconvert fields of icv structure
      We have to scale only if do_range is TRUE. If ranges don't
      match, or we have to do user normalization, or if we are normalizing
      and MIimagemax or MIimagemin vary over the variable. We don't have
      to scale if input and output are both floating point. */

   icvp->do_scale = 
      (icvp->user_do_range && 
       ((icvp->user_vmax!=icvp->var_vmax) ||
        (icvp->user_vmin!=icvp->var_vmin) ||
        (icvp->user_do_norm && icvp->user_user_norm) ||
        (icvp->user_do_norm && (icvp->derv_firstdim>=0))) );

   if ((icvp->derv_usr_float && icvp->derv_var_float))
      icvp->do_scale = FALSE;

   icvp->do_dimconvert = FALSE;

   /* Set the cdfid and varid fields */
   icvp->cdfid = cdfid;
   icvp->varid = varid;

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_get_type
@INPUT      : icvp  - pointer to icv structure
              cdfid - cdf file id
              varid - variable id
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets the type and sign of a variable for mi2_icv_attach.
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : August 10, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
static int MI2_icv_get_type(mi2_icv_type *icvp, int cdfid, int varid)
{
   int oldncopts;            /* For saving value of ncopts */
   char stringa[MI_MAX_ATTSTR_LEN];
   char *string=stringa;     /* String for sign info */

   MI2_SAVE_ROUTINE_NAME("MI2_icv_get_type");

   /* Inquire about the variable */
   if (ncvarinq(cdfid, varid, NULL, &(icvp->var_type), 
                &(icvp->var_ndims), icvp->var_dim, NULL) < 0) {
       MI2_RETURN(MI_ERROR);
   }

   /* Check that the variable type is numeric */
   if (icvp->var_type==MI_TYPE_BYTE) {
       milog_message(MI2_MSG_VARNOTNUM);
       MI2_RETURN(MI_ERROR);
   }

   /* Try to find out the sign of the variable using MIsigntype. */
//TODO:CNV   oldncopts = ncopts; ncopts = 0;
   string=miattgetstr(cdfid, varid, MIsigntype, MI_MAX_ATTSTR_LEN, string);
//TODO:CNV   ncopts = oldncopts;
   icvp->var_sign  = MI2_get_sign_from_string(icvp->var_type, string);

   /* Get type lengths */
   icvp->var_typelen = nctypelen(icvp->var_type);
   icvp->user_typelen = nctypelen(icvp->user_type);

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_get_vrange
@INPUT      : icvp  - pointer to icv structure
              cdfid - cdf file id
              varid - variable id
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets the valid range of a variable
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : August 10, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
static int MI2_icv_get_vrange(mi2_icv_type *icvp, int cdfid, int varid)
{
   double vrange[2];         /* Valid range buffer */

   MI2_SAVE_ROUTINE_NAME("MI2_icv_get_vrange");

   if (miget_valid_range(cdfid, varid, vrange) == MI_ERROR) {
      MI2_RETURN(MI_ERROR);
   }
   icvp->var_vmin = vrange[0];
   icvp->var_vmax = vrange[1];

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_get_default_range
@INPUT      : what     - MIvalid_min means get default min, MIvalid_min means 
                 get default min
              datatype - type of variable
              sign     - sign of variable
@OUTPUT     : (none)
@RETURNS    : default maximum or minimum for the datatype
@DESCRIPTION: Return the defaults maximum or minimum for a given datatype
              and sign.
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : August 10, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
static double MI2_get_default_range(char *what, mitype_t datatype, int sign)
{
   double range[2];

   MI2_SAVE_ROUTINE_NAME("MI2_get_default_range");

   (void) miget_default_range(datatype, (sign == MI2_PRIV_SIGNED), range);

   if (STRINGS_EQUAL(what, MIvalid_max)) {
      MI2_RETURN(range[1]);
   }
   else if (STRINGS_EQUAL(what, MIvalid_min)) {
      MI2_RETURN(range[0]);
   }
   else {
//TODO:CNV      ncopts = NC_VERBOSE | NC_FATAL;
      MI2_LOG_PKG_ERROR2(-1,"MINC bug - this line should never be printed");
   }

   MI2_RETURN(MI2_DEFAULT_MIN);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_get_norm
@INPUT      : icvp  - pointer to icv structure
              cdfid - cdf file id
              varid - variable id
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets the normalization info for a variable
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : August 10, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
static int MI2_icv_get_norm(mi2_icv_type *icvp, int cdfid, int varid)
     /* ARGSUSED */
{
   int oldncopts;             /* For saving value of ncopts */
   int vid[2];                /* Variable ids for max and min */
   int ndims;                 /* Number of dimensions for image max and min */
   int dim[MI2_MAX_VAR_DIMS];     /* Dimensions */
   int imm;                   /* Counter for looping through max and min */
   double image_range[2];
   int idim, i;

   MI2_SAVE_ROUTINE_NAME("MI2_icv_get_norm");

   /* Check for floating point or double precision values for user or
      in variable - set flag to not do normalization if needed */
   icvp->derv_var_float = ((icvp->var_type == MI_TYPE_DOUBLE) ||
                           (icvp->var_type == MI_TYPE_FLOAT));
   icvp->derv_usr_float = ((icvp->user_type == MI_TYPE_DOUBLE) ||
                           (icvp->user_type == MI_TYPE_FLOAT));

   /* Initialize first dimension over which MIimagemax or MIimagemin
      vary - assume that they don't vary at all */
   icvp->derv_firstdim=(-1);

   /* Look for image max, image min variables */
//TODO:CNV   oldncopts=ncopts; ncopts=0;
   icvp->imgmaxid=ncvarid(cdfid, icvp->user_maxvar);
   icvp->imgminid=ncvarid(cdfid, icvp->user_minvar);
//TODO:CNV   ncopts = oldncopts;

   /* Check to see if normalization to variable max, min should be done */
   if (!icvp->user_do_norm) {
      icvp->derv_imgmax = MI2_DEFAULT_MAX;
      icvp->derv_imgmin = MI2_DEFAULT_MIN;
   }
   else {

      /* Get the image min and max, either from the user definition or 
         from the file. */
      if (icvp->user_user_norm) {
         icvp->derv_imgmax = icvp->user_imgmax;
         icvp->derv_imgmin = icvp->user_imgmin;
      }
      else {
          if (miget_image_range(cdfid, image_range) < 0) {
              MI2_RETURN(MI_ERROR);
          }
         icvp->derv_imgmin = image_range[0];
         icvp->derv_imgmax = image_range[1];
      }

      /* Check each of the dimensions of image-min/max variables to see
         which is the fastest varying dimension of the image variable. */
      vid[0]=icvp->imgminid;
      vid[1]=icvp->imgmaxid;
      if ((vid[0] != MI_ERROR) && (vid[1] != MI_ERROR)) {
         for (imm=0; imm < 2; imm++) {
             if (ncvarinq(cdfid, vid[imm], NULL, NULL, &ndims, dim, NULL) < 0) {
                 MI2_RETURN(MI_ERROR);
             }
            for (idim=0; idim<ndims; idim++) {
               for (i=0; i<icvp->var_ndims; i++) {
                  if (icvp->var_dim[i]==dim[idim])
                     icvp->derv_firstdim = MAX(icvp->derv_firstdim, i);
               }
            }
         }
      }

   }

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_detach
@INPUT      : icvid - icv id
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Detaches the cdf file and variable from the image conversion
              variable, allowing modifications to the icv.
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : August 10, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_detach(int icvid)
{
   mi2_icv_type *icvp;
   int idim;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_detach");

   /* Check icv id */
   if ((icvp=MI2_icv_chkid(icvid)) == NULL) MI2_RETURN(MI_ERROR);

   /* Check that the icv is in fact attached */
   if (icvp->cdfid == MI_ERROR)
      MI2_RETURN(MI_NOERROR);

   /* Free the pixel offset arrays */
   if (icvp->derv_var_pix_off != NULL) FREE(icvp->derv_var_pix_off);
   if (icvp->derv_usr_pix_off != NULL) FREE(icvp->derv_usr_pix_off);

   /* Reset values that are read-only (and set when attached) */
   icvp->derv_imgmax = MI2_DEFAULT_MAX;
   icvp->derv_imgmin = MI2_DEFAULT_MIN;
   for (idim=0; idim<MI2_MAX_IMGDIMS; idim++) {
      icvp->derv_dim_step[idim] = 0.0;
      icvp->derv_dim_start[idim] = 0.0;
   }

   /* Set cdfid field to MI_ERROR to indicate that icv is detached */
   icvp->cdfid = MI_ERROR;
   icvp->varid = MI_ERROR;

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_get
@INPUT      : icvid  - icv id
              start  - coordinates of start of hyperslab (see ncvarget)
              count  - size of hyperslab (see ncvarget)
@OUTPUT     : values - array of values returned
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets a hyperslab of values from a netcdf variable through
              the image conversion variable (icvid) 
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : August 10, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_get(int icvid, long start[], long count[], void *values)
{
   mi2_icv_type *icvp;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_get");

   /* Check icv id */
   if ((icvp=MI2_icv_chkid(icvid)) == NULL) MI2_RETURN(MI_ERROR);

   /* Get the data */
   if (MI2_icv_access(MI2_PRIV_GET, icvp, start, count, values) < 0) {
       MI2_RETURN(MI_ERROR);
   }

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_put
@INPUT      : icvid  - icv id
              start  - coordinates of start of hyperslab (see ncvarput)
              count  - size of hyperslab (see ncvarput)
              values - array of values to store
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Stores a hyperslab of values in a netcdf variable through
              the image conversion variable (icvid)
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
MNCAPI int mi2_icv_put(int icvid, long start[], long count[], void *values)
{
   mi2_icv_type *icvp;

   MI2_SAVE_ROUTINE_NAME("mi2_icv_put");

   /* Check icv id */
   if ((icvp=MI2_icv_chkid(icvid)) == NULL) MI2_RETURN(MI_ERROR);


   if (MI2_icv_access(MI2_PRIV_PUT, icvp, start, count, values) < 0) {
       MI2_RETURN(MI_ERROR);
   }

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_access
@INPUT      : operation - MI2_PRIV_GET or MI2_PRIV_PUT
              icvid     - icv id
              start     - coordinates of start of hyperslab (see ncvarput)
              count     - size of hyperslab (see ncvarput)
              values    - array of values to put
@OUTPUT     : values    - array of values to get
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Does the work of getting or putting values from an icv.
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : August 11, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
static int MI2_icv_access(int operation, mi2_icv_type *icvp, long start[], 
                          long count[], void *values)
{
   int *bufsize_step;                /* Pointer to array giving increments
                                        for allocating variable buffer
                                        (NULL if we don't care) */
   long chunk_count[MI2_MAX_VAR_DIMS];   /* Number of elements to get for chunk */
   long chunk_start[MI2_MAX_VAR_DIMS];   /* Starting index for getting a chunk */
   long chunk_size;                  /* Size of chunk in bytes */
   void *chunk_values;               /* Pointer to next chunk to get */
   long var_start[MI2_MAX_VAR_DIMS];     /* Coordinates of first var element */
   long var_count[MI2_MAX_VAR_DIMS];     /* Edge lengths in variable */
   long var_end[MI2_MAX_VAR_DIMS];       /* Coordinates of last var element */
   int firstdim;
   int idim, ndims;

   MI2_SAVE_ROUTINE_NAME("MI2_icv_access");

   /* Check that icv is attached to a variable */
   if (icvp->cdfid == MI_ERROR) {
       milog_message(MI2_MSG_ICVNOTATTACHED);

       MI2_RETURN(MI_ERROR);
   }

   /* Zero the user's buffer if needed */
   if ((operation == MI2_PRIV_GET) && (icvp->derv_do_zero))
       if (MI2_icv_zero_buffer(icvp, count, values) < 0) {
           MI2_RETURN(MI_ERROR);
       }

   /* Translate icv coordinates to variable coordinates */
   if (MI2_icv_coords_tovar(icvp, start, count, var_start, var_count) < 0) {
       MI2_RETURN(MI_ERROR);
   }

   /* Save icv coordinates for future reference (for dimension conversion
      routines) */
   ndims = icvp->var_ndims;
   if (icvp->var_is_vector && icvp->user_do_scalar)
      ndims--;
   for (idim=0; idim < ndims; idim++) {
      icvp->derv_icv_start[idim] = start[idim];
      icvp->derv_icv_count[idim] = count[idim];
   }

   /* Do we care about getting variable in convenient increments ? 
      Only if we are getting data and the icv structure wants it */
   if ((operation==MI2_PRIV_GET) && (icvp->derv_do_bufsize_step))
      bufsize_step = icvp->derv_bufsize_step;
   else
      bufsize_step = NULL;

   /* Set up variables for looping through variable. The biggest chunk that
      we can get in one call is determined by the subscripts of MIimagemax
      and MIimagemin. These must be constant over the chunk that we get if
      we are doing normalization. */
   for (idim=0; idim<icvp->var_ndims; idim++) {
      chunk_start[idim] = var_start[idim];
      var_end[idim]=var_start[idim]+var_count[idim];
   }
   (void) miset_coords(icvp->var_ndims, 1L, chunk_count);
   /* Get size of chunk in user's buffer. Dimension conversion routines
      don't need the buffer pointer incremented - they do it themselves */
   if (!icvp->do_dimconvert)
      chunk_size = nctypelen(icvp->user_type);
   else
      chunk_size = 0;
   for (idim=MAX(icvp->derv_firstdim+1,0); idim < icvp->var_ndims; idim++) {
      chunk_count[idim]=var_count[idim];
      chunk_size *= chunk_count[idim];
   }
   firstdim = MAX(icvp->derv_firstdim, 0);

   /* Loop through variable */
   chunk_values = values;
   while (chunk_start[0] < var_end[0]) {

      /* Set the do_fillvalue flag if the user wants it and we are doing
         a get. We must do it inside the loop since the scale factor 
         calculation can change it if the scale is zero. (Fillvalue checking
         is always done if the the scale is zero.) */
      icvp->do_fillvalue = 
         icvp->user_do_fillvalue && (operation == MI2_PRIV_GET);
      icvp->fill_valid_min = icvp->var_vmin;
      icvp->fill_valid_max = icvp->var_vmax;

      /* Calculate scale factor */
      if (icvp->do_scale) {
          if (MI2_icv_calc_scale(operation, icvp, chunk_start) < 0) {
              MI2_RETURN(MI_ERROR);
          }
      }

// fprintf(stderr, "Getting values at %p\n", chunk_start);

      /* Get the values */
      if (MI2_varaccess(operation, icvp->cdfid, icvp->varid,
                       chunk_start, chunk_count,
                       icvp->user_type, icvp->user_sign,
                       chunk_values, bufsize_step, icvp) < 0) {
          MI2_RETURN(MI_ERROR);
      }

      /* Increment the start counter */
      chunk_start[firstdim] += chunk_count[firstdim];
      for (idim=firstdim; 
           (idim>0) && (chunk_start[idim]>=var_end[idim]); idim--) {
         chunk_start[idim]=var_start[idim];
         chunk_start[idim-1]++;
      }

      /* Increment the pointer to values */
      chunk_values = (void *) ((char *) chunk_values + (size_t) chunk_size);

   }

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_zero_buffer
@INPUT      : icvp      - icv structure pointer
              count     - count vector
              values    - pointer to user's buffer
@OUTPUT     : 
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Zeros the user's buffer, with a size given by the vector count.
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : September 9, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
static int MI2_icv_zero_buffer(mi2_icv_type *icvp, long count[], void *values)
{
   double zeroval, zerobuf;
   void *zerostart;
   int zerolen, idim, ndims;
   char *bufptr, *bufend, *zeroptr, *zeroend;
   long buflen;

   MI2_SAVE_ROUTINE_NAME("MI2_icv_zero_buffer");

   /* Create a zero pixel and get its size */
   zerostart = (void *) (&zerobuf);
   if (icvp->do_scale)
      zeroval = icvp->offset;
   else
      zeroval = 0.0;
   {MI2_FROM_DOUBLE(zeroval, icvp->user_type, icvp->user_sign, zerostart)}
   zerolen = icvp->user_typelen;
   
   /* Get the buffer size */
   ndims = icvp->var_ndims;
   if (icvp->var_is_vector && icvp->user_do_scalar)
      ndims--;
   buflen = zerolen;
   for (idim=0; idim<ndims; idim++)
      buflen *= count[idim];

   /* Loop through the buffer, copying the zero pixel */
   bufend = (char *) values + buflen;
   zeroend = (char *) zerostart + zerolen;
   for (bufptr = (char *) values, zeroptr = (char *) zerostart;
        bufptr < bufend; bufptr++, zeroptr++) {
      if (zeroptr >= zeroend)
         zeroptr = (char *) zerostart;
      *bufptr = *zeroptr;
   }

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_coords_tovar
@INPUT      : icvp      - icv structure pointer
              icv_start - start vector for icv
              icv_count - count vector for icv
@OUTPUT     : var_start - start vector for variable
              var_count - count vector for variable
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Converts a start and count vector for referencing an icv
              to the corresponding vectors for referencing a NetCDF variable.
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : September 1, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
static int MI2_icv_coords_tovar(mi2_icv_type *icvp, 
                                long icv_start[], long icv_count[],
                                long var_start[], long var_count[])
{
   int i, j;
   int num_non_img_dims;
   long coord, last_coord, icv_dim_size;

   MI2_SAVE_ROUTINE_NAME("MI2_icv_coords_tovar");

   /* Do we have to worry about dimension conversions? If not, then
      just copy the vectors and return. */
   if (!icvp->do_dimconvert) {
      for (i=0; i < icvp->var_ndims; i++) {
         var_count[i] = icv_count[i];
         var_start[i] = icv_start[i];
      }
      MI2_RETURN(MI_NOERROR);
   }

   /* Get the number of non image dimensions */
   num_non_img_dims=icvp->var_ndims-icvp->user_num_imgdims;
   if (icvp->var_is_vector)
      num_non_img_dims--;

   /* Go through first, non-image dimensions */
   for (i=0; i < num_non_img_dims; i++) {
      var_count[i] = icv_count[i];
      var_start[i] = icv_start[i];
   }

   /* Go through image dimensions */
   for (i=num_non_img_dims, j=icvp->user_num_imgdims-1; 
        i < num_non_img_dims+icvp->user_num_imgdims; i++, j--) {
      /* Check coordinates. */
      icv_dim_size = (icvp->user_dim_size[j] > 0) ?
            icvp->user_dim_size[j] : icvp->var_dim_size[j];
      last_coord = icv_start[i] + icv_count[i] - 1;
      if ((icv_start[i]<0) || (icv_start[i]>=icv_dim_size) ||
          (last_coord<0) || (last_coord>=icv_dim_size) ||
          (icv_count[i]<0)) {
          milog_message(MI2_MSG_ICVCOORDS);
          MI2_RETURN(MI_ERROR);
      }
      /* Remove offset */
      coord = icv_start[i]-icvp->derv_dim_off[j];
      /* Check for growing or shrinking */
      if (icvp->derv_dim_grow[j]) {
         var_count[i] = (icv_count[i]+icvp->derv_dim_scale[j]-1)
            /icvp->derv_dim_scale[j];
         coord /= icvp->derv_dim_scale[j];
      }
      else {
         var_count[i] = icv_count[i]*icvp->derv_dim_scale[j];
         coord *= icvp->derv_dim_scale[j];
      }
      /* Check for flipping */
      if (icvp->derv_dim_flip[j])
         coord = icvp->var_dim_size[j] - coord -
            ((icv_count!=NULL) ? var_count[i] : 0L);
      var_start[i] = coord;
      /* Check for indices out of variable bounds (but in icv bounds) */
      last_coord = var_start[i] + var_count[i];
      if ((var_start[i]<0) || (last_coord>=icvp->var_dim_size[j])) {
         if (var_start[i]<0) var_start[i] = 0;
         if (last_coord>=icvp->var_dim_size[j]) 
            last_coord = icvp->var_dim_size[j] - 1;
         var_count[i] = last_coord - var_start[i] + 1;
      }
   }

   /* Check for vector dimension */
   if (icvp->var_is_vector) {
      if (icvp->user_do_scalar) {
         var_count[icvp->var_ndims-1] = icvp->var_vector_size;
         var_start[icvp->var_ndims-1] = 0;
      }
      else {
         var_count[icvp->var_ndims-1] = icv_count[icvp->var_ndims-1];
         var_start[icvp->var_ndims-1] = icv_start[icvp->var_ndims-1];
      }
   }

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_calc_scale
@INPUT      : operation - MI2_PRIV_GET or MI2_PRIV_PUT
              icvp      - icv structure pointer
              coords    - coordinates of first value to get or put
@OUTPUT     : icvp      - fields scale and offset set
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Calculates the scale and offset needed for getting or putting
              values, starting at index coords (assumes that scale is constant
              over that range).
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF routines
@CREATED    : August 10, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
static int MI2_icv_calc_scale(int operation, mi2_icv_type *icvp, long coords[])
{
   long mmcoords[MI2_MAX_VAR_DIMS];   /* Coordinates for max/min variable */
   double usr_imgmax, usr_imgmin;
   double var_imgmax, var_imgmin;
   double var_imgmax_true, var_imgmin_true;
   double usr_vmax, usr_vmin;
   double var_vmax, var_vmin;
   double slice_imgmax, slice_imgmin;
   double usr_scale;
   double denom;

   MI2_SAVE_ROUTINE_NAME("MI2_icv_calc_scale");

   /* Set variable valid range */
   var_vmax = icvp->var_vmax;
   var_vmin = icvp->var_vmin;

   /* Set image max/min for user and variable values depending on whether
      normalization should be done or not. Whenever floating-point values
      are involved, some type of normalization is done. When the icv type
      is floating point, normalization is always done. When the file type
      is floating point and the icv type is integer, slices are normalized
      to the real range of the slice (or chunk being read). */
   if (!icvp->derv_var_float && !icvp->derv_usr_float && !icvp->user_do_norm) {
      usr_imgmax = var_imgmax = MI2_DEFAULT_MAX;
      usr_imgmin = var_imgmin = MI2_DEFAULT_MIN;
   }
   else {

      /* Get the real range for the slice or chunk that is being examined */
      slice_imgmax = MI2_DEFAULT_MAX;
      slice_imgmin = MI2_DEFAULT_MIN;
      if ((!icvp->derv_var_float || !icvp->user_do_norm) &&
          (icvp->imgmaxid!=MI_ERROR) && (icvp->imgminid!=MI_ERROR)) {
         if (mitranslate_coords(icvp->cdfid, icvp->varid, coords, 
                                icvp->imgmaxid, mmcoords) == NULL)
            MI2_RETURN(MI_ERROR);
         if (mivarget1(icvp->cdfid, icvp->imgmaxid, mmcoords,
                       MI_TYPE_DOUBLE, NULL, &slice_imgmax) < 0) {
             MI2_RETURN(MI_ERROR);
         }
         if (mitranslate_coords(icvp->cdfid, icvp->varid, coords, 
                                icvp->imgminid, mmcoords) == NULL) {
            MI2_RETURN(MI_ERROR);
         }
         if (mivarget1(icvp->cdfid, icvp->imgminid, mmcoords,
                       MI_TYPE_DOUBLE, NULL, &slice_imgmin) < 0) {
             MI2_RETURN(MI_ERROR);
         }
      }

      /* Get the user real range */
      if (icvp->user_do_norm) {
         usr_imgmax = icvp->derv_imgmax;
         usr_imgmin = icvp->derv_imgmin;
      }
      else {
         usr_imgmax = slice_imgmax;
         usr_imgmin = slice_imgmin;
      }

      /* Get the file real range */
      if (icvp->derv_var_float) {
         var_imgmax = var_vmax;
         var_imgmin = var_vmin;
      }
      else {
         var_imgmax = slice_imgmax;
         var_imgmin = slice_imgmin;
      }
   }

   /* Prevent scaling between file floats and real value */
   if (icvp->derv_var_float) {
      var_imgmax = var_vmax;
      var_imgmin = var_vmin;
   }

   /* Get user valid range */
   if (icvp->derv_usr_float) {
      usr_vmax = usr_imgmax;
      usr_vmin = usr_imgmin;
   }
   else {
      usr_vmax = icvp->user_vmax;
      usr_vmin = icvp->user_vmin;
   }

   /* Save real var_imgmin/max for fillvalue checking later */
   var_imgmax_true = var_imgmax;
   var_imgmin_true = var_imgmin;

   /* Even though we have already carefully set the vmax/min and imgmax/min
      values to handle the floating point case, we can still have problems
      with the scale calculations (rounding errors) if full range max/min 
      are used (-FLT_MAX to FLT_MAX). To avoid this, we just force the
      values to 0 and 1 which will give the correct scale. That is why 
      we save the true values above. */

   if (icvp->derv_usr_float) {
      usr_imgmax = usr_vmax = MI2_DEFAULT_MAX;
      usr_imgmin = usr_vmin = MI2_DEFAULT_MIN;
   }
   if (icvp->derv_var_float) {
      var_imgmax = var_vmax = MI2_DEFAULT_MAX;
      var_imgmin = var_vmin = MI2_DEFAULT_MIN;
   }

   /* Calculate scale and offset for MI2_PRIV_GET */

   /* Scale */
   denom = usr_imgmax - usr_imgmin;
   if (denom!=0.0)
      usr_scale=(usr_vmax - usr_vmin) / denom;
   else
      usr_scale=0.0;
   denom = var_vmax - var_vmin;
   if (denom!=0.0)
      icvp->scale = usr_scale * (var_imgmax - var_imgmin) / denom;
   else
      icvp->scale = 0.0;

   /* Offset */
   icvp->offset = usr_vmin - icvp->scale * var_vmin
                + usr_scale * (var_imgmin - usr_imgmin);

   /* If we want a MI2_PRIV_PUT, invert */
   if (operation==MI2_PRIV_PUT) {
      if (icvp->scale!=0.0) {
         icvp->offset = (-icvp->offset) / icvp->scale;
         icvp->scale  = 1.0/icvp->scale;
      }
      else {
         icvp->offset = var_vmin;
         icvp->scale  = 0.0;
      }
   }

   /* Do fill value checking if scale is zero */
   if (icvp->scale == 0.0) {

      /* Check for floating point on both sides of conversion. We should
         not be doing scaling in this case, but we will check to be safe. */
      if (icvp->derv_var_float && icvp->derv_usr_float) {
         icvp->do_scale = FALSE;
         icvp->do_fillvalue = FALSE;
      }

      else {      /* Not pure floating point */

         icvp->do_fillvalue = TRUE;

         /* For output, set the range properly depending on whether the user
            type is floating point or not */
         if (operation == MI2_PRIV_PUT) {
            if (icvp->derv_usr_float) {
               icvp->fill_valid_min = var_imgmin_true;
               icvp->fill_valid_max = var_imgmax_true;
            }
            else if (usr_scale != 0.0) {
               icvp->fill_valid_min = 
                  usr_vmin + (var_imgmin_true - usr_imgmin) / usr_scale;
               icvp->fill_valid_max = 
                  usr_vmin + (var_imgmax_true - usr_imgmin) / usr_scale;
            }
            else {
               icvp->fill_valid_min = usr_vmin;
               icvp->fill_valid_max = usr_vmax;
            }
         }        /* If output operation */

      }        /* If not pure floating point */

   }       /* If scale == 0.0 */

   MI2_RETURN(MI_NOERROR);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_chkid
@INPUT      : icvid  - icv id
@OUTPUT     : (none)
@RETURNS    : Pointer to icv structure if it exists, otherwise NULL.
@DESCRIPTION: Checks that an icv id is valid and returns a pointer to the
              structure.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : August 7, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
static mi2_icv_type *MI2_icv_chkid(int icvid)
{
   MI2_SAVE_ROUTINE_NAME("MI2_icv_chkid");

   /* Check icv id */
   if ((icvid<0) || (icvid>=minc_icv_list_nalloc) || 
       (minc_icv_list[icvid]==NULL)) {
       milog_message(MI2_MSG_BADICV);
       MI2_RETURN((void *) NULL);
   }

   MI2_RETURN(minc_icv_list[icvid]);
}
