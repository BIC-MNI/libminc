/**minc2 API */
#ifndef __MINC2_API_H__
#define __MINC2_API_H__ 1

#ifdef __cplusplus
extern "C" {               /* Hey, Mr. Compiler - this is "C" code! */
#endif /* __cplusplus defined */


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


#endif /*__MINC2_API_H__*/