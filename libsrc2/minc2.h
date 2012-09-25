/** \file minc2.h 
 * \brief MINC 2.0 public constants, types, and definitions.
 */

#ifndef _MINC2_H_		/* Avoid multiple inclusions */
#define _MINC2_H_ 1

#ifndef MNCAPI
#if defined(_MSC_VER)
/* If we are building on the Microsoft C compiler, we want to
 * explicitly import all public functions from the DLL
 */
#define MNCAPI __declspec(dllimport)
#else
#define MNCAPI
#endif /* _MSC_VER not defined */
#endif /* MNCAPI not defined */

#ifdef __cplusplus
extern "C" {               /* Hey, Mr. Compiler - this is "C" code! */
#endif /* __cplusplus defined */

/************************************************************************
 * CONSTANTS
 ************************************************************************/

/* Some useful constants */
#define MI_EMPTY_STRING ""
/* Error flags */
#define MI_ERROR (-1)
#define MI_NOERROR 0
/* Maximum length of standard attributes */
#define MI_MAX_ATTSTR_LEN  64
/* Number of spatial dimensions */
#define MI_NUM_SPACE_DIMS 3
/* Maximum number of image dimensions for image conversion */

/* Bert 10-Aug-2004 - MI_MAX_IMGDIMS used to be defined to be MAX_VAR_DIMS,
 * a constant defined in netcdf.h. For many years MAX_VAR_DIMS was 100,
 * but in netCDF 3.5.1 the value was changed to 512.
 * Unfortunately, the definitions of MI_ICV_DIM_SIZE, MI_ICV_DIM_STEP,
 * and MI_ICV_DIM_START assume that MI_MAX_IMGDIMS is less than or
 * equal to 100.  To avoid changing the MINC API, we have to define
 * MI_MAX_IMGDIMS to 100 here.  Otherwise the miicv_inqdbl() function
 * will return bogus values for these ICV properties.
 */
#define MI_MAX_IMGDIMS 100

/* NetCDF standard attributes */
#define MIunits       "units"
#define MIlong_name   "long_name"
#define MIvalid_range "valid_range"
#define MIvalid_max   "valid_max"
#define MIvalid_min   "valid_min"
#define MI_FillValue  "_FillValue"
#define MItitle       "title"
#define MIhistory     "history"

/* General variable attributes */
#define MIvartype  "vartype"
#define MIvarid    "varid"
#define MIsigntype "signtype"
#define MIparent   "parent"
#define MIchildren "children"
#define MIcomments "comments"
#define MIversion  "version"

/* General attribute constants */
/*    Prefix for identifying a variable attribute pointer */
#define MI_VARATT_POINTER_PREFIX "--->"
/*    Separator for elements of MIchildren */
#define MI_CHILD_SEPARATOR "\n"
/*    MIvartype values */
#define MI_GROUP     "group________"
#define MI_DIMENSION "dimension____"
#define MI_DIM_WIDTH "dim-width____"
#define MI_VARATT    "var_attribute"
/*    MIvarid value */
#define MI_STDVAR "MINC standard variable"
/*    MIsigntype values */
#define MI_SIGNED   "signed__"
#define MI_UNSIGNED "unsigned"
/*    MIversion value */
#define MI_VERSION_1_0 "MINC Version    1.0"
#define MI_CURRENT_VERSION MI_VERSION_1_0
/* Generally useful values for boolean attributes */
#define MI_TRUE  "true_"
#define MI_FALSE "false"

/* Dimension names and names of associated variables */
#define MIxspace           "xspace"
#define MIyspace           "yspace"
#define MIzspace           "zspace"
#define MItime             "time"
#define MItfrequency       "tfrequency"
#define MIxfrequency       "xfrequency"
#define MIyfrequency       "yfrequency"
#define MIzfrequency       "zfrequency"
#define MIvector_dimension "vector_dimension"
#define MIxspace_width     "xspace-width"
#define MIyspace_width     "yspace-width"
#define MIzspace_width     "zspace-width"
#define MItime_width       "time-width"
#define MItfrequency_width "tfrequency-width"
#define MIxfrequency_width "xfrequency-width"
#define MIyfrequency_width "yfrequency-width"
#define MIzfrequency_width "zfrequency-width"

/* Dimension variable attribute names */
/* For dimension variables (MIspacing is also for dimension width vars) */
#define MIspacing           "spacing"
#define MIstep              "step"
#define MIstart             "start"
#define MIspacetype         "spacetype"
#define MIalignment         "alignment"
#define MIdirection_cosines "direction_cosines"
/* For dimension width variables */
#define MIwidth             "width"
#define MIfiltertype        "filtertype"

/* Dimension attribute constants */
/*    MIgridtype values */
#define MI_REGULAR   "regular__"
#define MI_IRREGULAR "irregular"
/*    MIspacetype values */
#define MI_NATIVE    "native____"
#define MI_TALAIRACH "talairach_"
#define MI_CALLOSAL  "callosal__"
/*    MIalignment values */
#define MI_START  "start_"
#define MI_CENTRE "centre"
#define MI_END    "end___"
#define MI_CENTER MI_CENTRE
/*    MIfiltertype values */
#define MI_SQUARE     "square____"
#define MI_GAUSSIAN   "gaussian__"
#define MI_TRIANGULAR "triangular"

/* The root variable */
#define MIrootvariable "rootvariable"

/* The image variable and its attributes */
#define MIimage    "image"
#define MIimagemax "image-max"
#define MIimagemin "image-min"
#define MIcomplete "complete"

/* The patient variable and its attributes */
#define MIpatient        "patient"
#define MIfull_name      "full_name"
#define MIother_names    "other_names"
#define MIidentification "identification"
#define MIother_ids      "other_ids"
#define MIbirthdate      "birthdate"
#define MIsex            "sex"
#define MIage            "age"
#define MIweight         "weight"
#define MIsize           "size"
#define MIaddress        "address"
#define MIinsurance_id   "insurance_id"

/* Patient attribute constants */
#define MI_MALE   "male__"
#define MI_FEMALE "female"
#define MI_OTHER  "other_"

/* The study variable and its attributes */
#define MIstudy               "study"
#define MIstart_time          "start_time"
#define MIstart_year          "start_year"
#define MIstart_month         "start_month"
#define MIstart_day           "start_day"
#define MIstart_hour          "start_hour"
#define MIstart_minute        "start_minute"
#define MIstart_seconds       "start_seconds"
#define MImodality            "modality"
#define MImanufacturer        "manufacturer"
#define MIdevice_model        "device_model"
#define MIinstitution         "institution"
#define MIdepartment          "department"
#define MIstation_id          "station_id"
#define MIreferring_physician "referring_physician"
#define MIattending_physician "attending_physician"
#define MIradiologist         "radiologist"
#define MIoperator            "operator"
#define MIadmitting_diagnosis "admitting_diagnosis"
#define MIprocedure           "procedure"
#define MIstudy_id            "study_id"

/* Study attribute constants */
#define MI_PET   "PET__"
#define MI_SPECT "SPECT"
#define MI_GAMMA "GAMMA"
#define MI_MRI   "MRI__"
#define MI_MRS   "MRS__"
#define MI_MRA   "MRA__"
#define MI_CT    "CT___"
#define MI_DSA   "DSA__"
#define MI_DR    "DR___"
#define MI_LABEL "label"

/* The acquisition variable and its attributes */
#define MIacquisition           "acquisition"
#define MIprotocol              "protocol"
#define MIscanning_sequence     "scanning_sequence"
#define MIrepetition_time       "repetition_time"
#define MIecho_time             "echo_time"
#define MIinversion_time        "inversion_time"
#define MInum_averages          "num_averages"
#define MIimaging_frequency     "imaging_frequency"
#define MIimaged_nucleus        "imaged_nucleus"
#define MIradionuclide          "radionuclide"
#define MIcontrast_agent        "contrast_agent"
#define MIradionuclide_halflife "radionuclide_halflife"
#define MItracer                "tracer"
#define MIinjection_time        "injection_time"
#define MIinjection_year        "injection_year"
#define MIinjection_month       "injection_month"
#define MIinjection_day         "injection_day"
#define MIinjection_hour        "injection_hour"
#define MIinjection_minute      "injection_minute"
#define MIinjection_seconds     "injection_seconds"
#define MIinjection_length      "injection_length"
#define MIinjection_dose        "injection_dose"
#define MIdose_units            "dose_units"
#define MIinjection_volume      "injection_volume"
#define MIinjection_route       "injection_route"

/* Constants for image conversion variable (icv) properties */
/* Maximum number of icv's allowed */
/* changed to 32 as netcdf 4.x no longer defines MAX_NC_OPEN */
/* #define MI_MAX_NUM_ICV MAX_NC_OPEN */
#define MI_MAX_NUM_ICV 32

/* Default max and min for normalization */
#define MI_DEFAULT_MAX 1.0
#define MI_DEFAULT_MIN 0.0
/* For converting data type */
#define MI_ICV_TYPE             1
#define MI_ICV_SIGN             2
#define MI_ICV_DO_RANGE         3
#define MI_ICV_VALID_MAX        4
#define MI_ICV_VALID_MIN        5
/* For doing normalization */
#define MI_ICV_DO_NORM          6
#define MI_ICV_USER_NORM        7
#define MI_ICV_IMAGE_MAX        8
#define MI_ICV_IMAGE_MIN        9
/* Values actually used in normalization - read-only */
#define MI_ICV_NORM_MAX        10
#define MI_ICV_NORM_MIN        11
/* For doing dimension conversions */
#define MI_ICV_DO_DIM_CONV     12
/* For converting vector fields to scalar */
#define MI_ICV_DO_SCALAR       13
/* For flipping axis direction */
#define MI_ICV_XDIM_DIR        14
#define MI_ICV_YDIM_DIR        15
#define MI_ICV_ZDIM_DIR        16
/* For changing size of first two dimensions (excluding MIvector_dimension) */
#define MI_ICV_ADIM_SIZE       17
#define MI_ICV_BDIM_SIZE       18
#define MI_ICV_KEEP_ASPECT     19
/* The pixel size and location of first two dimensions (these are readonly) */
#define MI_ICV_ADIM_STEP       20
#define MI_ICV_BDIM_STEP       21
#define MI_ICV_ADIM_START      22
#define MI_ICV_BDIM_START      23
/* Number of image dimensions for dimension conversion */
#define MI_ICV_NUM_IMGDIMS     24
/* Number of dimensions of image variable taking into account vector/scalar
   data (read-only property) */
#define MI_ICV_NUM_DIMS        25
/* Id of file and image variable (read-only properties) */
#define MI_ICV_CDFID           26
#define MI_ICV_VARID           27
/* Names of MIimagemax and MIimagemin variables */
#define MI_ICV_MAXVAR          28
#define MI_ICV_MINVAR          29
/* For setting input values to a specified fillvalue */
#define MI_ICV_DO_FILLVALUE    30
#define MI_ICV_FILLVALUE       31
/* Image dimension properties. For each dimension, add the dimension 
   number (counting from fastest to slowest). */
#define MI_ICV_DIM_SIZE        1000
#define MI_ICV_DIM_STEP        1100
#define MI_ICV_DIM_START       1200

/* Constants that can be used as values for the above properties. */
/* Possible values for MI_ICV_?DIM_DIR */
#define MI_ICV_POSITIVE         1
#define MI_ICV_NEGATIVE       (-1)
#define MI_ICV_ANYDIR           0
/* Possible value for MI_ICV_?DIM_SIZE */
#define MI_ICV_ANYSIZE        (-1)

/* Error codes.
   Note that they must not conflict with NetCDF error codes since
   they are stored in the same global variable. */
#define MI_ERR_NONNUMERIC       1331  /* Non-numeric type */
#define MI_ERR_NONCHAR          1332  /* Non-character type */
#define MI_ERR_NONSCALAR        1333  /* Non-scalar attribute */
#define MI_ERR_BADOP            1334  /* Bad operation for MI_varaccess */
#define MI_ERR_NOTPOINTER       1335  /* Attribute is not a pointer */
#define MI_ERR_BAD_STDVAR       1336  /* Not a standard variable */
#define MI_ERR_BADSUFFIX        1337  /* Bad dimension width suffix */
#define MI_ERR_NOICV            1338  /* Out of icv slots */
#define MI_ERR_BADICV           1339  /* Illegal icv identifier */
#define MI_ERR_BADPROP          1340  /* Unknown icv property */
#define MI_ERR_ICVATTACHED      1341  /* Tried to modify attached icv */
#define MI_ERR_TOOFEWDIMS       1342  /* Too few dimensions to be an image */
#define MI_ERR_ICVNOTATTACHED   1343  /* Tried to access an unattached icv */
#define MI_ERR_DIMSIZE          1344  /* Dimensions differ in size */
#define MI_ERR_ICV_INVCOORDS    1345  /* Invalid icv coordinates */
#define MI_ERR_WRONGNDIMS       1346  /* Too many dimensions for a dim var */
#define MI_ERR_BADMATCH         1347  /* Variables do not match for copy */
#define MI_ERR_MAXMIN_DIMS      1348  /* Imagemax/min variables vary over
                                         image dimensions */
#define MI_ERR_UNCOMPRESS       1349  /* Not able to uncompress file */


#ifndef MI_NOERROR
/** Generic return code for successful operations. */
#define MI_NOERROR 0
#endif /* MI_NOERROR not defined */

#ifndef MI_ERROR
/** Generic return code for operations which fail for any reason. */
#define MI_ERROR (-1)
#endif /* MI_ERROR not defined */

#define MI_NATIVE    "native____"
#define MI_TALAIRACH "talairach_"
#define MI_CALLOSAL  "callosal__"

#ifndef TRUE
#define TRUE  1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

/** World spatial coordinates should always have this structure.
 */
#define MI2_3D 3
#define MI2_X 0
#define MI2_Y 1
#define MI2_Z 2

/** Dimension attribute values.
 */
#define MI_DIMATTR_ALL 0
#define MI_DIMATTR_REGULARLY_SAMPLED 0x1
#define MI_DIMATTR_NOT_REGULARLY_SAMPLED 0x2

/** Maximum length of a standard string.
 */
#define MI2_CHAR_LENGTH 128

/** Maximum number of dimensions a variable can have.
 */
#define MI2_MAX_VAR_DIMS 100

#define MI2_CHUNK_SIZE 32	/* Length of chunk, per dimension */
#define MI2_DEFAULT_ZLIB_LEVEL 4
#define MI2_MAX_ZLIB_LEVEL 9

#define MI2_MAX_PATH 128
#define MI2_MAX_RESOLUTION_GROUP 16

#define MI2_OPEN_READ 0x0001
#define MI2_OPEN_RDWR 0x0002

#define MI_VERSION_2_0 "MINC Version    2.0"
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
  MI_TYPE_BYTE = 1,		/**< 8-bit signed integer */
  MI_TYPE_SHORT = 3,		/**< 16-bit signed integer */
  MI_TYPE_INT = 4,		/**< 32-bit signed integer */
  MI_TYPE_FLOAT = 5,		/**< 32-bit floating point */
  MI_TYPE_DOUBLE = 6,		/**< 64-bit floating point */
  MI_TYPE_STRING = 7,		/**< ASCII string */
  MI_TYPE_UBYTE = 100,		/**< 8-bit unsigned integer */
  MI_TYPE_USHORT = 101,		/**< 16-bit unsigned integer */
  MI_TYPE_UINT = 102,		/**< 32-bit unsigned integer */
  MI_TYPE_SCOMPLEX = 1000,	/**< 16-bit signed integer complex */
  MI_TYPE_ICOMPLEX = 1001,	/**< 32-bit signed integer complex */
  MI_TYPE_FCOMPLEX = 1002,	/**< 32-bit floating point complex */
  MI_TYPE_DCOMPLEX = 1003,	/**< 64-bit floating point complex */
  MI_TYPE_UNKNOWN  = -1         /**< when the type is a record */
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
    MI_DIMCLASS_ANY = 0,	/**< Don't care (or unknown) */
    MI_DIMCLASS_SPATIAL = 1,	/**< Spatial dimensions (x, y, z) */
    MI_DIMCLASS_TIME = 2,	/**< Time dimension */
    MI_DIMCLASS_SFREQUENCY = 3,	/**< Spatial frequency dimensions */
    MI_DIMCLASS_TFREQUENCY = 4,	/**< Temporal frequency dimensions */
    MI_DIMCLASS_USER = 5,	/**< Arbitrary user-defined dimension */
    MI_DIMCLASS_RECORD = 6	/**< Record as dimension */
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

/************************************************************************
 * FUNCTION DECLARATIONS
 ************************************************************************/

/* ATTRIBUTE/GROUP FUNCTIONS */
extern int milist_start(mihandle_t vol, const char *path, int flags,
			milisthandle_t *handle);
extern int milist_attr_next(mihandle_t vol, milisthandle_t handle, 
			    char *path, int maxpath,
			    char *name, int maxname);
extern int milist_finish(milisthandle_t handle);
extern int milist_grp_next(milisthandle_t handle, char *path, int maxpath);
extern int micreate_group(mihandle_t vol, const char *path, const char *name);
extern int midelete_attr(mihandle_t vol, const char *path, const char *name);
extern int midelete_group(mihandle_t vol, const char *path, const char *name);
extern int miget_attr_length(mihandle_t vol, const char *path, 
			     const char *name, int *length);
extern int miget_attr_type(mihandle_t vol, const char *path, const char *name,
			   mitype_t *data_type);
extern int micopy_attr(mihandle_t vol, const char *path, mihandle_t new_vol);
extern int miget_attr_values(mihandle_t vol, mitype_t data_type,
			     const char *path, const char *name, 
			     int length, void *values);
extern int miset_attr_values(mihandle_t vol, mitype_t data_type,
			     const char *path, const char *name, int length,
			     const void *values);
extern int miadd_history_attr(mihandle_t vol, int length, const void *values);			     
/* FREE FUNCTIONS */
extern int mifree_name(char *name_ptr);
extern int mifree_names(char **name_pptr);

/* DATA TYPE/SPACE FUNCTIONS */
extern int miget_data_class(mihandle_t vol, miclass_t *volume_class);
extern int miget_data_type(mihandle_t vol, mitype_t *volume_data_type);
extern int miget_data_type_size(mihandle_t vol, misize_t *voxel_size);
extern int miget_space_name(mihandle_t vol, char **name);
extern int miset_space_name(mihandle_t vol, const char *name);

/* DIMENSION FUNCTIONS */
extern int miget_volume_from_dimension(midimhandle_t dimension, mihandle_t *volume);
extern int micopy_dimension(midimhandle_t dim_ptr, midimhandle_t *new_dim_ptr);
extern int micreate_dimension(const char *name, midimclass_t dimclass, midimattr_t attr, 
			      unsigned int length, midimhandle_t *new_dim_ptr);
extern int mifree_dimension_handle(midimhandle_t dim_ptr);
extern int miget_volume_dimensions(mihandle_t volume, midimclass_t dimclass, midimattr_t attr,
				   miorder_t order, int array_length, 
				   midimhandle_t dimensions[]);
extern int miset_apparent_dimension_order(mihandle_t volume, int array_length, midimhandle_t dimensions[]);
extern int miset_apparent_dimension_order_by_name(mihandle_t volume, int array_length, char **names);
extern int miset_apparent_record_dimension_flag(mihandle_t volume, int record_flag);
extern int miget_dimension_apparent_voxel_order(midimhandle_t dimension, miflipping_t *file_order,
						miflipping_t *sign);
extern int miset_dimension_apparent_voxel_order(midimhandle_t dimension, miflipping_t flipping_order);
extern int miget_dimension_class(midimhandle_t dimension, midimclass_t *dimclass);
extern int miset_dimension_class(midimhandle_t dimension, midimclass_t dimclass);
extern int miget_dimension_cosines(midimhandle_t dimension, 
                                   double direction_cosines[3]);
extern int miset_dimension_cosines(midimhandle_t dimension, 
                                   const double direction_cosines[3]);
extern int miset_dimension_description(midimhandle_t dimension, const char *comments);
extern int miget_dimension_description(midimhandle_t dimension, char **comments_ptr);
extern int miget_dimension_name(midimhandle_t dimension, char **name_ptr);
extern int miset_dimension_name(midimhandle_t dimension, const char *name);
extern int miget_dimension_offsets(midimhandle_t dimension, unsigned long array_length, 
				   unsigned long start_position, double offsets[]);
extern int miset_dimension_offsets(midimhandle_t dimension, unsigned long array_length, 
				   unsigned long start_position, const double offsets[]);
extern int miget_dimension_sampling_flag(midimhandle_t dimension, miboolean_t *sampling_flag);
extern int miset_dimension_sampling_flag(midimhandle_t dimension, miboolean_t sampling_flag);
extern int miget_dimension_separation(midimhandle_t dimension, 
                                      mivoxel_order_t voxel_order, 
				      double *separation_ptr);
extern int miset_dimension_separation(midimhandle_t dimension, 
                                      double separation);
extern int miget_dimension_separations(const midimhandle_t dimensions[], 
                                       mivoxel_order_t voxel_order, 
                                       int array_length, 
                                       double separations[]);
extern int miset_dimension_separations(const midimhandle_t dimensions[], int array_length,
				 const double separations[]);
extern int miget_dimension_size(midimhandle_t dimension, unsigned int *size_ptr);
extern int miset_dimension_size(midimhandle_t dimension, unsigned int size);
extern int miget_dimension_sizes(const midimhandle_t dimensions[], int array_length,
				 unsigned int sizes[]);
extern int miget_dimension_start(midimhandle_t dimension, 
                                 mivoxel_order_t voxel_order,
				 double *start_ptr);
extern int miset_dimension_start(midimhandle_t dimension, double start_ptr);
extern int miget_dimension_starts(const midimhandle_t dimensions[], mivoxel_order_t voxel_order,
				  int array_length, double starts[]);
extern int miset_dimension_starts(const midimhandle_t dimensions[], int array_length, 
				  const double starts[]);
extern int miget_dimension_units(midimhandle_t dimension, char **units_ptr);
extern int miset_dimension_units(midimhandle_t dimension, const char *units);
extern int miget_dimension_width(midimhandle_t dimension, double *width_ptr);
extern int miset_dimension_width(midimhandle_t dimension, double width_ptr);
extern int miget_dimension_widths(midimhandle_t dimension, mivoxel_order_t voxel_order,
				  unsigned long array_length, unsigned long start_position,
				  double widths[]);
extern int miset_dimension_widths(midimhandle_t dimension, unsigned long array_length,
				  unsigned long start_position, const double widths[]);


/* VOLUME FUNCTIONS */
extern int micreate_volume(const char *filename, int number_of_dimensions,
			   midimhandle_t dimensions[],
			   mitype_t volume_type,
			   miclass_t volume_class,
			   mivolumeprops_t create_props,
			   mihandle_t *volume);
extern int micreate_volume_image(mihandle_t volume);
extern int miget_volume_dimension_count(mihandle_t volume, midimclass_t dimclass,
					midimattr_t attr, int *number_of_dimensions);
extern int miget_volume_voxel_count(mihandle_t volume, int *number_of_voxels);
extern int miopen_volume(const char *filename, int mode, mihandle_t *volume);
extern int miclose_volume(mihandle_t volume);

extern int miget_slice_scaling_flag(mihandle_t volume, 
				    miboolean_t *slice_scaling_flag);
extern int miset_slice_scaling_flag(mihandle_t volume, 
				    miboolean_t slice_scaling_flag);

/* VOLUME PROPERTIES FUNCTIONS */
extern int minew_volume_props(mivolumeprops_t *props);
extern int mifree_volume_props(mivolumeprops_t props);
extern int miget_volume_props(mihandle_t vol, mivolumeprops_t *props);
extern int miset_props_multi_resolution(mivolumeprops_t props, miboolean_t enable_flag, 
				int depth);
extern int miget_props_multi_resolution(mivolumeprops_t props, miboolean_t *enable_flag,
				int *depth);
extern int miselect_resolution(mihandle_t volume, int depth);
extern int miflush_from_resolution(mihandle_t volume, int depth);
extern int miset_props_compression_type(mivolumeprops_t props, micompression_t compression_type);
extern int miget_props_compression_type(mivolumeprops_t props, micompression_t *compression_type);
extern int miset_props_zlib_compression(mivolumeprops_t props, int zlib_level);
extern int miget_props_zlib_compression(mivolumeprops_t props, int *zlib_level);
extern int miset_props_blocking(mivolumeprops_t props, int edge_count, const int *edge_lengths);
extern int miget_props_blocking(mivolumeprops_t props, int *edge_count, int *edge_lengths,
				int max_lengths);
extern int miset_props_record(mivolumeprops_t props, long record_length, char *record_name); 
extern int miset_props_template(mivolumeprops_t props, int template_flag);

/* SLICE/VOLUME SCALE FUNCTIONS */
extern int miget_slice_max(mihandle_t volume, 
			   const unsigned long start_positions[],
			   int array_length, double *slice_max);
extern int miset_slice_max(mihandle_t volume, 
			   const unsigned long start_positions[],
			   int array_length, double slice_max);
extern int miget_slice_min(mihandle_t volume, 
			   const unsigned long start_positions[],
			   int array_length, double *slice_min);
extern int miset_slice_min(mihandle_t volume, 
			   const unsigned long start_positions[],
			   int array_length, double slice_min);
extern int miget_slice_range(mihandle_t volume,
			     const unsigned long start_positions[],
			     int array_length, double *slice_max,
			     double *slice_min);
extern int miset_slice_range(mihandle_t volume, 
			     const unsigned long start_positions[],
			     int array_length, double slice_max, 
			     double slice_min);
extern int miget_volume_max(mihandle_t volume, double *slice_max);
extern int miset_volume_max(mihandle_t volume, double slice_max);
extern int miget_volume_min(mihandle_t volume, double *slice_min);
extern int miset_volume_min(mihandle_t volume, double slice_min);
extern int miget_volume_range(mihandle_t volume, double *slice_max, 
			      double *slice_min);
extern int miset_volume_range(mihandle_t volume, double slice_max, 
			      double slice_min);
/* HYPERSLAB FUNCTIONS */
extern int miget_hyperslab_size(mitype_t volume_data_type, int n_dimensions, 
				const unsigned long count[], 
				misize_t *size_ptr);

extern int miget_hyperslab_normalized(mihandle_t volume, 
                                      mitype_t buffer_data_type,
                                      const unsigned long start[], 
                                      const unsigned long count[],
                                      double min, 
                                      double max, 
                                      void *buffer);

extern int miget_hyperslab_with_icv(mihandle_t volume, 
				    int icv,
				    mitype_t buffer_data_type, 
				    const unsigned long start[], 
				    const unsigned long count[], 
				    void *buffer);

extern int miset_hyperslab_with_icv(mihandle_t volume,
				    int icv, 
				    mitype_t buffer_data_type, 
				    const unsigned long start[],
				    const unsigned long count[],
				    void *buffer);

extern int miget_real_value_hyperslab(mihandle_t volume,
                                      mitype_t buffer_data_type,
                                      const unsigned long start[],
                                      const unsigned long count[],
                                      void *buffer);

extern int miset_real_value_hyperslab(mihandle_t volume,
                                      mitype_t buffer_data_type,
                                      const unsigned long start[],
                                      const unsigned long count[],
                                      void *buffer);

extern int miget_voxel_value_hyperslab(mihandle_t volume,
                                       mitype_t buffer_data_type,
                                       const unsigned long start[],
                                       const unsigned long count[],
                                       void *buffer);

extern int miset_voxel_value_hyperslab(mihandle_t volume,
                                       mitype_t buffer_data_type,
                                       const unsigned long start[],
                                       const unsigned long count[],
                                       void *buffer);



/* CONVERT FUNCTIONS */
extern int miconvert_real_to_voxel(mihandle_t volume,
                                   const unsigned long coords[],
                                   int ncoords,
                                   double real_value,
                                   double *voxel_value_ptr);

extern int miconvert_voxel_to_real(mihandle_t volume,
                                   const unsigned long coords[],
                                   int ncoords,
                                   double voxel_value,
                                   double *real_value_ptr);

extern int miconvert_voxel_to_world(mihandle_t volume,
                                    const double voxel[],
                                    double world[]);

extern int miconvert_world_to_voxel(mihandle_t volume,
                                    const double world[],
                                    double voxel[]);

extern int miget_real_value(mihandle_t volume,
                            const unsigned long coords[],
                            int ndims,
                            double *value_ptr);

extern int miset_real_value(mihandle_t volume,
                            const unsigned long coords[],
                            int ndims,
                            double value);
extern int miget_voxel_value(mihandle_t volume,
                             const unsigned long coords[],
                             int ndims,
                             double *voxel_ptr);

extern int miset_voxel_value(mihandle_t volume,
                             const unsigned long coords[],
                             int ndims,
                             double voxel);

extern int miget_volume_real_range(mihandle_t volume, double real_range[2]);

extern int miset_world_origin(mihandle_t volume, double origin[MI2_3D]);

/* VALID functions */
extern int miget_volume_valid_max(mihandle_t volume, double *valid_max);
extern int miset_volume_valid_max(mihandle_t volume, double valid_max);
extern int miget_volume_valid_min(mihandle_t volume, double *valid_min);
extern int miset_volume_valid_min(mihandle_t volume, double valid_min);
extern int miget_volume_valid_range(mihandle_t volume,
                                    double *valid_max, double *valid_min);
extern int miset_volume_valid_range(mihandle_t volume, 
                                    double valid_max, double valid_min);

/* RECORD functions */
extern int miget_record_name(mihandle_t volume, char **name);
extern int miget_record_length(mihandle_t volume, int *length);
extern int miget_record_field_name(mihandle_t volume, int index, char **name);
extern int miset_record_field_name(mihandle_t volume, int index, 
                                   const char *name);

/* LABEL functions */
extern int midefine_label(mihandle_t volume, int value, const char *name);
extern int miget_label_name(mihandle_t volume, int value, char **name);
extern int miget_label_value(mihandle_t volume, const char *name, int *value);
extern int miget_number_of_defined_labels(mihandle_t volume, int *number_of_labels);
  extern int miget_label_value_by_index(mihandle_t volume, int idx, int *value);

#ifdef __cplusplus
}
#endif /* __cplusplus defined */

#endif /* _MINC2_H_ */
