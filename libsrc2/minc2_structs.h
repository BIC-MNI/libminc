/** internal minc2 data structures*/
#ifndef __MINC2_STRUCTS_H__
#define __MINC2_STRUCTS_H__ 1 

/************************************************************************
 * ENUMS, STRUCTS, and TYPEDEFS
 ************************************************************************/
/* These structure declarations exist to allow the following typedefs to
 * work.  Since the details of these structures are not meant to be public,
 * the actual structure definitions are in minc2_private.h
 */
struct mivolprops;
struct midimension;
struct mivolume;

/** \typedef mivolumeprops_t 
 * Opaque pointer to volume properties.
 */
typedef struct mivolprops *mivolumeprops_t;


/** \typedef midimhandle_t
 * Opaque pointer to a MINC dimension object.
 */
typedef struct midimension *midimhandle_t;


/** \typedef mihandle_t 
 * The mihandle_t is an opaque type that represents a MINC file object.
 */
typedef struct mivolume *mihandle_t;


typedef void *milisthandle_t;

/**
 * This typedef used to represent the type of an individual voxel <b>as
 * stored</b> by MINC 2.0.  If a volume is 
 */
typedef enum {
  MI_TYPE_BYTE = 1,         /**< 8-bit signed integer */
  MI_TYPE_SHORT = 3,        /**< 16-bit signed integer */
  MI_TYPE_INT = 4,          /**< 32-bit signed integer */
  MI_TYPE_FLOAT = 5,        /**< 32-bit floating point */
  MI_TYPE_DOUBLE = 6,       /**< 64-bit floating point */
  MI_TYPE_STRING = 7,       /**< ASCII string */
  MI_TYPE_UBYTE = 100,      /**< 8-bit unsigned integer */
  MI_TYPE_USHORT = 101,     /**< 16-bit unsigned integer */
  MI_TYPE_UINT = 102,       /**< 32-bit unsigned integer */
  MI_TYPE_SCOMPLEX = 1000,  /**< 16-bit signed integer complex */
  MI_TYPE_ICOMPLEX = 1001,  /**< 32-bit signed integer complex */
  MI_TYPE_FCOMPLEX = 1002,  /**< 32-bit floating point complex */
  MI_TYPE_DCOMPLEX = 1003,  /**< 64-bit floating point complex */
  MI_TYPE_UNKNOWN  = -1     /**< when the type is a record */
} mitype_t;

/** 
 * This typedef is used to represent the class of the MINC file.  
 *
 * The class specifies the data's interpretation rather than its 
 * storage format. For example, a floating point class implies
 * that the data may be stored as integers but must nonetheless be 
 * scaled into a "real" range before any mathematical operations
 * are performed.  A label class implies that the values of a voxel
 * should be considered to represent a symbol, and therefore many 
 * operations on the voxels would be considered meaningless.
 */
typedef enum {
  MI_CLASS_REAL = 0,            /**< Floating point (default) */
  MI_CLASS_INT = 1,             /**< Integer */
  MI_CLASS_LABEL = 2,           /**< Enumerated (named data values) */
  MI_CLASS_COMPLEX = 3,         /**< Complex (real/imaginary) values */
  MI_CLASS_UNIFORM_RECORD = 4,  /**< Aggregate datatypes consisting of multiple values of the same underlying type. */
  MI_CLASS_NON_UNIFORM_RECORD = 5 /**< Aggregate datatypes consisting of multiple values of potentially differing types (not yet implemented). */
} miclass_t;

/** Dimensions be members of one of several classes.  The "MI_DIMCLASS_ANY"
 * value is never actually assigned to a dimension.  It is used in the 
 * programming interface to specify that an operation should apply to
 * all dimensions regardless of class.
 */
typedef enum {
  MI_DIMCLASS_ANY = 0,        /**< Don't care (or unknown) */
  MI_DIMCLASS_SPATIAL = 1,    /**< Spatial dimensions (x, y, z) */
  MI_DIMCLASS_TIME = 2,       /**< Time dimension */
  MI_DIMCLASS_SFREQUENCY = 3, /**< Spatial frequency dimensions */
  MI_DIMCLASS_TFREQUENCY = 4, /**< Temporal frequency dimensions */
  MI_DIMCLASS_USER = 5,       /**< Arbitrary user-defined dimension */
  MI_DIMCLASS_RECORD = 6      /**< Record as dimension */
} midimclass_t;

/** Dimension order refers to the idea that data can be structured in 
 * a variety of ways with respect to the dimensions.  For example, a typical
 * 3D scan could be structured as a transverse (ZYX) or sagittal (XZY) image.
 * Since it may be convenient to write code which expects a particular 
 * dimension order, a user can specify an alternative ordering by using 
 * miset_apparent_dimension_order().  This will cause most functions
 * to return data as if the file was in the apparent, rather than the
 * file (native) order.
 */
typedef enum {
  MI_DIMORDER_FILE      = 0,
  MI_DIMORDER_APPARENT  = 1
} miorder_t;

/** Voxel order can be either file (native), or apparent, as set by
 * the function miset_dimension_apparent_voxel_order().
 */
typedef enum {
  MI_ORDER_FILE      = 0,       /**< File order */
  MI_ORDER_APPARENT  = 1        /**< Apparent (user) order  */
} mivoxel_order_t;

/**
 * Voxel flipping can be specified to either follow the file's native
 * order, the opposite of the file's order, or it can be tied to the
 * value of the dimension's step attribute.  A value of MI_NEGATIVE
 * implies that the voxel order should be rearranged such that the step
 * attribute is negative, a value of MI_POSITIVE implies the opposite.
 */
typedef enum {
  MI_FILE_ORDER         = 0,    /**< no flip */
  MI_COUNTER_FILE_ORDER = 1,    /**< flip */
  MI_POSITIVE           = 2,    /**< force step value to be positive */
  MI_NEGATIVE           = 3     /**< force step value to be negative */
} miflipping_t;

/** Compression type
 */
typedef enum {
  MI_COMPRESS_NONE = 0,         /**< No compression */
  MI_COMPRESS_ZLIB = 1          /**< GZIP compression */
} micompression_t;

typedef int miboolean_t;

typedef unsigned int midimattr_t;

typedef unsigned long misize_t;

/** 16-bit integer complex voxel.
 */
typedef struct {
  short real;                   /**< Real part */
  short imag;                   /**< Imaginary part */
} miscomplex_t;

/** 32-bit integer complex voxel.
 */
typedef struct {
  int real;                     /**< Real part */
  int imag;                     /**< Imaginary part */
} miicomplex_t;

/** 32-bit floating point complex voxel.
 */
typedef struct {
  float real;                   /**< Real part */
  float imag;                   /**< Imaginary part */
} mifcomplex_t;

/** 64-bit floating point complex voxel.
 */
typedef struct {
  double real;                  /**< Real part */
  double imag;                  /**< Imaginary part */
} midcomplex_t;



/* Image conversion variable structure type */

typedef struct mi2_icv_struct mi2_icv_type;

struct mi2_icv_struct {

   /* semiprivate : fields available to the package */

   int     do_scale;       /* Indicates to MI_convert_type that scaling should
                              be done */
   double  scale;          /* For scaling in MI_convert_type */
   double  offset;
   int     do_dimconvert;  /* Indicates that dimensional conversion function
                              should be given */
   int   (*dimconvert_func) (int operation, mi2_icv_type *icvp, 
                             long start[], long count[], void *values,
                             long bufstart[], long bufcount[], void *buffer);
   int     do_fillvalue;   /* Indicates to MI_convert_type that fillvalue
                              checking should be done */
   double  fill_valid_min; /* Range limits for fillvalue checking */
   double  fill_valid_max;

   /* private : fields available only to icv routines */

   /* Fields that hold values passed by user */
   mitype_t user_type;      /* Type to that user wants */
   int     user_typelen;   /* Length of type in bytes */
   int     user_sign;      /* Sign that user wants */
   int     user_do_range;  /* Does the user want range scaling? */
   double  user_vmax;      /* Range of values that user wants */
   double  user_vmin;
   int     user_do_norm;   /* Indicates that user wants value normalization */
   int     user_user_norm; /* If TRUE, user specifies range for norm, otherwise
                                 norm is taken from variable range */
   char    *user_maxvar;   /* Name of MIimagemax variable */
   char    *user_minvar;   /* Name of MIimagemin variable */
   double  user_imgmax;    /* Range for normalization */
   double  user_imgmin;
   int     user_do_dimconv; /* Indicates that user wants to do dimension 
                               conversion stuff */
   int     user_do_scalar; /* Indicates that user wants scalar fields */
   int     user_xdim_dir;  /* Direction for x, y and z dimensions */
   int     user_ydim_dir;
   int     user_zdim_dir;
   int     user_num_imgdims; /* Number of image (fastest varying) dimensions */
   long    user_dim_size[MI2_MAX_IMGDIMS]; /* Size of fastest varying 
                                              dimensions for user */
   int     user_keep_aspect; /* Indicates that user wants to preserve the
                                aspect ratio when resizing images */
   int     user_do_fillvalue; /* Indicates that user wants fillvalue checking
                                 to be done */
   double  user_fillvalue;    /* Fillvalue that user wants */

   /* Fields that hold values from real variable */
   mihandle_t      volume;     /* handle to the volume */
   midimhandle_t   imgmaxid;       /* Id of MIimagemax */
   midimhandle_t   imgminid;       /* Id of Miimagemin */
   miboolean_t     slice_scaling; /* Slice scaling enabled */
   
   int     var_ndims;      /* Number of dimensions of variable */
   int     var_dim[MI2_MAX_VAR_DIMS]; /* Dimensions of variable */
   mitype_t var_type;       /* Variable type */
   int     var_typelen;    /* Length of type in bytes */
   int     var_sign;       /* Variable sign */
   double  var_vmax;       /* Range of values in variable */
   double  var_vmin;
   int     var_is_vector;  /* Is this variable a vector field */
   long    var_vector_size; /* Size of vector dimension */
   long    var_dim_size[MI2_MAX_IMGDIMS]; /* Size of image dimensions in 
                                             variable */

   /* Fields derived from user values and variable values */
   int     derv_usr_float; /* Are user or variable values floating point? */
   int     derv_var_float;
   double  derv_imgmax;    /* Range for normalization */
   double  derv_imgmin;
   int     derv_firstdim;  /* First dimension (counting from fastest, ie.
                                 backwards) over which MIimagemax or 
                                 MIimagemin vary */
   int     derv_do_zero;   /* Indicates if we should zero user's buffer
                              on GETs */
   int     derv_do_bufsize_step; /* Indicates if we need to worry about 
                                    bufsize_step */
   int     derv_bufsize_step[MI2_MAX_VAR_DIMS];  /* Array of convenient multiples
                                                for buffer allocation */
   int     derv_var_compress; /* Indicate need for compressing variable or */
   int     derv_usr_compress;    /* user buffer */
   int     derv_dimconv_fastdim;  /* Fastest varying dimensions for dimension
                                     conversion */
   long    derv_var_pix_num; /* Number of pixels to compress/expand for */
   long   *derv_var_pix_off;    /* variable and user buffers, as well as */
   long    derv_usr_pix_num;    /* pointers to arrays of offsets */
   long   *derv_usr_pix_off;
   long    derv_icv_start[MI2_MAX_VAR_DIMS]; /* Space for storing parameters to */
   long    derv_icv_count[MI2_MAX_VAR_DIMS]; /* MI_icv_access */

                           /* Stuff that affects first user_num_imgdims
                              (excluding any vector dimension) as image
                              dimensions */
   int     derv_dim_flip[MI2_MAX_IMGDIMS];   /* Flip dimension? */
   int     derv_dim_grow[MI2_MAX_IMGDIMS];   /* Expand variable to fit user's 
                                                array? */
   int     derv_dim_scale[MI2_MAX_IMGDIMS];  /* Grow/shrink scale factor */
   int     derv_dim_off[MI2_MAX_IMGDIMS];    /* Pixels to skip in user's 
                                                image */
   double  derv_dim_step[MI2_MAX_IMGDIMS];   /* Step, start for user's image 
                                                (analogous to MIstep, 
                                                MIstart) for first 
                                                user_num_imgdims dims */
   double  derv_dim_start[MI2_MAX_IMGDIMS];
};

/* Structure for passing values for MI_icv_dimconvert */
typedef struct {
   int do_compress, do_expand;
   long end[MI2_MAX_VAR_DIMS];
   long in_pix_num,     out_pix_num; /* Variables for compressing/expanding */
   long *in_pix_off,   *out_pix_off;
   void *in_pix_first, *out_pix_first;
   void *in_pix_last,  *out_pix_last;
   mitype_t intype, outtype;     /* Variable types and signs */
   int insign, outsign;
   long buf_step[MI2_MAX_VAR_DIMS];    /* Step sizes for pointers */
   long usr_step[MI2_MAX_VAR_DIMS];
   long *istep, *ostep;
   void *istart, *ostart;       /* Beginning of buffers */
} mi2_icv_dimconv_type;

/* Structure for passing values for MI2_varaccess */
typedef struct {
   int operation;
   mihandle_t  volume;     /* handle to the volume */
   mitype_t var_type, call_type;
   int var_sign, call_sign;
   int var_value_size, call_value_size;
   mi2_icv_type *icvp;
   int do_scale;
   int do_dimconvert;
   int do_fillvalue;
   long *start, *count;
   void *values;
} mi2_varaccess_type;

#endif //__MINC2_STRUCTS_H__