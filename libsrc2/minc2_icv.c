/* ----------------------------- MNI Header -----------------------------------
@NAME       : minc2_icv.c
@DESCRIPTION: File of functions to manipulate image conversion variables
              (icv). These variables allow conversion of netcdf variables
              (the MINC image variable, in particular) to a form more
              convenient for a program.
@METHOD     : Routines included in this file :
@CREATED    : July 27, 1992. (Peter Neelin, Montreal Neurological Institute)
@MODIFIED   :
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
#include "minc2_private.h"
#include <math.h>

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


#define MI2_STRINGS_EQUAL(str1,str2) (strcmp(str1,str2)==0)


/* Private functions */
static int MI2_icv_get_type ( mi2_icv_type *icvp, mihandle_t volume );
static int MI2_icv_get_vrange ( mi2_icv_type *icvp, mihandle_t volume );
static double MI2_get_default_range ( char *what, mitype_t datatype );
static int MI2_icv_get_norm ( mi2_icv_type *icvp, mihandle_t volume );
static int MI2_icv_access ( int operation, mi2_icv_type *icvp, long start[],
                            long count[], void *values );
static int MI2_icv_zero_buffer ( mi2_icv_type *icvp, long count[], void *values );
static int MI2_icv_coords_tovar ( mi2_icv_type *icvp,
                                  long icv_start[], long icv_count[],
                                  long var_start[], long var_count[] );
static int MI2_icv_calc_scale ( int operation, mi2_icv_type *icvp, long coords[] );

static mi2_icv_type *MI2_icv_chkid ( int icvid );

static int MI2_icv_get_dim ( mi2_icv_type *icvp, mihandle_t volume );
static int MI2_get_dim_flip ( mi2_icv_type *icvp, mihandle_t volume, int dimvid[],
                              int subsc[] );
static int MI2_get_dim_scale ( mi2_icv_type *icvp, mihandle_t volume, int dimvid[] );
static int MI2_get_dim_bufsize_step ( mi2_icv_type *icvp, int subsc[] );
static int MI2_icv_get_dim_conversion ( mi2_icv_type *icvp, int subsc[] );
static int MI2_icv_dimconvert ( int operation, mi2_icv_type *icvp,
                                long start[], long count[], void *values,
                                long bufstart[], long bufcount[], void *buffer );
static int MI2_icv_dimconv_init ( int operation, mi2_icv_type *icvp,
                                  mi2_icv_dimconv_type *dcp,
                                  long start[], long count[], void *values,
                                  long bufstart[], long bufcount[], void *buffer );

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
int mi2_icv_create()
{
  int new_icv;       /* Id of newly created icv */
  mi2_icv_type *icvp;  /* Pointer to new icv structure */
  int idim;
  int new_nalloc;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_create" );

  /* Look for free slot */
  for ( new_icv = 0; new_icv < minc_icv_list_nalloc; new_icv++ )
    if ( minc_icv_list[new_icv] == NULL )
      break;

  /* If none, then extend the list */
  if ( new_icv >= minc_icv_list_nalloc ) {

    /* How much space will be needed? */
    new_nalloc = minc_icv_list_nalloc + MI2_MAX_NUM_ICV;

    /* Check for first allocation */
    if ( minc_icv_list_nalloc == 0 ) {
      minc_icv_list = MALLOC ( new_nalloc, mi2_icv_type * );
    } else {
      minc_icv_list = REALLOC ( minc_icv_list, new_nalloc, mi2_icv_type * );
    }

    /* Check that the allocation was successful */
    if ( minc_icv_list == NULL ) {
      MI2_LOG_SYS_ERROR1 ( "mi2_icv_create" );
      MI2_RETURN ( MI_ERROR );
    }

    /* Put in NULL pointers */
    for ( new_icv = minc_icv_list_nalloc; new_icv < new_nalloc; new_icv++ )
      minc_icv_list[new_icv] = NULL;

    /* Use the first free slot and update the list length */
    new_icv = minc_icv_list_nalloc;
    minc_icv_list_nalloc = new_nalloc;

  }

  /* Allocate a new structure */
  if ( ( minc_icv_list[new_icv] = MALLOC ( 1, mi2_icv_type ) ) == NULL ) {
    MI2_LOG_SYS_ERROR1 ( "mi2_icv_create" );
    MI2_RETURN ( MI_ERROR );
  }

  icvp = minc_icv_list[new_icv];

  /* Fill in defaults */

  /* Stuff for calling MI2_varaccess */
  icvp->do_scale = FALSE;
  icvp->do_dimconvert = FALSE;
  icvp->do_fillvalue = FALSE;
  icvp->fill_valid_min = -DBL_MAX;
  icvp->fill_valid_max = DBL_MAX;

  /* User defaults */
  icvp->user_type = MI_TYPE_SHORT;
  icvp->user_typelen = mitype_len ( icvp->user_type );
  icvp->user_sign = MI2_PRIV_SIGNED;
  icvp->user_do_range = TRUE;
  icvp->user_vmax = MI2_get_default_range ( MIvalid_max, icvp->user_type );
  icvp->user_vmin = MI2_get_default_range ( MIvalid_min, icvp->user_type );
  icvp->user_do_norm = FALSE;
  icvp->user_user_norm = FALSE;
  icvp->user_maxvar = strdup ( MIimagemax );
  icvp->user_minvar = strdup ( MIimagemin );
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

  for ( idim = 0; idim < MI2_MAX_IMGDIMS; idim++ ) {
    icvp->user_dim_size[idim] = MI2_ICV_ANYSIZE;
  }

  /* Variable values */
  icvp->volume = NULL;            /* Set so that we can recognise an */
  icvp->imgmaxid = NULL;         /* unattached icv */
  icvp->imgminid = NULL;

  /* Values that can be read by user */
  icvp->derv_imgmax = MI2_DEFAULT_MAX;
  icvp->derv_imgmin = MI2_DEFAULT_MIN;

  for ( idim = 0; idim < MI2_MAX_IMGDIMS; idim++ ) {
    icvp->derv_dim_step[idim] = 0.0;
    icvp->derv_dim_start[idim] = 0.0;
  }

  MI2_RETURN ( new_icv );
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
int mi2_icv_free ( int icvid )
{
  mi2_icv_type *icvp;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_free" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN ( MI_ERROR );

  /* Detach the icv if it is attached */
  if ( icvp->volume ) {
    if ( mi2_icv_detach ( icvid ) < 0 ) {
      MI2_RETURN ( MI_ERROR );
    }
  }

  /* Free anything allocated at creation time */
  FREE ( icvp->user_maxvar );
  FREE ( icvp->user_minvar );

  /* Free the structure */
  FREE ( icvp );
  minc_icv_list[icvid] = NULL;

  /* Delete entire structure if no longer in use. */
  int new_icv;

  for ( new_icv = 0; new_icv < minc_icv_list_nalloc; new_icv++ )
    if ( minc_icv_list[new_icv] != NULL )
      break;

  if ( new_icv >= minc_icv_list_nalloc ) {
    FREE ( minc_icv_list );
    minc_icv_list_nalloc = 0;
  }

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_setdbl ( int icvid, int icv_property, double value )
{
  int ival, idim;
  mi2_icv_type *icvp;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_setdbl" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN ( MI_ERROR );

  /* Check that the icv is not attached to a file */
  if ( ! icvp->volume ) {
    milog_message ( MI2_MSG_ICVATTACHED );
    MI2_RETURN ( MI_ERROR );
  }

  /* Set the property */
  switch ( icv_property ) {
  case MI2_ICV_TYPE:
    icvp->user_type   = ( mitype_t ) value;
    icvp->user_typelen = mitype_len ( icvp->user_type );
    icvp->user_vmax   = MI2_get_default_range ( MIvalid_max, icvp->user_type );
    icvp->user_vmin   = MI2_get_default_range ( MIvalid_min, icvp->user_type );
    break;
  case MI2_ICV_DO_RANGE:
    icvp->user_do_range = value;
    break;
  case MI2_ICV_VALID_MAX:
    icvp->user_vmax   = value;
    break;
  case MI2_ICV_VALID_MIN:
    icvp->user_vmin   = value;
    break;
  case MI2_ICV_DO_NORM:
    icvp->user_do_norm = value;
    break;
  case MI2_ICV_USER_NORM:
    icvp->user_user_norm = value;
    break;
  case MI2_ICV_IMAGE_MAX:
    icvp->user_imgmax = value;
    break;
  case MI2_ICV_IMAGE_MIN:
    icvp->user_imgmin = value;
    break;
  case MI2_ICV_DO_FILLVALUE:
    icvp->user_do_fillvalue = value;
    break;
  case MI2_ICV_FILLVALUE:
    icvp->user_fillvalue = value;
    break;
  case MI2_ICV_DO_DIM_CONV:
    icvp->user_do_dimconv = value;
    break;
  case MI2_ICV_DO_SCALAR:
    icvp->user_do_scalar = value;
    break;
  case MI2_ICV_XDIM_DIR:
    ival = value;
    icvp->user_xdim_dir = ( ( ival == MI2_ICV_POSITIVE ) ||
                            ( ival == MI2_ICV_NEGATIVE ) ) ? ival : MI2_ICV_ANYDIR;
    break;
  case MI2_ICV_YDIM_DIR:
    ival = value;
    icvp->user_ydim_dir = ( ( ival == MI2_ICV_POSITIVE ) ||
                            ( ival == MI2_ICV_NEGATIVE ) ) ? ival : MI2_ICV_ANYDIR;
    break;
  case MI2_ICV_ZDIM_DIR:
    ival = value;
    icvp->user_zdim_dir = ( ( ival == MI2_ICV_POSITIVE ) ||
                            ( ival == MI2_ICV_NEGATIVE ) ) ? ival : MI2_ICV_ANYDIR;
    break;
  case MI2_ICV_NUM_IMGDIMS:
    ival = value;

    if ( ( ival < 0 ) || ( ival > MI2_MAX_IMGDIMS ) ) {
      milog_message ( MI2_MSG_BADPROP, _ ( "MI2_ICV_NUM_IMGDIMS out of range" ) );
      MI2_RETURN ( MI_ERROR );
    }

    icvp->user_num_imgdims = ival;
    break;
  case MI2_ICV_ADIM_SIZE:
    icvp->user_dim_size[0] = value;
    break;
  case MI2_ICV_BDIM_SIZE:
    icvp->user_dim_size[1] = value;
    break;
  case MI2_ICV_KEEP_ASPECT:
    icvp->user_keep_aspect = value;
    break;
  case MI2_ICV_SIGN:
  case MI2_ICV_MAXVAR:
  case MI2_ICV_MINVAR:
    milog_message ( MI2_MSG_BADPROP,
                    _ ( "Can't store a number in a string value" ) );
    MI2_RETURN ( MI_ERROR );
    break;
  default:

    /* Check for image dimension properties */
    if ( ( icv_property >= MI2_ICV_DIM_SIZE ) &&
         ( icv_property < MI2_ICV_DIM_SIZE + MI2_MAX_IMGDIMS ) ) {
      idim = icv_property - MI2_ICV_DIM_SIZE;
      icvp->user_dim_size[idim] = value;
    } else {
      milog_message ( MI2_MSG_BADPROP, "Unknown code" );
      MI2_RETURN ( MI_ERROR );
    }

    break;
  }

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_setint ( int icvid, int icv_property, int value )
{

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_setint" );

  if ( mi2_icv_setdbl ( icvid, icv_property, ( double ) value ) < 0 ) {
    MI2_RETURN ( MI_ERROR );
  }

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_setlong ( int icvid, int icv_property, long value )
{

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_setlong" );

  if ( mi2_icv_setdbl ( icvid, icv_property, ( double ) value ) < 0 ) {
    MI2_RETURN ( MI_ERROR );
  }

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_setstr ( int icvid, int icv_property, const char *value )
{
  mi2_icv_type *icvp;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_setstr" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN ( MI_ERROR );

  /* Check that the icv is not attached to a file */
  if ( !icvp->volume ) {
    milog_message ( MI2_MSG_ICVATTACHED );
    MI2_RETURN ( MI_ERROR );
  }

  /* Set the property */
  switch ( icv_property ) {
  case MI2_ICV_SIGN:
    icvp->user_sign   = MI2_get_sign_from_string ( icvp->user_type, value );
    icvp->user_vmax   = MI2_get_default_range ( MIvalid_max, icvp->user_type );
    icvp->user_vmin   = MI2_get_default_range ( MIvalid_min, icvp->user_type );
    break;
  case MI2_ICV_MAXVAR:

    if ( value != NULL ) {
      FREE ( icvp->user_maxvar );
      icvp->user_maxvar = strdup ( value );
    }

    break;
  case MI2_ICV_MINVAR:

    if ( value != NULL ) {
      FREE ( icvp->user_minvar );
      icvp->user_minvar = strdup ( value );
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
    milog_message ( MI2_MSG_BADPROP, "Can't store a string in a numeric property" );
    MI2_RETURN ( MI_ERROR );
    break;
  default:

    /* Check for image dimension properties */
    if ( ( icv_property >= MI2_ICV_DIM_SIZE ) &&
         ( icv_property < MI2_ICV_DIM_SIZE + MI2_MAX_IMGDIMS ) ) {
      milog_message ( MI2_MSG_BADPROP, "Can't store a string in a numeric property" );
      MI2_RETURN ( MI_ERROR );
    } else {
      milog_message ( MI2_MSG_BADPROP, "Unknown code" );
      MI2_RETURN ( MI_ERROR );
    }

    break;
  }

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_inqdbl ( int icvid, int icv_property, double *value )
{
  int idim;
  mi2_icv_type *icvp;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_inqdbl" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN ( MI_ERROR );

  /* Set the property */
  switch ( icv_property ) {
  case MI2_ICV_TYPE:
    *value = icvp->user_type;
    break;
  case MI2_ICV_DO_RANGE:
    *value = icvp->user_do_range;
    break;
  case MI2_ICV_VALID_MAX:
    *value = icvp->user_vmax;
    break;
  case MI2_ICV_VALID_MIN:
    *value = icvp->user_vmin;
    break;
  case MI2_ICV_DO_NORM:
    *value = icvp->user_do_norm;
    break;
  case MI2_ICV_USER_NORM:
    *value = icvp->user_user_norm;
    break;
  case MI2_ICV_IMAGE_MAX:
    *value = icvp->user_imgmax;
    break;
  case MI2_ICV_IMAGE_MIN:
    *value = icvp->user_imgmin;
    break;
  case MI2_ICV_NORM_MAX:
    *value = icvp->derv_imgmax;
    break;
  case MI2_ICV_NORM_MIN:
    *value = icvp->derv_imgmin;
    break;
  case MI2_ICV_DO_FILLVALUE:
    *value = icvp->user_do_fillvalue;
    break;
  case MI2_ICV_FILLVALUE:
    *value = icvp->user_fillvalue;
    break;
  case MI2_ICV_DO_DIM_CONV:
    *value = icvp->user_do_dimconv;
    break;
  case MI2_ICV_DO_SCALAR:
    *value = icvp->user_do_scalar;
    break;
  case MI2_ICV_XDIM_DIR:
    *value = icvp->user_xdim_dir;
    break;
  case MI2_ICV_YDIM_DIR:
    *value = icvp->user_ydim_dir;
    break;
  case MI2_ICV_ZDIM_DIR:
    *value = icvp->user_zdim_dir;
    break;
  case MI2_ICV_NUM_IMGDIMS:
    *value = icvp->user_num_imgdims;
    break;
  case MI2_ICV_NUM_DIMS:
    *value = icvp->var_ndims;

    if ( icvp->var_is_vector && icvp->user_do_scalar )
      ( *value )--;

    break;
//   case MI2_ICV_CDFID:
//     *value = icvp->cdfid;
//     break;
//   case MI2_ICV_VARID:
//     *value = icvp->varid;
//     break;
  case MI2_ICV_ADIM_SIZE:
    *value = icvp->user_dim_size[0];
    break;
  case MI2_ICV_BDIM_SIZE:
    *value = icvp->user_dim_size[1];
    break;
  case MI2_ICV_ADIM_STEP:
    *value = icvp->derv_dim_step[0];
    break;
  case MI2_ICV_BDIM_STEP:
    *value = icvp->derv_dim_step[1];
    break;
  case MI2_ICV_ADIM_START:
    *value = icvp->derv_dim_start[0];
    break;
  case MI2_ICV_BDIM_START:
    *value = icvp->derv_dim_start[1];
    break;
  case MI2_ICV_KEEP_ASPECT:
    *value = icvp->user_keep_aspect;
    break;
  case MI2_ICV_SIGN:
  case MI2_ICV_MAXVAR:
  case MI2_ICV_MINVAR:
    milog_message ( MI2_MSG_BADPROP,
                    _ ( "Tried to get icv string property as a number" ) );
    MI2_RETURN ( MI_ERROR );
    break;
  default:

    /* Check for image dimension properties */
    if ( ( icv_property >= MI2_ICV_DIM_SIZE ) &&
         ( icv_property < MI2_ICV_DIM_SIZE + MI2_MAX_IMGDIMS ) ) {
      idim = icv_property - MI2_ICV_DIM_SIZE;
      *value = icvp->user_dim_size[idim];
    } else if ( ( icv_property >= MI2_ICV_DIM_STEP ) &&
                ( icv_property < MI2_ICV_DIM_STEP + MI2_MAX_IMGDIMS ) ) {
      idim = icv_property - MI2_ICV_DIM_STEP;
      *value = icvp->derv_dim_step[idim];
    } else if ( ( icv_property >= MI2_ICV_DIM_START ) &&
                ( icv_property < MI2_ICV_DIM_START + MI2_MAX_IMGDIMS ) ) {
      idim = icv_property - MI2_ICV_DIM_START;
      *value = icvp->derv_dim_start[idim];
    } else {
      milog_message ( MI2_MSG_BADPROP, _ ( "Tried to get unknown icv property" ) );
      MI2_RETURN ( MI_ERROR );
    }

    break;
  }

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_inqint ( int icvid, int icv_property, int *value )
{
  double dvalue;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_inqint" );

  if ( mi2_icv_inqdbl ( icvid, icv_property, &dvalue ) < 0 ) {
    MI2_RETURN ( MI_ERROR );
  }

  *value = dvalue;

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_inqlong ( int icvid, int icv_property, long *value )
{
  double dvalue;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_inqlong" );

  if ( mi2_icv_inqdbl ( icvid, icv_property, &dvalue ) < 0 ) {
    MI2_RETURN ( MI_ERROR );
  }

  *value = dvalue;

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_inqstr ( int icvid, int icv_property, char *value )
{
  mi2_icv_type *icvp;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_inqstr" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN ( MI_ERROR );

  /* Set the property */
  switch ( icv_property ) {
  case MI2_ICV_SIGN:

    if ( icvp->user_sign == MI2_PRIV_SIGNED )
      ( void ) strcpy ( value, MI_SIGNED );
    else if ( icvp->user_sign == MI2_PRIV_UNSIGNED )
      ( void ) strcpy ( value, MI_UNSIGNED );
    else
      ( void ) strcpy ( value, MI_EMPTY_STRING );

    break;
  case MI2_ICV_MAXVAR:
    ( void ) strcpy ( value, icvp->user_maxvar );
    break;
  case MI2_ICV_MINVAR:
    ( void ) strcpy ( value, icvp->user_minvar );
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
    milog_message ( MI2_MSG_BADPROP,
                    _ ( "Tried to get icv numeric property as a string" ) );
    MI2_RETURN ( MI_ERROR );
    break;
  default:

    /* Check for image dimension properties */
    if ( ( ( icv_property >= MI2_ICV_DIM_SIZE ) &&
           ( icv_property < MI2_ICV_DIM_SIZE + MI2_MAX_IMGDIMS ) ) ||
         ( ( icv_property >= MI2_ICV_DIM_STEP ) &&
           ( icv_property < MI2_ICV_DIM_STEP + MI2_MAX_IMGDIMS ) ) ||
         ( ( icv_property >= MI2_ICV_DIM_START ) &&
           ( icv_property < MI2_ICV_DIM_START + MI2_MAX_IMGDIMS ) ) ) {
      milog_message ( MI2_MSG_BADPROP,
                      _ ( "Tried to get icv numeric property as a string" ) );
      MI2_RETURN ( MI_ERROR );
    } else {
      milog_message ( MI2_MSG_BADPROP,
                      _ ( "Tried to get unknown icv property" ) );
      MI2_RETURN ( MI_ERROR );
    }

    break;
  }

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_ndattach ( int icvid, mihandle_t volume )
{
  mi2_icv_type *icvp;         /* Pointer to icv structure */
  int idim;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_ndattach" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN ( MI_ERROR );

  /* If the icv is attached, then detach it */
  if ( icvp->volume ) {
    if ( mi2_icv_detach ( icvid ) < 0 ) {
      MI2_RETURN ( MI_ERROR );
    }
  }

  /* Inquire about the variable's type, sign and number of dimensions */
  if ( MI2_icv_get_type ( icvp, volume ) < 0 ) {
    MI2_RETURN ( MI_ERROR );
  }

  /* If not doing range calculations, just set derv_firstdim for
     MI2_icv_access, otherwise, call routines to calculate range and
     normalization */
  if ( !icvp->user_do_range ) {
    icvp->derv_firstdim = -1;
  } else {
    /* Get valid range */
    if ( MI2_icv_get_vrange ( icvp, volume ) < 0 ) {
      MI2_RETURN ( MI_ERROR );
    }

    /* Get normalization info */
    if ( MI2_icv_get_norm ( icvp, volume ) < 0 ) {
      MI2_RETURN ( MI_ERROR );
    }
  }

  /* Set other fields to defaults */
  icvp->var_is_vector = FALSE;
  icvp->var_vector_size = 1;
  icvp->derv_do_zero = FALSE;
  icvp->derv_do_bufsize_step = FALSE;
  icvp->derv_var_pix_off = NULL;
  icvp->derv_usr_pix_off = NULL;

  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {
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
    ( icvp->user_do_range &&
      ( ( icvp->user_vmax != icvp->var_vmax ) ||
        ( icvp->user_vmin != icvp->var_vmin ) ||
        ( icvp->user_do_norm && icvp->user_user_norm ) ||
        ( icvp->user_do_norm && ( icvp->derv_firstdim >= 0 ) ) ) );

  if ( ( icvp->derv_usr_float && icvp->derv_var_float ) )
    icvp->do_scale = FALSE;

  icvp->do_dimconvert = FALSE;

  /* Set the cdfid and varid fields */
  icvp->volume = volume;

  MI2_RETURN ( MI_NOERROR );
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_get_type
@INPUT      : icvp  - pointer to icv structure
              dset_id - dataset id
              varid - variable id
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets the type and sign of a variable for mi2_icv_attach.
@METHOD     :
@GLOBALS    :
@CALLS      : HDF5 routines
@CREATED    :
@MODIFIED   :
---------------------------------------------------------------------------- */
static int MI2_icv_get_type ( mi2_icv_type *icvp,  mihandle_t volume )
{
  int oldncopts;            /* For saving value of ncopts */
  const char *sign_string = NULL;   /* String for sign info */

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_get_type" );

  /* Inquire about the variable */
//   if ( ncvarinq ( cdfid, varid, NULL, & ( icvp->var_type ),
//                   & ( icvp->var_ndims ), icvp->var_dim, NULL ) < 0 ) {
//     MI2_RETURN ( MI_ERROR );
//   }
  miget_data_type ( volume, &icvp->var_type );
  sign_string = mitype_sign ( icvp->var_type );

  /* Check that the variable type is numeric */
  /* VF: I don't uderstand this*/
  if ( icvp->var_type == MI_TYPE_BYTE ) {
    milog_message ( MI2_MSG_VARNOTNUM );
    MI2_RETURN ( MI_ERROR );
  }

  /* Get type lengths */
  icvp->var_typelen = mitype_len ( icvp->var_type );
  icvp->user_typelen = mitype_len ( icvp->user_type );

  MI2_RETURN ( MI_NOERROR );
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
static int MI2_icv_get_vrange ( mi2_icv_type *icvp, mihandle_t volume )
{
  double vrange[2];         /* Valid range buffer */

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_get_vrange" );

  if ( miget_volume_valid_range ( volume, &vrange[0], &vrange[1] ) == MI_ERROR ) {
    MI2_RETURN ( MI_ERROR );
  }

  icvp->var_vmin = vrange[0];
  icvp->var_vmax = vrange[1];

  MI2_RETURN ( MI_NOERROR );
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
static double MI2_get_default_range ( char *what, mitype_t datatype )
{
  double range[2];

  MI2_SAVE_ROUTINE_NAME ( "MI2_get_default_range" );

  miinit_default_range ( datatype, &range[0] , &range[1] );

  if ( MI2_STRINGS_EQUAL ( what, MIvalid_max ) ) {
    MI2_RETURN ( range[1] );
  } else if ( MI2_STRINGS_EQUAL ( what, MIvalid_min ) ) {
    MI2_RETURN ( range[0] );
  } else {
//TODO:CNV      ncopts = NC_VERBOSE | NC_FATAL;
    MI2_LOG_PKG_ERROR2 ( -1, "MINC bug - this line should never be printed" );
  }

  MI2_RETURN ( MI2_DEFAULT_MIN );
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_icv_get_norm
@INPUT      : icvp  - pointer to icv structure
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets the normalization info for a variable
@METHOD     :
@GLOBALS    :
@CALLS      : HDF5 routines
@CREATED    : August 10, 1992 (Peter Neelin)
@MODIFIED   :
---------------------------------------------------------------------------- */
static int MI2_icv_get_norm ( mi2_icv_type *icvp, mihandle_t volume )
/* ARGSUSED */
{
  int oldncopts;             /* For saving value of ncopts */
  int vid[2];                /* Variable ids for max and min */
  int ndims;                 /* Number of dimensions for image max and min */
  midimhandle_t dim[MI2_MAX_VAR_DIMS];     /* Dimensions */
  int imm;                   /* Counter for looping through max and min */
  double image_range[2];
  int idim, i;


  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_get_norm" );

  /* Check for floating point or double precision values for user or
     in variable - set flag to not do normalization if needed */
  icvp->derv_var_float = ( ( icvp->var_type == MI_TYPE_DOUBLE ) ||
                           ( icvp->var_type == MI_TYPE_FLOAT ) );
  icvp->derv_usr_float = ( ( icvp->user_type == MI_TYPE_DOUBLE ) ||
                           ( icvp->user_type == MI_TYPE_FLOAT ) );

  /* Initialize first dimension over which MIimagemax or MIimagemin
     vary - assume that they don't vary at all */
  icvp->derv_firstdim = ( -1 );

  /* Look for image max, image min variables */
  miget_slice_scaling_flag ( volume, &icvp->slice_scaling );

//   icvp->imgmaxid = ncvarid ( cdfid, icvp->user_maxvar );
//   icvp->imgminid = ncvarid ( cdfid, icvp->user_minvar );


  /* Check to see if normalization to variable max, min should be done */
  if ( !icvp->user_do_norm ) {
    icvp->derv_imgmax = MI2_DEFAULT_MAX;
    icvp->derv_imgmin = MI2_DEFAULT_MIN;
  } else {

    /* Get the image min and max, either from the user definition or
       from the file. */
    if ( icvp->user_user_norm ) {
      icvp->derv_imgmax = icvp->user_imgmax;
      icvp->derv_imgmin = icvp->user_imgmin;
    } else {
      if ( miget_volume_range ( volume, &image_range[0], &image_range[1] ) < 0 ) {
        MI2_RETURN ( MI_ERROR );
      }

      icvp->derv_imgmin = image_range[0];
      icvp->derv_imgmax = image_range[1];
    }

    /* Check each of the dimensions of image-min/max variables to see
       which is the fastest varying dimension of the image variable. */
    vid[0] = icvp->imgminid;
    vid[1] = icvp->imgmaxid;

    if ( ( vid[0] != MI_ERROR ) && ( vid[1] != MI_ERROR ) ) {
      for ( imm = 0; imm < 2; imm++ ) {
        if ( miget_volume_dimensions(volume, MI_DIMCLASS_ANY, MI_DIMATTR_ALL,
                                MI_DIMORDER_FILE, MI2_MAX_VAR_DIMS, dim) < 0 ) {
          MI2_RETURN ( MI_ERROR );
        }

        for ( idim = 0; idim < ndims; idim++ ) {
          for ( i = 0; i < icvp->var_ndims; i++ ) {
            if ( icvp->var_dim[i] == dim[idim] )
              icvp->derv_firstdim = MAX ( icvp->derv_firstdim, i );
          }
        }
      }
    }
  }

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_detach ( int icvid )
{
  mi2_icv_type *icvp;
  int idim;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_detach" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN ( MI_ERROR );

  /* Check that the icv is in fact attached */
  if ( !icvp->volume )
    MI2_RETURN ( MI_NOERROR );

  /* Free the pixel offset arrays */
  if ( icvp->derv_var_pix_off != NULL )
    FREE ( icvp->derv_var_pix_off );

  if ( icvp->derv_usr_pix_off != NULL )
    FREE ( icvp->derv_usr_pix_off );

  /* Reset values that are read-only (and set when attached) */
  icvp->derv_imgmax = MI2_DEFAULT_MAX;
  icvp->derv_imgmin = MI2_DEFAULT_MIN;

  for ( idim = 0; idim < MI2_MAX_IMGDIMS; idim++ ) {
    icvp->derv_dim_step[idim] = 0.0;
    icvp->derv_dim_start[idim] = 0.0;
  }

  icvp->volume = NULL;

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_get ( int icvid, long start[], long count[], void *values )
{
  mi2_icv_type *icvp;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_get" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN ( MI_ERROR );

  /* Get the data */
  if ( MI2_icv_access ( MI2_PRIV_GET, icvp, start, count, values ) < 0 ) {
    MI2_RETURN ( MI_ERROR );
  }

  MI2_RETURN ( MI_NOERROR );
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
int mi2_icv_put ( int icvid, long start[], long count[], void *values )
{
  mi2_icv_type *icvp;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_put" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN ( MI_ERROR );


  if ( MI2_icv_access ( MI2_PRIV_PUT, icvp, start, count, values ) < 0 ) {
    MI2_RETURN ( MI_ERROR );
  }

  MI2_RETURN ( MI_NOERROR );
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
static int MI2_icv_access ( int operation, mi2_icv_type *icvp, long start[],
                            long count[], void *values )
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

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_access" );

  /* Check that icv is attached to a variable */
  if ( !icvp->volume ) {
    milog_message ( MI2_MSG_ICVNOTATTACHED );

    MI2_RETURN ( MI_ERROR );
  }

  /* Zero the user's buffer if needed */
  if ( ( operation == MI2_PRIV_GET ) && ( icvp->derv_do_zero ) )
    if ( MI2_icv_zero_buffer ( icvp, count, values ) < 0 ) {
      MI2_RETURN ( MI_ERROR );
    }

  /* Translate icv coordinates to variable coordinates */
  if ( MI2_icv_coords_tovar ( icvp, start, count, var_start, var_count ) < 0 ) {
    MI2_RETURN ( MI_ERROR );
  }

  /* Save icv coordinates for future reference (for dimension conversion
     routines) */
  ndims = icvp->var_ndims;

  if ( icvp->var_is_vector && icvp->user_do_scalar )
    ndims--;

  for ( idim = 0; idim < ndims; idim++ ) {
    icvp->derv_icv_start[idim] = start[idim];
    icvp->derv_icv_count[idim] = count[idim];
  }

  /* Do we care about getting variable in convenient increments ?
     Only if we are getting data and the icv structure wants it */
  if ( ( operation == MI2_PRIV_GET ) && ( icvp->derv_do_bufsize_step ) )
    bufsize_step = icvp->derv_bufsize_step;
  else
    bufsize_step = NULL;

  /* Set up variables for looping through variable. The biggest chunk that
     we can get in one call is determined by the subscripts of MIimagemax
     and MIimagemin. These must be constant over the chunk that we get if
     we are doing normalization. */
  for ( idim = 0; idim < icvp->var_ndims; idim++ ) {
    chunk_start[idim] = var_start[idim];
    var_end[idim] = var_start[idim] + var_count[idim];
  }

  ( void ) miset_coords ( icvp->var_ndims, 1L, chunk_count );

  /* Get size of chunk in user's buffer. Dimension conversion routines
     don't need the buffer pointer incremented - they do it themselves */
  if ( !icvp->do_dimconvert )
    chunk_size = mitype_len ( icvp->user_type );
  else
    chunk_size = 0;

  for ( idim = MAX ( icvp->derv_firstdim + 1, 0 ); idim < icvp->var_ndims; idim++ ) {
    chunk_count[idim] = var_count[idim];
    chunk_size *= chunk_count[idim];
  }

  firstdim = MAX ( icvp->derv_firstdim, 0 );

  /* Loop through variable */
  chunk_values = values;

  while ( chunk_start[0] < var_end[0] ) {

    /* Set the do_fillvalue flag if the user wants it and we are doing
       a get. We must do it inside the loop since the scale factor
       calculation can change it if the scale is zero. (Fillvalue checking
       is always done if the the scale is zero.) */
    icvp->do_fillvalue =
      icvp->user_do_fillvalue && ( operation == MI2_PRIV_GET );
    icvp->fill_valid_min = icvp->var_vmin;
    icvp->fill_valid_max = icvp->var_vmax;

    /* Calculate scale factor */
    if ( icvp->do_scale ) {
      if ( MI2_icv_calc_scale ( operation, icvp, chunk_start ) < 0 ) {
        MI2_RETURN ( MI_ERROR );
      }
    }

// fprintf(stderr, "Getting values at %p\n", chunk_start);

    /* Get the values */
    if ( MI2_varaccess ( operation, icvp->volume,
                         chunk_start, chunk_count,
                         icvp->user_type, icvp->user_sign,
                         chunk_values, bufsize_step, icvp ) < 0 ) {
      MI2_RETURN ( MI_ERROR );
    }

    /* Increment the start counter */
    chunk_start[firstdim] += chunk_count[firstdim];

    for ( idim = firstdim;
          ( idim > 0 ) && ( chunk_start[idim] >= var_end[idim] ); idim-- ) {
      chunk_start[idim] = var_start[idim];
      chunk_start[idim - 1]++;
    }

    /* Increment the pointer to values */
    chunk_values = ( void * ) ( ( char * ) chunk_values + ( size_t ) chunk_size );

  }

  MI2_RETURN ( MI_NOERROR );
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
static int MI2_icv_zero_buffer ( mi2_icv_type *icvp, long count[], void *values )
{
  double zeroval, zerobuf;
  void *zerostart;
  int zerolen, idim, ndims;
  char *bufptr, *bufend, *zeroptr, *zeroend;
  long buflen;

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_zero_buffer" );

  /* Create a zero pixel and get its size */
  zerostart = ( void * ) ( &zerobuf );

  if ( icvp->do_scale )
    zeroval = icvp->offset;
  else
    zeroval = 0.0;

  {
    MI2_FROM_DOUBLE ( zeroval, icvp->user_type, icvp->user_sign, zerostart )
  }
  zerolen = icvp->user_typelen;

  /* Get the buffer size */
  ndims = icvp->var_ndims;

  if ( icvp->var_is_vector && icvp->user_do_scalar )
    ndims--;

  buflen = zerolen;

  for ( idim = 0; idim < ndims; idim++ )
    buflen *= count[idim];

  /* Loop through the buffer, copying the zero pixel */
  bufend = ( char * ) values + buflen;
  zeroend = ( char * ) zerostart + zerolen;

  for ( bufptr = ( char * ) values, zeroptr = ( char * ) zerostart;
        bufptr < bufend; bufptr++, zeroptr++ ) {
    if ( zeroptr >= zeroend )
      zeroptr = ( char * ) zerostart;

    *bufptr = *zeroptr;
  }

  MI2_RETURN ( MI_NOERROR );
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
static int MI2_icv_coords_tovar ( mi2_icv_type *icvp,
                                  long icv_start[], long icv_count[],
                                  long var_start[], long var_count[] )
{
  int i, j;
  int num_non_img_dims;
  long coord, last_coord, icv_dim_size;

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_coords_tovar" );

  /* Do we have to worry about dimension conversions? If not, then
     just copy the vectors and return. */
  if ( !icvp->do_dimconvert ) {
    for ( i = 0; i < icvp->var_ndims; i++ ) {
      var_count[i] = icv_count[i];
      var_start[i] = icv_start[i];
    }

    MI2_RETURN ( MI_NOERROR );
  }

  /* Get the number of non image dimensions */
  num_non_img_dims = icvp->var_ndims - icvp->user_num_imgdims;

  if ( icvp->var_is_vector )
    num_non_img_dims--;

  /* Go through first, non-image dimensions */
  for ( i = 0; i < num_non_img_dims; i++ ) {
    var_count[i] = icv_count[i];
    var_start[i] = icv_start[i];
  }

  /* Go through image dimensions */
  for ( i = num_non_img_dims, j = icvp->user_num_imgdims - 1;
        i < num_non_img_dims + icvp->user_num_imgdims; i++, j-- ) {
    /* Check coordinates. */
    icv_dim_size = ( icvp->user_dim_size[j] > 0 ) ?
                   icvp->user_dim_size[j] : icvp->var_dim_size[j];
    last_coord = icv_start[i] + icv_count[i] - 1;

    if ( ( icv_start[i] < 0 ) || ( icv_start[i] >= icv_dim_size ) ||
         ( last_coord < 0 ) || ( last_coord >= icv_dim_size ) ||
         ( icv_count[i] < 0 ) ) {
      milog_message ( MI2_MSG_ICVCOORDS );
      MI2_RETURN ( MI_ERROR );
    }

    /* Remove offset */
    coord = icv_start[i] - icvp->derv_dim_off[j];

    /* Check for growing or shrinking */
    if ( icvp->derv_dim_grow[j] ) {
      var_count[i] = ( icv_count[i] + icvp->derv_dim_scale[j] - 1 )
                     / icvp->derv_dim_scale[j];
      coord /= icvp->derv_dim_scale[j];
    } else {
      var_count[i] = icv_count[i] * icvp->derv_dim_scale[j];
      coord *= icvp->derv_dim_scale[j];
    }

    /* Check for flipping */
    if ( icvp->derv_dim_flip[j] )
      coord = icvp->var_dim_size[j] - coord -
              ( ( icv_count != NULL ) ? var_count[i] : 0L );

    var_start[i] = coord;
    /* Check for indices out of variable bounds (but in icv bounds) */
    last_coord = var_start[i] + var_count[i];

    if ( ( var_start[i] < 0 ) || ( last_coord >= icvp->var_dim_size[j] ) ) {
      if ( var_start[i] < 0 )
        var_start[i] = 0;

      if ( last_coord >= icvp->var_dim_size[j] )
        last_coord = icvp->var_dim_size[j] - 1;

      var_count[i] = last_coord - var_start[i] + 1;
    }
  }

  /* Check for vector dimension */
  if ( icvp->var_is_vector ) {
    if ( icvp->user_do_scalar ) {
      var_count[icvp->var_ndims - 1] = icvp->var_vector_size;
      var_start[icvp->var_ndims - 1] = 0;
    } else {
      var_count[icvp->var_ndims - 1] = icv_count[icvp->var_ndims - 1];
      var_start[icvp->var_ndims - 1] = icv_start[icvp->var_ndims - 1];
    }
  }

  MI2_RETURN ( MI_NOERROR );
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
static int MI2_icv_calc_scale ( int operation, mi2_icv_type *icvp, long coords[] )
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

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_calc_scale" );

  /* Set variable valid range */
  var_vmax = icvp->var_vmax;
  var_vmin = icvp->var_vmin;

  /* Set image max/min for user and variable values depending on whether
     normalization should be done or not. Whenever floating-point values
     are involved, some type of normalization is done. When the icv type
     is floating point, normalization is always done. When the file type
     is floating point and the icv type is integer, slices are normalized
     to the real range of the slice (or chunk being read). */
  if ( !icvp->derv_var_float && !icvp->derv_usr_float && !icvp->user_do_norm ) {
    usr_imgmax = var_imgmax = MI2_DEFAULT_MAX;
    usr_imgmin = var_imgmin = MI2_DEFAULT_MIN;
  } else {

    /* Get the real range for the slice or chunk that is being examined */
    slice_imgmax = MI2_DEFAULT_MAX;
    slice_imgmin = MI2_DEFAULT_MIN;

    if ( ( !icvp->derv_var_float || !icvp->user_do_norm ) &&
         ( icvp->imgmaxid != MI_ERROR ) && ( icvp->imgminid != MI_ERROR ) ) {
      
      if ( mitranslate_coords ( icvp->cdfid, icvp->varid, coords,
                                icvp->imgmaxid, mmcoords ) == NULL )
        MI2_RETURN ( MI_ERROR );
      

      if ( mivarget1 ( icvp->cdfid, icvp->imgmaxid, mmcoords,
                       MI_TYPE_DOUBLE, NULL, &slice_imgmax ) < 0 ) {
        MI2_RETURN ( MI_ERROR );
      }

      if ( mitranslate_coords ( icvp->cdfid, icvp->varid, coords,
                                icvp->imgminid, mmcoords ) == NULL ) {
        MI2_RETURN ( MI_ERROR );
      }

      if ( mivarget1 ( icvp->cdfid, icvp->imgminid, mmcoords,
                       MI_TYPE_DOUBLE, NULL, &slice_imgmin ) < 0 ) {
        MI2_RETURN ( MI_ERROR );
      }
    }

    /* Get the user real range */
    if ( icvp->user_do_norm ) {
      usr_imgmax = icvp->derv_imgmax;
      usr_imgmin = icvp->derv_imgmin;
    } else {
      usr_imgmax = slice_imgmax;
      usr_imgmin = slice_imgmin;
    }

    /* Get the file real range */
    if ( icvp->derv_var_float ) {
      var_imgmax = var_vmax;
      var_imgmin = var_vmin;
    } else {
      var_imgmax = slice_imgmax;
      var_imgmin = slice_imgmin;
    }
  }

  /* Prevent scaling between file floats and real value */
  if ( icvp->derv_var_float ) {
    var_imgmax = var_vmax;
    var_imgmin = var_vmin;
  }

  /* Get user valid range */
  if ( icvp->derv_usr_float ) {
    usr_vmax = usr_imgmax;
    usr_vmin = usr_imgmin;
  } else {
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

  if ( icvp->derv_usr_float ) {
    usr_imgmax = usr_vmax = MI2_DEFAULT_MAX;
    usr_imgmin = usr_vmin = MI2_DEFAULT_MIN;
  }

  if ( icvp->derv_var_float ) {
    var_imgmax = var_vmax = MI2_DEFAULT_MAX;
    var_imgmin = var_vmin = MI2_DEFAULT_MIN;
  }

  /* Calculate scale and offset for MI2_PRIV_GET */

  /* Scale */
  denom = usr_imgmax - usr_imgmin;

  if ( denom != 0.0 )
    usr_scale = ( usr_vmax - usr_vmin ) / denom;
  else
    usr_scale = 0.0;

  denom = var_vmax - var_vmin;

  if ( denom != 0.0 )
    icvp->scale = usr_scale * ( var_imgmax - var_imgmin ) / denom;
  else
    icvp->scale = 0.0;

  /* Offset */
  icvp->offset = usr_vmin - icvp->scale * var_vmin
                 + usr_scale * ( var_imgmin - usr_imgmin );

  /* If we want a MI2_PRIV_PUT, invert */
  if ( operation == MI2_PRIV_PUT ) {
    if ( icvp->scale != 0.0 ) {
      icvp->offset = ( -icvp->offset ) / icvp->scale;
      icvp->scale  = 1.0 / icvp->scale;
    } else {
      icvp->offset = var_vmin;
      icvp->scale  = 0.0;
    }
  }

  /* Do fill value checking if scale is zero */
  if ( icvp->scale == 0.0 ) {

    /* Check for floating point on both sides of conversion. We should
       not be doing scaling in this case, but we will check to be safe. */
    if ( icvp->derv_var_float && icvp->derv_usr_float ) {
      icvp->do_scale = FALSE;
      icvp->do_fillvalue = FALSE;
    }

    else {      /* Not pure floating point */

      icvp->do_fillvalue = TRUE;

      /* For output, set the range properly depending on whether the user
         type is floating point or not */
      if ( operation == MI2_PRIV_PUT ) {
        if ( icvp->derv_usr_float ) {
          icvp->fill_valid_min = var_imgmin_true;
          icvp->fill_valid_max = var_imgmax_true;
        } else if ( usr_scale != 0.0 ) {
          icvp->fill_valid_min =
            usr_vmin + ( var_imgmin_true - usr_imgmin ) / usr_scale;
          icvp->fill_valid_max =
            usr_vmin + ( var_imgmax_true - usr_imgmin ) / usr_scale;
        } else {
          icvp->fill_valid_min = usr_vmin;
          icvp->fill_valid_max = usr_vmax;
        }
      }        /* If output operation */

    }        /* If not pure floating point */

  }       /* If scale == 0.0 */

  MI2_RETURN ( MI_NOERROR );
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
static mi2_icv_type *MI2_icv_chkid ( int icvid )
{
  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_chkid" );

  /* Check icv id */
  if ( ( icvid < 0 ) || ( icvid >= minc_icv_list_nalloc ) ||
       ( minc_icv_list[icvid] == NULL ) ) {
    milog_message ( MI2_MSG_BADICV );
    MI2_RETURN ( ( void * ) NULL );
  }

  MI2_RETURN ( minc_icv_list[icvid] );
}



/* ----------------------------- MNI Header -----------------------------------
@NAME       : mi2_icv_attach
@INPUT      : icvid - icv id
              cdfid - cdf file id
              varid - cdf variable id
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Attaches an open MINC file and variable to an image conversion
              variable for subsequent access through miicvget and miicvput.

@METHOD     :
@GLOBALS    :
@CALLS      : MINC2 routines
@CREATED    : September 9, 1992 (Peter Neelin)
@MODIFIED   :
---------------------------------------------------------------------------- */
int mi2_icv_attach ( int icvid, mihandle_t volume )
{
  mi2_icv_type *icvp;         /* Pointer to icv structure */
  long size_diff, user_dim_size;
  int idim;

  MI2_SAVE_ROUTINE_NAME ( "mi2_icv_attach" );

  /* Check icv id */
  if ( ( icvp = MI2_icv_chkid ( icvid ) ) == NULL )
    MI2_RETURN_ERROR ( MI_ERROR );

  /* Call routine to set variables for everything except dimension
     conversion */
  {
    MI2_CHK_ERR ( mi2_icv_ndattach ( icvid, volume ) )
  }

  /* Check to see if we need to worry about dimension conversion */
  if ( !icvp->user_do_dimconv ) {
    MI2_RETURN ( MI_NOERROR );
  }

  /* Reset cdfid and varid in icv structure in case something goes wrong
     in dimension conversion calculations */
  icvp->volume = 0;
  icvp->varid = MI_ERROR;

  /* Get dimensions info */
  {
    MI2_CHK_ERR ( MI2_icv_get_dim ( icvp, volume ) )
  }

  /* Set the do_dimconvert field of icv structure
     We do dimension conversion if any dimension needs flipping, scaling
     or offset, or if we have to convert from vector to scalar. */

  icvp->do_dimconvert = ( icvp->user_do_scalar && icvp->var_is_vector );

  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {
    if ( icvp->derv_dim_flip[idim] || ( icvp->derv_dim_scale[idim] != 1 ) ||
         ( icvp->derv_dim_off[idim] != 0 ) )
      icvp->do_dimconvert = TRUE;
  }

  icvp->dimconvert_func = MI2_icv_dimconvert;

  /* Check if we have to zero user's buffer on GETs */
  icvp->derv_do_zero = FALSE;

  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {
    user_dim_size = ( ( icvp->user_dim_size[idim] <= 0 ) ?
                      icvp->var_dim_size[idim] :
                      icvp->user_dim_size[idim] );

    if ( icvp->derv_dim_grow[idim] )
      size_diff = user_dim_size -
                  icvp->var_dim_size[idim] * icvp->derv_dim_scale[idim];
    else
      size_diff = user_dim_size - 1 -
                  ( icvp->var_dim_size[idim] - 1 )
                  / icvp->derv_dim_scale[idim];

    if ( ( icvp->derv_dim_off[idim] != 0 ) || ( size_diff != 0 ) )
      icvp->derv_do_zero = TRUE;
  }

  /* Set the cdfid and varid fields */
  icvp->volume = volume;

  MI2_RETURN ( MI_NOERROR );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_icv_get_dim
@INPUT      : icvp  - pointer to icv structure
              cdfid - cdf file id
              varid - variable id
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Gets dimension info for the icv
@METHOD     :
@GLOBALS    :
@CALLS      : NetCDF routines
@CREATED    : August 10, 1992 (Peter Neelin)
@MODIFIED   :
---------------------------------------------------------------------------- */
static int MI2_icv_get_dim ( mi2_icv_type *icvp, mihandle_t volume )
/* ARGSUSED */
{
  int oldncopts;             /* For saving value of ncopts */
  char dimname[MI2_MAX_DIM_NAME]; /* Dimensions name */
  int idim;                  /* Looping counter for fastest image dims */
  int subsc[MI2_MAX_IMGDIMS]; /* Subscripts for fastest image dims */
  int dimvid[MI2_MAX_IMGDIMS]; /* Variable ids for dimensions */

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_get_dim" );

  /* Check that the variable has at least icvp->user_num_imgdims dimensions */
  if ( icvp->var_ndims < icvp->user_num_imgdims ) {
    MI2_LOG_PKG_ERROR2 ( MI_ERR_TOOFEWDIMS,
                         "Variable has too few dimensions to be an image" );
    MI2_RETURN_ERROR ( MI_ERROR );
  }

  /* Check the first dimensions of the variable */
  /*TODO: convert to MINC2 call*/
  MI_CHK_ERR ( ncdiminq ( cdfid, icvp->var_dim[icvp->var_ndims - 1], dimname,
                          & ( icvp->var_vector_size ) ) )
  icvp->var_is_vector = MI2_STRINGS_EQUAL ( dimname, MIvector_dimension );

  /* Check that the variable has at least icvp->user_num_imgdims+1
     dimensions if it is a vector field */
  if ( icvp->var_is_vector && ( icvp->var_ndims < icvp->user_num_imgdims + 1 ) ) {
    MI2_LOG_PKG_ERROR2 ( MI_ERR_TOOFEWDIMS,
                         "Variable has too few dimensions to be an image" );
    MI2_RETURN_ERROR ( MI_ERROR );
  }

  /* Check for dimension flipping and get dimension sizes */

  /* Get subscripts for first icvp->user_num_imgdims dimensions */
  subsc[0] = ( icvp->var_is_vector ) ? icvp->var_ndims - 2 : icvp->var_ndims - 1;

  for ( idim = 1; idim < icvp->user_num_imgdims; idim++ )
    subsc[idim] = subsc[idim - 1] - 1;

  /* Get dimension variable ids */
  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {
    {
      /*TODO: convert to MINC2 call*/
      MI_CHK_ERR ( ncdiminq ( cdfid, icvp->var_dim[subsc[idim]], dimname,
                              & ( icvp->var_dim_size[idim] ) ) )
    };
    /*oldncopts = ncopts;
    ncopts = 0;*/
    /*TODO: convert to MINC2 call*/
    dimvid[idim] = ncvarid ( cdfid, dimname );
    /*ncopts = oldncopts;*/
  }

  /* Check for flipping */
  {
    MI2_CHK_ERR ( MI2_get_dim_flip ( icvp, cdfid, dimvid, subsc ) )
  }

  /* Check for scaling of dimension */
  {
    MI2_CHK_ERR ( MI2_get_dim_scale ( icvp, cdfid, dimvid ) )
  }

  /* Check for variable buffer size increments */
  {
    MI2_CHK_ERR ( MI2_get_dim_bufsize_step ( icvp, subsc ) )
  }

  /* Get information for dimension conversion */
  {
    MI2_CHK_ERR ( MI2_icv_get_dim_conversion ( icvp, subsc ) )
  }

  MI2_RETURN ( MI_NOERROR );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_icv_get_dim_flip
@INPUT      : icvp  - icv pointer
              cdfid - cdf file id
              dimvid - variable id
              subsc - array of dimension subscripts for fastest varying
                 dimensions
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Checks for flipping of icv.
@METHOD     :
@GLOBALS    :
@CALLS      : NetCDF routines
@CREATED    : September 1, 1992 (Peter Neelin)
@MODIFIED   :
---------------------------------------------------------------------------- */
static int MI2_get_dim_flip ( mi2_icv_type *icvp, mihandle_t volume, int dimvid[],
                              int subsc[] )
{
  int oldncopts;             /* For saving value of ncopts */
  char dimname[MI2_MAX_DIM_NAME]; /* Dimensions name */
  int dim_dir;               /* Desired direction for current dimension */
  double dimstep;            /* Dimension step size (and direction) */
  int idim;

  MI2_SAVE_ROUTINE_NAME ( "MI2_get_dim_flip" );

  /* Loop through fast dimensions */

  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {

    /* Get name of the dimension */
    {
      /*TODO: convert to MINC2 call*/
      MI_CHK_ERR ( ncdiminq ( cdfid, icvp->var_dim[subsc[idim]], dimname,
                              NULL ) )
    }

    /* Should we look for dimension flipping? */
    icvp->derv_dim_flip[idim] = FALSE;

    if ( MI2_STRINGS_EQUAL ( dimname, MIxspace ) ||
         MI2_STRINGS_EQUAL ( dimname, MIxfrequency ) )
      dim_dir = icvp->user_xdim_dir;
    else if ( MI2_STRINGS_EQUAL ( dimname, MIyspace ) ||
              MI2_STRINGS_EQUAL ( dimname, MIyfrequency ) )
      dim_dir = icvp->user_ydim_dir;
    else if ( MI2_STRINGS_EQUAL ( dimname, MIzspace ) ||
              MI2_STRINGS_EQUAL ( dimname, MIzfrequency ) )
      dim_dir = icvp->user_zdim_dir;
    else
      dim_dir = MI2_ICV_ANYDIR;

    /* Look for variable corresponding to dimension */
    if ( dim_dir != MI2_ICV_ANYDIR ) { /* Look for flipping? */

      /* Get the value of the MIstep attribute to determine whether flipping
         is needed. Assume that direction is positive if no step is
         provided. */
      dimstep = 1.0;

      if ( dimvid[idim] != MI_ERROR ) { /* if dimension exists */
        /*
        oldncopts = ncopts;
        ncopts = 0;*/
        /*TODO: convert to MINC2 call*/
        miattget1 ( cdfid, dimvid[idim], MIstep, NC_DOUBLE, &dimstep );
        /*ncopts = oldncopts;*/
      }                           /* if dimension exists */

      if ( dim_dir == MI2_ICV_POSITIVE )
        icvp->derv_dim_flip[idim] = ( dimstep < 0.0 );
      else if ( dim_dir == MI2_ICV_NEGATIVE )
        icvp->derv_dim_flip[idim] = ( dimstep > 0.0 );
    }                          /* if look for flipping */

  }                           /* for each dimension */

  MI2_RETURN ( MI_NOERROR );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_icv_get_dim_scale
@INPUT      : icvp  - icv pointer
              cdfid - cdf file id
              dimvid - dimension variable id
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Checks for scaling of images
@METHOD     :
@GLOBALS    :
@CALLS      : NetCDF routines
@CREATED    : September 1, 1992 (Peter Neelin)
@MODIFIED   :
---------------------------------------------------------------------------- */
static int MI2_get_dim_scale ( mi2_icv_type *icvp, mihandle_t volume, int dimvid[] )
{
  int oldncopts;             /* For saving value of ncopts */
  int min_grow, dim_grow;
  int min_scale, dim_scale;
  double dimstep, dimstart;
  int idim;
  long user_dim_size;

  MI2_SAVE_ROUTINE_NAME ( "MI2_get_dim_scale" );

  /* Loop through dimensions, calculating scale and looking for smallest
     one. For each dimension, check to see if we need to shrink or grow the
     image. (This is get-oriented: grow is TRUE if the variable dimension
     has to be expanded to fit the user's dimensions). */

  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {

    /* Check to see if we user wants resize */
    if ( icvp->user_dim_size[idim] <= 0 ) {
      icvp->derv_dim_grow[idim] = TRUE;
      icvp->derv_dim_scale[idim] = 1;
    } else {

      /* Check for growing or shrinking */
      icvp->derv_dim_grow[idim] =
        ( icvp->var_dim_size[idim] <= icvp->user_dim_size[idim] );

      /* If growing */
      if ( icvp->derv_dim_grow[idim] ) {
        /* Get scale so that whole image fits in user array */
        icvp->derv_dim_scale[idim] =
          icvp->user_dim_size[idim] / icvp->var_dim_size[idim];
      }

      /* Otherwise, shrinking. Things are complicated by the fact that
         the external variable must fit completely in the user's array */
      else {

        icvp->derv_dim_scale[idim] = 1 +
                                     ( icvp->var_dim_size[idim] - 1 ) / icvp->user_dim_size[idim];
      }
    }           /* if user wants resizing */

    /* Check for smallest scale */
    if ( idim == 0 ) {
      min_grow = icvp->derv_dim_grow[idim];
      min_scale = icvp->derv_dim_scale[idim];
    } else {
      dim_grow  = icvp->derv_dim_grow[idim];
      dim_scale = icvp->derv_dim_scale[idim];

      /* Check for one of three conditions :
            (1) smallest so far is growing, but this dim is shrinking
            (2) both are growing and this dim has smaller scale
            (3) both are shrinking and this dim has larger scale */
      if ( ( min_grow && !dim_grow ) ||
           ( ( min_grow && dim_grow ) &&
             ( min_scale > dim_scale ) ) ||
           ( ( !min_grow && !dim_grow ) &&
             ( min_scale < dim_scale ) ) ) {
        min_grow = dim_grow;
        min_scale = dim_scale;
      }
    }           /* if not first dim */

  }           /* for each dimension, get scale */

  /* Loop through dimensions, resetting scale if needed, setting offset
     and pixel step and start */

  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {

    /* Check for aspect ratio */
    if ( icvp->user_keep_aspect ) {
      icvp->derv_dim_grow[idim]  = min_grow;
      icvp->derv_dim_scale[idim] = min_scale;
    }

    /* Get user's buffer size */
    user_dim_size = ( ( icvp->user_dim_size[idim] <= 0 ) ?
                      icvp->var_dim_size[idim] :
                      icvp->user_dim_size[idim] );

    /* Set offset of variable into user's image */

    /* If growing */
    if ( icvp->derv_dim_grow[idim] ) {
      /* Calculate remainder and split it in half */
      icvp->derv_dim_off[idim] =
        ( user_dim_size -
          icvp->var_dim_size[idim] * icvp->derv_dim_scale[idim] )
        / 2;
    }
    /* Otherwise, shrinking. Things are complicated by the fact that
       the external variable must fit completely in the user's array */
    else {
      /* Calculate remainder and split it in half */
      icvp->derv_dim_off[idim] =
        ( user_dim_size - 1 -
          ( icvp->var_dim_size[idim] - 1 )
          / icvp->derv_dim_scale[idim] ) / 2 ;
    }

    /* Get pixel step and start for variable and calculate for user.
       Look for them in the dimension variable (if MIstep is not
       there, then use defaults step = 1.0, start = 0.0 */
    /*oldncopts = ncopts;
    ncopts = 0;
    dimstep = 1.0;*/

    /*TODO: convert to MINC2 call*/
    miattget1 ( cdfid, dimvid[idim], MIstep, NC_DOUBLE, &dimstep );

    /* Flip dimstep if needed */
    if ( icvp->derv_dim_flip[idim] )
      dimstep *= ( -1 );

    /* Get step size for user's image */
    icvp->derv_dim_step[idim] = icvp->derv_dim_grow[idim] ?
                                dimstep / icvp->derv_dim_scale[idim] :
                                dimstep * icvp->derv_dim_scale[idim];
    /* Get start position for user's image - if no MIstart for
       dimension, then assume 0.0 */
    dimstart = 0.0;
    /*TODO: convert to MINC2 call*/
    miattget1 ( cdfid, dimvid[idim], MIstart, NC_DOUBLE, &dimstart );

    /* Flip dimstart if needed */
    if ( icvp->derv_dim_flip[idim] )
      dimstart -= dimstep * ( icvp->var_dim_size[idim] - 1 );

    /* Calculate start position */
    icvp->derv_dim_start[idim] = dimstart +
                                 ( icvp->derv_dim_step[idim] - dimstep ) / 2.0 -
                                 icvp->derv_dim_off[idim] * icvp->derv_dim_step[idim];
    /*ncopts = oldncopts;*/

  }                 /* for each dimension */

  MI2_RETURN ( MI_NOERROR );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_icv_get_dim_bufsize_step
@INPUT      : icvp  - icv pointer
              subsc - array of dimension subscripts for fastest varying
                 dimensions
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Sets the variables giving variable buffer size
@METHOD     :
@GLOBALS    :
@CALLS      : NetCDF routines
@CREATED    : September 3, 1992 (Peter Neelin)
@MODIFIED   :
---------------------------------------------------------------------------- */
static int MI2_get_dim_bufsize_step ( mi2_icv_type *icvp, int subsc[] )
{
  int idim;

  MI2_SAVE_ROUTINE_NAME ( "MI2_get_dim_bufsize_step" );

  /* Set default buffer size step */
  for ( idim = 0; idim < MI2_MAX_VAR_DIMS; idim++ )
    icvp->derv_bufsize_step[idim] = 1;

  /* Check for converting vector to scalar */
  icvp->derv_do_bufsize_step = ( icvp->var_is_vector && icvp->user_do_scalar );

  if ( icvp->derv_do_bufsize_step )
    icvp->derv_bufsize_step[icvp->var_ndims - 1] = icvp->var_vector_size;

  /* Check each dimension to see if we need to worry about the variable
     buffer size. This only occurs if we are shrinking the dimension from
     the variable buffer to the user buffer. */
  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {
    if ( !icvp->derv_dim_grow[idim] )
      icvp->derv_bufsize_step[subsc[idim]] = icvp->derv_dim_scale[idim];

    if ( icvp->derv_bufsize_step[subsc[idim]] != 1 )
      icvp->derv_do_bufsize_step = TRUE;
  }

  MI2_RETURN ( MI_NOERROR );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_icv_get_dim_conversion
@INPUT      : icvp  - icv pointer
              subsc - array of dimension subscripts for fastest varying
                 dimensions
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Sets the variables for dimensions converions
@METHOD     :
@GLOBALS    :
@CALLS      : NetCDF routines
@CREATED    : September 8, 1992 (Peter Neelin)
@MODIFIED   :
---------------------------------------------------------------------------- */
static int MI2_icv_get_dim_conversion ( mi2_icv_type *icvp, int subsc[] )
/* ARGSUSED */
{
  int idim;

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_get_dim_conversion" );

  /* Find out whether we need to compress variable or user buffer */
  icvp->derv_var_compress = ( icvp->var_is_vector && icvp->user_do_scalar );
  icvp->derv_usr_compress = FALSE;

  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {
    if ( icvp->derv_dim_scale[idim] != 1 ) {
      if ( icvp->derv_dim_grow[idim] )
        icvp->derv_usr_compress = TRUE;
      else
        icvp->derv_var_compress = TRUE;
    }
  }

  /* Get the fastest varying dimension in user's buffer */
  icvp->derv_dimconv_fastdim = icvp->var_ndims - 1;

  if ( icvp->var_is_vector && icvp->user_do_scalar )
    icvp->derv_dimconv_fastdim--;

  /* Find out how many pixels to compress/expand for variable and user
     buffers and allocate arrays */
  if ( icvp->var_is_vector && icvp->user_do_scalar )
    icvp->derv_var_pix_num = icvp->var_vector_size;
  else
    icvp->derv_var_pix_num = 1;

  icvp->derv_usr_pix_num = 1;

  for ( idim = 0; idim < icvp->user_num_imgdims; idim++ ) {
    if ( icvp->derv_dim_grow[idim] )
      icvp->derv_usr_pix_num *= icvp->derv_dim_scale[idim];
    else
      icvp->derv_var_pix_num *= MIN ( icvp->var_dim_size[idim],
                                      icvp->derv_dim_scale[idim] );
  }

  icvp->derv_var_pix_off = MALLOC ( icvp->derv_var_pix_num, long );
  icvp->derv_usr_pix_off = MALLOC ( icvp->derv_usr_pix_num, long );

  if ( ( icvp->derv_var_pix_off == NULL ) || ( icvp->derv_usr_pix_off == NULL ) ) {
    MI2_LOG_SYS_ERROR1 ( "MI2_icv_get_dim_conversion" );
    MI2_RETURN_ERROR ( MI_ERROR );
  }

  MI2_RETURN ( MI_NOERROR );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_icv_dimconvert
@INPUT      : operation  - MI_PRIV_GET or MI_PRIV_PUT
              icvp       - icv structure pointer
              start      - start passed by user
              count      - count passed by user
              values     - pointer to user's data area (for put)
              bufstart   - start of variable buffer
              bufcount   - count of variable buffer
              buffer     - pointer to variable buffer (for get)
@OUTPUT     : values     - pointer to user's data area (for get)
              buffer     - pointer to variable buffer (for put)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Converts values and dimensions from an input buffer to the
              user's buffer. Called by MI_var_action.
@METHOD     :
@GLOBALS    :
@CALLS      : NetCDF routines
@CREATED    : August 27, 1992 (Peter Neelin)
@MODIFIED   :
---------------------------------------------------------------------------- */
static int MI2_icv_dimconvert ( int operation, mi2_icv_type *icvp,
                                long start[], long count[], void *values,
                                long bufstart[], long bufcount[], void *buffer )
{
  mi2_icv_dimconv_type dim_conv_struct;
  mi2_icv_dimconv_type *dcp;
  double sum0, sum1;           /* Counters for averaging values */
  double dvalue;               /* Pixel value */
  long counter[MI2_MAX_VAR_DIMS];  /* Dimension loop counter */
  void *ptr, *iptr, *optr;     /* Pointers for looping through fastest dim */
  void *ivecptr[MI2_MAX_VAR_DIMS]; /* Pointers to start of each dimension */
  void *ovecptr[MI2_MAX_VAR_DIMS];
  long *end;                   /* Pointer to array of dimension ends */
  int fastdim;                 /* Dimension that varies fastest */
  long ipix;                   /* Buffer subscript */
  int idim;                    /* Dimension subscript */
  int notmodified;             /* First dimension not reset */
  int out_of_range;            /* Flag indicating one pixel of sum out of
                                   range */
  double dmin, dmax, epsilon;  /* Range limits */

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_dimconvert" );

  /* Initialize variables */
  dcp = &dim_conv_struct;
  {
    MI2_CHK_ERR ( MI2_icv_dimconv_init ( operation, icvp, dcp, start, count, values,
                                         bufstart, bufcount, buffer ) )
  }

  /* Initialize local variables */
  iptr    = dcp->istart;
  optr    = dcp->ostart;
  end     = dcp->end;
  fastdim = icvp->derv_dimconv_fastdim;
  dmax = icvp->fill_valid_max;
  dmin = icvp->fill_valid_min;
  epsilon = ( dmax - dmin ) * MI2_FILLVALUE_EPSILON;
  epsilon = fabs ( epsilon );
  dmax += epsilon;
  dmin -= epsilon;

  /* Initialize counters */
  for ( idim = 0; idim <= fastdim; idim++ ) {
    counter[idim] = 0;
    ivecptr[idim] = iptr;
    ovecptr[idim] = optr;
  }

  /* Loop through data */

  while ( counter[0] < end[0] ) {

    /* Compress data by averaging if needed */
    if ( !dcp->do_compress ) {
      {
        MI2_TO_DOUBLE ( dvalue, dcp->intype, dcp->insign, iptr )
      }
      out_of_range = ( icvp->do_fillvalue &&
                       ( ( dvalue < dmin ) || ( dvalue > dmax ) ) );
    } else {
      sum1 = 0.0;
      sum0 = 0.0;
      out_of_range = FALSE;

      for ( ipix = 0; ipix < dcp->in_pix_num; ipix++ ) {
        ptr = ( void * ) ( ( char * ) iptr + dcp->in_pix_off[ipix] );

        /* Check if we are outside the buffer.
           If we are looking before the buffer, then we need to
           add in the previous result to do averaging properly. If
           we are looking after the buffer, then break. */
        if ( ptr < dcp->in_pix_first ) {
          /* Get the output value and re-scale it */
          {
            MI2_TO_DOUBLE ( dvalue, dcp->outtype, dcp->outsign, optr )
          }

          if ( icvp->do_scale ) {
            dvalue = ( ( icvp->scale == 0.0 ) ?
                       0.0 : ( dvalue - icvp->offset ) / icvp->scale );
          }
        } else if ( ptr > dcp->in_pix_last ) {
          continue;
        } else {
          {
            MI2_TO_DOUBLE ( dvalue, dcp->intype, dcp->insign, ptr )
          }
        }

        /* Add in the value, checking for range if needed */
        if ( icvp->do_fillvalue && ( ( dvalue < dmin ) || ( dvalue > dmax ) ) ) {
          out_of_range = TRUE;
        } else {
          sum1 += dvalue;
          sum0++;
        }
      }         /* Foreach pixel to compress */

      /* Average values */
      if ( sum0 != 0.0 )
        dvalue = sum1 / sum0;
      else
        dvalue = 0.0;
    }           /* If compress */

    /* Check for out of range values and scale result */
    if ( out_of_range ) {
      dvalue = icvp->user_fillvalue;
    } else if ( icvp->do_scale ) {
      dvalue = icvp->scale * dvalue + icvp->offset;
    }

    /* Expand data if needed */
    if ( !dcp->do_expand ) {
      {
        MI2_FROM_DOUBLE ( dvalue, dcp->outtype, dcp->outsign, optr )
      }
    } else {
      for ( ipix = 0; ipix < dcp->out_pix_num; ipix++ ) {
        ptr = ( void * ) ( ( char * ) optr + dcp->out_pix_off[ipix] );

        /* Check if we are outside the buffer. */
        if ( ( ptr >= dcp->out_pix_first ) && ( ptr <= dcp->out_pix_last ) ) {
          {
            MI2_FROM_DOUBLE ( dvalue, dcp->outtype, dcp->outsign, ptr )
          }
        }

      }         /* Foreach pixel to expand */
    }         /* if expand */

    /* Increment the counter and the pointers */
    if ( ( ++counter[fastdim] ) < end[fastdim] ) {
      optr = ( void * ) ( ( char * ) optr + dcp->ostep[fastdim] );
      iptr = ( void * ) ( ( char * ) iptr + dcp->istep[fastdim] );
    } else {
      /* If we reach the end of fastdim, then reset the counter and
         increment the next dimension down - keep going as needed.
         The vectors ovecptr and ivecptr give the starting values of optr
         and iptr for that dimension. */
      idim = fastdim;

      while ( ( idim > 0 ) && ( counter[idim] >= end[idim] ) ) {
        counter[idim] = 0;
        idim--;
        counter[idim]++;
        ovecptr[idim] = ( void * ) ( ( char * ) ovecptr[idim] + dcp->ostep[idim] );
        ivecptr[idim] = ( void * ) ( ( char * ) ivecptr[idim] + dcp->istep[idim] );
      }

      notmodified = idim;

      /* Copy the starting index up the vector */
      for ( idim = notmodified + 1; idim <= fastdim; idim++ ) {
        ovecptr[idim] = ovecptr[notmodified];
        ivecptr[idim] = ivecptr[notmodified];
      }

      optr = ovecptr[fastdim];
      iptr = ivecptr[fastdim];
    }      /* if at end of row */

  }      /* while more pixels to process */


  MI2_RETURN ( MI_NOERROR );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_icv_dimconv_init
@INPUT      : operation  - MI_PRIV_GET or MI_PRIV_PUT
              icvp       - icv structure pointer
              dcp        - dimconvert structure pointer
              start      - start passed by user
              count      - count passed by user
              values     - pointer to user's data area (for put)
              bufstart   - start of variable buffer
              bufcount   - count of variable buffer
              buffer     - pointer to variable buffer (for get)
@OUTPUT     :
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Sets up stuff for MI_icv_dimconvert.
@METHOD     :
@GLOBALS    :
@CALLS      : NetCDF routines
@CREATED    : September 4, 1992 (Peter Neelin)
@MODIFIED   :
---------------------------------------------------------------------------- */
static int MI2_icv_dimconv_init ( int operation, mi2_icv_type *icvp,
                                  mi2_icv_dimconv_type *dcp,
                                  long start[], long count[], void *values,
                                  long bufstart[], long bufcount[], void *buffer )
/* ARGSUSED */
{
  long buffer_len, values_len; /* Buffer lengths, offsets and indices */
  long buffer_off, values_off;
  long buffer_index, values_index;
  int imgdim_high, imgdim_low; /* Range of subscripts of image dimensions */
  int scale, offset;            /* Dimension scale and offset */
  int idim, jdim;
  int fastdim;
  /* Variables for calculating pixel offsets for compress/expand */
  long var_dcount[MI_MAX_IMGDIMS + 1], var_dend[MI_MAX_IMGDIMS + 1];
  long usr_dcount[MI_MAX_IMGDIMS + 1], usr_dend[MI_MAX_IMGDIMS + 1];
  long pixcount;
  int var_fd, usr_fd, dshift;
  long ipix;

  MI2_SAVE_ROUTINE_NAME ( "MI2_icv_dimconv_init" );

  /* Check to see if any compression or expansion needs to be done.
     Work it out for a GET and then swap if a PUT. */
  if ( operation == MI2_PRIV_GET ) {
    dcp->do_compress = icvp->derv_var_compress;
    dcp->do_expand   = icvp->derv_usr_compress;
  } else {
    dcp->do_expand   = icvp->derv_var_compress;
    dcp->do_compress = icvp->derv_usr_compress;
  }

  fastdim = icvp->derv_dimconv_fastdim;

  /* Get the indices of high and low image dimensions */
  imgdim_high = icvp->var_ndims - 1;

  if ( icvp->var_is_vector )
    imgdim_high--;

  imgdim_low = imgdim_high - icvp->user_num_imgdims + 1;

  /* Get the buffer sizes */
  buffer_len = icvp->var_typelen;
  values_len = icvp->user_typelen;

  for ( idim = 0; idim < icvp->var_ndims; idim++ ) {
    buffer_len *= bufcount[idim];

    if ( idim <= fastdim )
      values_len *= icvp->derv_icv_count[idim];
  }

  /* Calculate step size for variable and user buffers. This does not
     allow for growing or shrinking pixels. That correction is done below. */
  if ( icvp->var_is_vector && icvp->user_do_scalar ) {
    dcp->buf_step[fastdim + 1] = icvp->var_typelen;
    dcp->buf_step[fastdim] = dcp->buf_step[fastdim + 1] * bufcount[fastdim + 1];
  } else {
    dcp->buf_step[fastdim] = icvp->var_typelen;
  }

  dcp->usr_step[fastdim] = icvp->user_typelen;

  for ( idim = fastdim - 1; idim >= 0; idim-- ) {
    dcp->buf_step[idim] = dcp->buf_step[idim + 1] * bufcount[idim + 1];
    dcp->usr_step[idim] = dcp->usr_step[idim + 1]
                          * icvp->derv_icv_count[idim + 1];
  }

  /* Set sign of user steps for flipping, if needed */
  for ( idim = imgdim_low; idim <= imgdim_high; idim++ ) {
    if ( icvp->derv_dim_flip[imgdim_high - idim] )
      dcp->usr_step[idim] *= ( -1 );
  }

  /* Get the pointers to the start of buffers and the number of pixels
     in each dimension (count a pixel as one expansion/compression -
     one time through the loop below) */
  buffer_off = 0;
  values_off = 0;

  for ( idim = 0; idim <= fastdim; idim++ ) {
    if ( ( idim < imgdim_low ) || ( idim > imgdim_high ) ) {
      dcp->end[idim] = bufcount[idim];
      buffer_index = 0;
      values_index = bufstart[idim] - icvp->derv_icv_start[idim];
    } else {
      jdim = imgdim_high - idim;
      scale = icvp->derv_dim_scale[jdim];
      offset = icvp->derv_dim_off[jdim];

      if ( icvp->derv_dim_grow[jdim] ) {
        dcp->end[idim] = bufcount[idim];
        buffer_index = 0;

        if ( !icvp->derv_dim_flip[jdim] )
          values_index = bufstart[idim] * scale
                         - icvp->derv_icv_start[idim] + offset;
        else
          values_index =
            ( icvp->var_dim_size[jdim] - bufstart[idim] ) * scale
            - 1 - icvp->derv_icv_start[idim] + offset;
      } else {
        dcp->end[idim] = ( bufcount[idim] - 1 + bufstart[idim] % scale )
                         / scale + 1;
        buffer_index = - ( bufstart[idim] % scale );

        if ( !icvp->derv_dim_flip[jdim] )
          values_index = bufstart[idim] / scale
                         - icvp->derv_icv_start[idim] + offset;
        else
          values_index =
            ( icvp->var_dim_size[jdim] - bufstart[idim] - 1 ) / scale
            - icvp->derv_icv_start[idim] + offset;
      }
    }

    buffer_off += buffer_index * labs ( dcp->buf_step[idim] );
    values_off += values_index * labs ( dcp->usr_step[idim] );
  }

  /* Calculate arrays of offsets for compress/expand. */
  if ( dcp->do_compress || dcp->do_expand ) {
    /* Initialize counters */
    var_fd = icvp->user_num_imgdims - 1;
    usr_fd = icvp->user_num_imgdims - 1;

    if ( icvp->var_is_vector && icvp->user_do_scalar ) {
      var_fd++;
      var_dcount[var_fd] = 0;
      var_dend[var_fd] = icvp->var_vector_size;
    }

    for ( jdim = 0; jdim < icvp->user_num_imgdims; jdim++ ) {
      idim = icvp->user_num_imgdims - jdim - 1;
      var_dcount[idim] = 0;
      usr_dcount[idim] = 0;
      var_dend[idim] = ( icvp->derv_dim_grow[jdim] ?
                         1 : MIN ( icvp->var_dim_size[jdim],
                                   icvp->derv_dim_scale[jdim] ) );
      usr_dend[idim] = ( icvp->derv_dim_grow[jdim] ?
                         icvp->derv_dim_scale[jdim] : 1 );
    }

    /* Loop through variable buffer pixels */
    pixcount = 0;
    dshift = imgdim_low;

    for ( ipix = 0; ipix < icvp->derv_var_pix_num; ipix++ ) {
      icvp->derv_var_pix_off[ipix] = pixcount;
      pixcount += dcp->buf_step[var_fd + dshift];

      if ( ( ++var_dcount[var_fd] ) >= var_dend[var_fd] ) {
        idim = var_fd;

        while ( ( idim > 0 ) && ( var_dcount[idim] >= var_dend[idim] ) ) {
          var_dcount[idim] = 0;
          idim--;
          var_dcount[idim]++;
        }

        for ( idim = 0, pixcount = 0; idim <= var_fd; idim++ ) {
          pixcount += var_dcount[idim] * dcp->buf_step[idim + dshift];
        }
      }
    }

    /* Loop through user buffer pixels */
    pixcount = 0;
    dshift = imgdim_low;

    for ( ipix = 0; ipix < icvp->derv_usr_pix_num; ipix++ ) {
      icvp->derv_usr_pix_off[ipix] = pixcount;
      pixcount += dcp->usr_step[usr_fd + dshift];

      if ( ( ++usr_dcount[usr_fd] ) >= usr_dend[usr_fd] ) {
        idim = usr_fd;

        while ( ( idim > 0 ) && ( usr_dcount[idim] >= usr_dend[idim] ) ) {
          usr_dcount[idim] = 0;
          idim--;
          usr_dcount[idim]++;
        }

        for ( idim = 0, pixcount = 0; idim <= var_fd; idim++ ) {
          pixcount += usr_dcount[idim] * dcp->usr_step[idim + dshift];
        }
      }
    }

    /* Correct buffer steps for compress/expand */
    for ( idim = imgdim_low; idim <= imgdim_high; idim++ ) {
      jdim = imgdim_high - idim;

      if ( icvp->derv_dim_grow[jdim] )
        dcp->usr_step[idim] *= icvp->derv_dim_scale[jdim];
      else
        dcp->buf_step[idim] *= icvp->derv_dim_scale[jdim];
    }

  }           /* if compress/expand */

  /* Set input and output variables */
  if ( operation == MI2_PRIV_GET ) {      /* For a GET */
    dcp->in_pix_num = icvp->derv_var_pix_num;
    dcp->in_pix_off = icvp->derv_var_pix_off;
    dcp->in_pix_first = buffer;
    dcp->in_pix_last = ( void * ) ( ( char * ) buffer + buffer_len - 1 );
    dcp->out_pix_num = icvp->derv_usr_pix_num;
    dcp->out_pix_off = icvp->derv_usr_pix_off;
    dcp->out_pix_first = values;
    dcp->out_pix_last = ( void * ) ( ( char * ) values + values_len - 1 );
    dcp->intype = icvp->var_type;
    dcp->insign = icvp->var_sign;
    dcp->outtype = icvp->user_type;
    dcp->outsign = icvp->user_sign;
    dcp->istep = dcp->buf_step;
    dcp->ostep = dcp->usr_step;
    dcp->istart = ( void * ) ( ( char * ) buffer + buffer_off );
    dcp->ostart = ( void * ) ( ( char * ) values + values_off );
  }                   /* if GET */
  else {                                 /* For a PUT */
    dcp->out_pix_num = icvp->derv_var_pix_num;
    dcp->out_pix_off = icvp->derv_var_pix_off;
    dcp->out_pix_first = buffer;
    dcp->out_pix_last = ( void * ) ( ( char * ) buffer + buffer_len - 1 );
    dcp->in_pix_num = icvp->derv_usr_pix_num;
    dcp->in_pix_off = icvp->derv_usr_pix_off;
    dcp->in_pix_first = values;
    dcp->in_pix_last = ( void * ) ( ( char * ) values + values_len - 1 );
    dcp->outtype = icvp->var_type;
    dcp->outsign = icvp->var_sign;
    dcp->intype = icvp->user_type;
    dcp->insign = icvp->user_sign;
    dcp->ostep = dcp->buf_step;
    dcp->istep = dcp->usr_step;
    dcp->ostart = ( void * ) ( ( char * ) buffer + buffer_off );
    dcp->istart = ( void * ) ( ( char * ) values + values_off );
  }                   /* if PUT */

  MI2_RETURN ( MI_NOERROR );
}
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 



/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI2_varaccess
@INPUT      : operation - either MI_PRIV_GET or MI_PRIV_PUT, indicating
                 whether the routine should get or put data from/to a
                 cdf file
              volume    - volume
              varid     - variable id
              start     - vector of coordinates of corner of hyperslab
              count     - vector of edge lengths of hyperslab
              datatype  - type that calling routine wants (one of the valid
                 netcdf data types, excluding NC_CHAR)
              sign      - sign that the calling routine wants (one of
                 MI_PRIV_SIGNED, MI_PRIV_UNSIGNED, MI_PRIV_DEFAULT).
              bufsize_step - vector of buffer size steps wanted by 
                 caller (MI_var_loop will try, but no guarantees); if
                 NULL, then 1 is assumed. For the first index that cannot be 
                 read in one piece, the allocated buffer will tend to have 
                 the count of as a multiple of the corresponding value in 
                 this vector.
              icvp      - pointer to icv structure (image conversion variable)
                 If NULL, then icvp->do_scale and icvp->do_dimconvert are
                 assumed to be FALSE.
                 icvp->do_scale        - boolean indicating whether scaling
                    should be done. If so, then 
                       outvalue = icvp->scale * (double) invalue + icvp->offset
                 icvp->scale           - (see do_scale)
                 icvp->offset          - (see do_scale)
                 icvp->do_dimconvert   - boolean indicating whether the
                    dimension conversion routine should be called
                 icvp->dimconvert_func - dimension conversion routine
              values    - values to store in variable (for put)
@OUTPUT     : values    - values to get from variable (for get)
@RETURNS    : MI_ERROR (=-1) when an error occurs
@DESCRIPTION: Routine to do work for getting/putting and converting 
              the type of variable values. Similar to routine ncvarget/
              ncvarput but the calling routine specifies the form in 
              which data should be returned/passed (datatype), as well as 
              the sign. The datatype can only be a numeric type. If the 
              variable in the file is of type NC_CHAR, then an error is 
              returned. Values can optionally be scaled (for image
              conversion routines) by setting icvp->do_scale to TRUE and 
              using icvp->scale and icvp->offset. Dimensional conversion
              can be done be setting icvp->do_dimconvert to TRUE and
              passing a function to be called (icvp->dimconvert_func).
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF and MINC routines
@CREATED    : July 29, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
int MI2_varaccess(int operation, mihandle_t volume, 
                             long start[], long count[],
                             nc_type datatype, int sign, void *values,
                             int *bufsize_step, mi_icv_type *icvp)
{
   mi2_varaccess_type strc;    /* Structure of values for functions */
   int ndims;                 /* Number of variable dimensions */
   char stringa[MI_MAX_ATTSTR_LEN];  /* String for attribute value */
   char *string = stringa;
   int oldncopts;             /* Save old value of ncopts */

   MI2_SAVE_ROUTINE_NAME("MI2_varaccess");

   /* Check to see if ivc structure was passed and set variables
      needed by this routine */
   if (icvp == NULL) {
      strc.do_scale      = FALSE;
      strc.do_dimconvert = FALSE;
      strc.do_fillvalue  = FALSE;
   }
   else {
      strc.do_scale      = icvp->do_scale;
      strc.do_dimconvert = icvp->do_dimconvert;
      strc.do_fillvalue  = icvp->do_fillvalue;
   }

   /* Inquire about the variable */
   /*TODO:Convert*/
   MI2_CHK_ERR(ncvarinq(cdfid, varid, NULL, &(strc.var_type), 
                       &ndims, NULL, NULL))

   /* Check that the variable type is numeric */
   if ((datatype==NC_CHAR) || (strc.var_type==NC_CHAR)) {
      milog_message(MI_MSG_VARNOTNUM);
      MI2_RETURN(MI_ERROR);
   }

   /* Try to find out the sign of the variable using MIsigntype.
      To avoid programs dying unexpectedly, we must change ncopts,
      then restore it */
   oldncopts = ncopts;
   ncopts = 0;
   string=miattgetstr(cdfid, varid, MIsigntype, MI_MAX_ATTSTR_LEN, string);
   ncopts = oldncopts;

   /* Get the signs */
   strc.var_sign  = MI2_get_sign_from_string(strc.var_type, string);
   strc.call_sign = MI2_get_sign(datatype, sign);

   /* Check to see if the type requested is the same as the variable type,
      the signs are the same and no dimension conversion is needed. If so, 
      just get/put the values */
   if ((datatype == strc.var_type) && (strc.call_sign == strc.var_sign) && 
                !strc.do_scale && !strc.do_dimconvert && !strc.do_fillvalue) {
      switch (operation) {
      case MI2_PRIV_GET:
         MI2_CHK_ERR(ncvarget(cdfid, varid, start, count, values))
         break;
      case MI2_PRIV_PUT:
         MI2_CHK_ERR(ncvarput(cdfid, varid, start, count, values))
         break;
      default:
         milog_message(MI_MSG_BADOP);
         MI2_RETURN(MI_ERROR);
      }
      MI2_RETURN(MI_NOERROR);
   }

   /* Otherwise, we have to loop through data. Set up structure
      and call MI_var_loop */
   strc.operation=operation;
   strc.cdfid=cdfid;
   strc.varid=varid;
   strc.call_type=datatype;
   strc.var_value_size=mitype_len(strc.var_type);
   strc.call_value_size=mitype_len(strc.call_type);
   strc.icvp=icvp;
   strc.start=start;
   strc.count=count;
   strc.values=values;
   MI_CHK_ERR( MI2_var_loop(ndims, start, count, 
                           strc.var_value_size, bufsize_step,
                           MI2_MAX_VAR_BUFFER_SIZE, 
                           (void *) &strc, MI2_var_action) )
   MI2_RETURN(MI_NOERROR);
   
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_convert_type
@INPUT      : number_of_values  - number of values to copy
              intype            - type of input values
              insign            - sign of input values (one of
                 MI_PRIV_DEFSIGN, MI_PRIV_SIGNED or MI_PRIV_UNSIGNED)
              invalues          - vector of values
              outtype           - type of output values
              outsign           - sign of output values
              icvp              - pointer to icv structure (if NULL,
                 then icvp->do_scale is assumed to be FALSE)
                 icvp->do_scale - boolean indicating whether scaling
                    should be done. If so, then 
                       outvalue = icvp->scale * (double) invalue + icvp->offset
                 icvp->scale    - (see do_scale)
                 icvp->offset   - (see do_scale)
@OUTPUT     : outvalues         - output values
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Converts the invalues to outvalues according to their type.
              Types must be numeric. Values out of range are truncated
              to the nearest value in range. The sign of integer values
              is given by insign and outsign, which must have values
              MI_PRIV_DEFSIGN, MI_PRIV_SIGNED or MI_PRIV_UNSIGNED. 
              If it is MI_PRIV_DEFSIGN then the default signs are
              used (from MI_get_sign) :
                 byte  : unsigned
                 short : signed
                 int   : signed
              Note that if a conversion must take place, then all input 
              values are converted to double. Values can be scaled through
              icvp->scale and icvp->offset by setting icvp->do_scale to TRUE.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : July 27, 1992 (Peter Neelin)
@MODIFIED   : August 28, 1992 (P.N.)
                 - replaced type conversions with macros
---------------------------------------------------------------------------- */
int MI2_convert_type(long number_of_values,
                      mitype_t intype,  int insign,  void *invalues,
                      mitype_t outtype, int outsign, void *outvalues,
                      mi2_icv_type *icvp)
{
   int inincr, outincr;    /* Pointer increments for arrays */
   int insgn, outsgn;      /* Signs for input and output */
   long i;
   double dvalue=0.0;      /* Temporary double for conversion */
   void *inptr, *outptr;   /* Pointers to input and output values */
   int do_scale;           /* Should scaling be done? */
   int do_fillvalue;       /* Should fillvalue checking be done? */
   double fillvalue;       /* Value to fill with */
   double dmax, dmin;      /* Range of legal values */
   double epsilon;         /* Epsilon for legal values comparisons */

   MI2_SAVE_ROUTINE_NAME("MI_convert_type");

   /* Check to see if icv structure was passed and set variables needed */
   if (icvp == NULL) {
      do_scale=FALSE;
      do_fillvalue = FALSE;
      dmax = dmin = 0.0;
      fillvalue = 0.0;
   }
   else {
      do_scale=icvp->do_scale;
      do_fillvalue=icvp->do_fillvalue;
      fillvalue = icvp->user_fillvalue;
      dmax = icvp->fill_valid_max;
      dmin = icvp->fill_valid_min;
      epsilon = (dmax - dmin) * MI2_FILLVALUE_EPSILON;
      epsilon = fabs(epsilon);
      dmax += epsilon;
      dmin -= epsilon;
   }

   /* Check the types and get their size */
   if ((intype==MI_TYPE_BYTE) || (outtype==MI_TYPE_BYTE)) {
      milog_message(MI2_MSG_VARNOTNUM);
      MI2_RETURN(MI_ERROR);
   }
   if (((inincr =mitype_len(intype ))==MI_ERROR) ||
       ((outincr=mitype_len(outtype))==MI_ERROR)) {
      MI_RETURN(MI_ERROR);
   }

   /* Get the sign of input and output values */
   insgn  = mitype_sign(intype,  insign);
   outsgn = mitype_sign(outtype, outsign);

   /* Check to see if a conversion needs to be made.
      If not, just copy the memory */
   if ((intype==outtype) && (insgn==outsgn) && !do_scale && !do_fillvalue) {
         (void) memcpy(outvalues, invalues, 
                       (size_t) number_of_values*inincr);
   }
   
   /* Otherwise, loop through */
   else {

      /* Step through values  */
      inptr=invalues; 
      outptr=outvalues;
      for (i=0 ; i<number_of_values; i++) { 

         /* Convert the input value */
         {MI2_TO_DOUBLE(dvalue, intype, insgn, inptr)}

         /* Check the value for range and scale the value if necessary */
         if (do_fillvalue && ((dvalue < dmin) || (dvalue > dmax))) {
            dvalue = fillvalue;
         }
         else if (do_scale) {
            dvalue = icvp->scale * dvalue + icvp->offset;
         }

         /* Truncate if necessary and assign the value */
         {MI2_FROM_DOUBLE(dvalue, outtype, outsgn, outptr)}

         inptr  = (void *) ((char *)inptr  + inincr);
         outptr = (void *) ((char *)outptr + outincr);

      }           /* End of for loop */

   }              /* End of else */

   MI2_RETURN(MI_NOERROR);
   
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_var_action
@INPUT      : ndims       - number of dimensions
              var_start   - coordinate vector of corner of hyperslab
              var_count   - vector of edge lengths of hyperslab
              nvalues     - number of values in hyperslab
              var_buffer  - pointer to variable buffer
              caller_data - pointer to data from MI_varaccess
@OUTPUT     : (none)
@RETURNS    : MI_ERROR if an error occurs
@DESCRIPTION: Buffer action routine to be called by MI_var_loop, for
              use by MI_varaccess.
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF and MINC routines
@CREATED    : July 30, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
int MI2_var_action(int ndims, long var_start[], long var_count[], 
                          long nvalues, void *var_buffer, void *caller_data)
     /* ARGSUSED */
{
   mi2_varaccess_type *ptr;   /* Pointer to data from MI_varaccess */
   int status;               /* Status returned by function call */

   MI2_SAVE_ROUTINE_NAME("MI2_var_action");

   ptr=(mi2_varaccess_type *) caller_data;

   /* Get/put values and do conversions, etc. */
   switch (ptr->operation) {
   case MI2_PRIV_GET:
      status=ncvarget(ptr->cdfid, ptr->varid, var_start, var_count, 
                      var_buffer);
      if (status != MI_ERROR) {
         /* If doing dimension conversion, let dimconvert function do all the 
            work, including type conversion */
         if (!ptr->do_dimconvert) {
            status=MI2_convert_type(nvalues,
                      ptr->var_type, ptr->var_sign, var_buffer,
                      ptr->call_type, ptr->call_sign, ptr->values,
                      ptr->icvp);
         }
         else {
            status=(*(ptr->icvp->dimconvert_func))(ptr->operation, ptr->icvp, 
                         ptr->start, ptr->count, ptr->values,
                         var_start, var_count, var_buffer);
         }
      }
      break;
   case MI2_PRIV_PUT:
      /* If doing dimension conversion, let dimconvert function do all the 
         work, including type conversion */
      if (!ptr->do_dimconvert) {
         status=MI_convert_type(nvalues,
                   ptr->call_type, ptr->call_sign, ptr->values,
                   ptr->var_type, ptr->var_sign, var_buffer,
                   ptr->icvp);
      }
      else {
         status=(*(ptr->icvp->dimconvert_func))(ptr->operation, ptr->icvp, 
                      ptr->start, ptr->count, ptr->values,
                      var_start, var_count, var_buffer);
      }
      if (status != MI_ERROR) {
         status=ncvarput(ptr->cdfid, ptr->varid, var_start, var_count, 
                         var_buffer);
      }
      break;
   default:
      milog_message(MI2_MSG_BADOP);
      status=MI2_ERROR;
   }

   /* Check for an error */
   MI2_CHK_ERR(status)

   /* Increment the values pointer */
   if (!ptr->do_dimconvert) {
      ptr->values = (void *) ((char *) ptr->values + 
                                   nvalues*ptr->call_value_size);
   }

   MI_RETURN(MI_NOERROR);

}



/* ----------------------------- MNI Header -----------------------------------
@NAME       : MI_var_loop
@INPUT      : ndims       - number of dimensions in variable
              start       - vector of coordinates of corner of hyperslab
              count       - vector of edge lengths of hyperslab
              value_size  - size (in bytes) of each value to be buffered
              bufsize_step - vector of buffer size steps wanted by 
                 caller (MI_var_loop will try, but no guarantees); if
                 NULL, then 1 is assumed. For the first index that cannot be 
                 read in one piece, the allocated buffer will tend to have 
                 the count of as a multiple of the corresponding value in 
                 this vector.
              max_buffer_size - maximum size (in bytes) of buffer
              caller_data - pointer to a structure of data to pass to
                 functions
              action_func - function to do something with each buffer
@OUTPUT     : (none)
@RETURNS    : MI_ERROR (=-1) when an error occurs
@DESCRIPTION: Routine to loop through a variable's indices, getting data
              into a buffer and doing something to it. A function pointer
              is passed that will perform these functions on each buffer.
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF and MINC routines
@CREATED    : July 29, 1992 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
int MI2_var_loop(int ndims, long start[], long count[],
                            int value_size, int *bufsize_step,
                            long max_buffer_size,
                            void *caller_data,
                            int (*action_func) (int, long [], long [], 
                                                long, void *, void *))
{
   long nvalues, newnvalues;  /* Number of values in fastest varying dims.
                                 Note that any dimensional subscript variables
                                 should be long */
   int firstdim;              /* First dimension that doesn't fit in buffer */
   long ntimes;               /* Number of firstdim elements that fit in buf */
   void *var_buffer;          /* Pointer to buffer for variable data */
   long var_count[MAX_VAR_DIMS];   /* Count, start and end coordinate */
   long var_start[MAX_VAR_DIMS];   /* vectors for getting buffers */
   long var_end[MAX_VAR_DIMS];
   int i;                     /* Looping variable - only used for dimension
                                 number, not dimension subscript */

   MI_SAVE_ROUTINE_NAME("MI_var_loop");

   /* Find out how much space we need and then allocate a buffer.
      To do this we find out how many dimensions will fit in our
      maximum buffer size. firstdim is the index of the first dimension
      that won't fit. nvalues is the number of values in the first dimensions
      that do fit in the buffer. ntimes is the number of times that the first
      dimensions fit in the buffer. To make things simpler, dimension 0 is
      always considered to not fit, even if it does. */
   nvalues=newnvalues=1;
   for (firstdim=ndims-1; firstdim>=1; firstdim--) {
      newnvalues *= count[firstdim];
      if (newnvalues*value_size > max_buffer_size) break;
      nvalues = newnvalues;
   }
   if (firstdim<0) {               /* Check for 0-dim variable */
      firstdim=0;
      ntimes=1;
   }
   else {
      ntimes = MIN(MI_MAX_VAR_BUFFER_SIZE/(nvalues*value_size),
                   count[firstdim]);
      /* Try to make ntimes an convenient multiple for the caller */
      if ((ntimes != count[firstdim]) && (bufsize_step != NULL)) {
         ntimes = MAX(1, ntimes - (ntimes % bufsize_step[firstdim]));
      }
   }

   /* Allocate space for variable values */
   if ((var_buffer = MALLOC(ntimes*nvalues*value_size, char)) 
                                     == NULL) {
      milog_message(MI_MSG_OUTOFMEM);
      MI_RETURN(MI_ERROR);
   }

   /* Create a count variable for the var buffer, with 1s for dimensions
      that vary slower than firstdim and count[i] for dimensions that
      vary faster. Set a start variable for the var buffer, equal to start.
      Set an end variable for the var buffer. */
   if (ndims <= 0) {             /* Handle zero-dimension variable */
      var_start[0]=0; var_end[0]=1; var_count[0]=1;
   }
   for (i=0; i<ndims; i++) {
      var_count[i] = (i>firstdim)  ? count[i] : 
                     (i==firstdim) ? ntimes : 1;
      var_start[i] = start[i];
      var_end[i] = start[i] + count[i];
   }
      
   /* Loop through the dimensions, copying buffers, etc. 
      Exit when the slowest varying dimension reaches its limit. */

   while (var_start[0] < var_end[0]) {
      var_count[firstdim] = 
         MIN(ntimes, var_end[firstdim] - var_start[firstdim]);
      
      /* Do the stuff on the buffer */
      if ((*action_func)(ndims, var_start, var_count, 
                         var_count[firstdim]*nvalues, var_buffer,
                         caller_data) == MI_ERROR) {
         FREE(var_buffer);
         MI_RETURN_ERROR(MI_ERROR);
      }

      /* Increment the start counters */
      var_start[firstdim] += var_count[firstdim];
      i=firstdim;
      while ( (i>0) && (var_start[i] >= var_end[i])) {
         var_start[i] = start[i];
         i--;
         var_start[i]++;
      }
      
   }

   /* Free the buffer and return */
   FREE(var_buffer);
   MI_RETURN(MI_NOERROR);
   
}