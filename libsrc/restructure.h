/*
 * \file restructure.h
 * \brief Just declares the prototype of restructure_array().
 */
extern void restructure_array(int ndims,
                              unsigned char *array, 
                              const size_t *lengths_perm,
                              int el_size,
                              const int *map,
                              const int *dir);

