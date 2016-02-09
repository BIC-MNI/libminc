/* Long overdue tests for the basic volume functions.
 * This was written largely as a way for me to make certain that I 
 * got copy_volume_new_type() more-or-less correct [bert].
 */
#include <volume_io.h>

#define ERROR fprintf(stderr, "ERROR in %s:%d\n", __func__, __LINE__)

int
test1(void)
{
  VIO_Volume v1;
  VIO_Volume v2;
  int sizes[VIO_MAX_DIMENSIONS] = { 11, 13, 17, 19, 23 };
  int read_sizes[VIO_MAX_DIMENSIONS] = { 1, 1, 1, 1, 1 };
  int i, j, k, t, v;

  v1 = create_volume(5, NULL, NC_BYTE, FALSE, 0, 255);

  if (v1 == NULL)
  {
    ERROR;
    return 1;
  }

  if (get_volume_n_dimensions( v1 ) != 5)
  {
    ERROR;
    return 1;
  }

  if (volume_is_alloced( v1 ))
  {
    ERROR;
    return 1;
  }

  get_volume_sizes( v1, read_sizes );

  if (get_volume_total_n_voxels( v1 ) != 0)
  {
    ERROR;
    return 1;
  }

  if (get_volume_data_type( v1 ) != VIO_UNSIGNED_BYTE )
  {
    ERROR;
    return 1;
  }

  set_volume_sizes( v1, sizes );

  if (get_volume_total_n_voxels (v1) != ((size_t) sizes[0] *
                                         (size_t) sizes[1] *
                                         (size_t) sizes[2] *
                                         (size_t) sizes[3] *
                                         (size_t) sizes[4]))
  {
    ERROR;
    return 1;
  }
  
  get_volume_sizes( v1, read_sizes );
  for (i = 0; i < get_volume_n_dimensions( v1 ); i++)
  {
    if (read_sizes[i] != sizes[i])
    {
      ERROR;
      return 1;
    }
  }

  alloc_volume_data( v1 );

  srand(0);
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
        for (t = 0; t < sizes[3]; t++)
          for (v = 0; v < sizes[4]; v++)
            set_volume_voxel_value( v1, i, j, k, t, v, rand() % 256 );

  srand(0);
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
        for (t = 0; t < sizes[3]; t++)
          for (v = 0; v < sizes[4]; v++)
          {
            VIO_Real value = get_volume_voxel_value( v1, i, j, k, t, v ) ;
            if (value != rand() % 256)
            {
              ERROR;
              return 1;
            }
          }

  v2 = copy_volume( v1 );
  if ( v2 == NULL )
  {
    ERROR;
    return 1;
  }

  srand(0);
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
        for (t = 0; t < sizes[3]; t++)
          for (v = 0; v < sizes[4]; v++)
          {
            VIO_Real value = get_volume_voxel_value( v2, i, j, k, t, v );
            int expected_value = rand() % 256;
            if ( value != expected_value )
            {
              ERROR;
              return 1;
            }
            set_volume_voxel_value( v2, i, j, k, t, v, value + 1 );

            value = get_volume_voxel_value( v1, i, j, k, t, v );
            if (value != expected_value )
            {
              ERROR;
              return 1;
            }
          }

  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
        for (t = 0; t < sizes[3]; t++)
          for (v = 0; v < sizes[4]; v++)
          {
            VIO_Real x1 = get_volume_voxel_value( v1, i, j, k, t, v );
            VIO_Real x2 = get_volume_voxel_value( v2, i, j, k, t, v );
            
            if (x2 != (int)(x1 + 1) % 256)
            {
              ERROR;
              fprintf(stderr, "%d %d %d %d %d: %f %f\n", 
                      i, j, k, t, v, x1, x2 );
              return 1;
            }
          }

  delete_volume( v2 );

  /* Expand volume from unsigned char to signed int.
   */
  v2 = copy_volume_new_type( v1, NC_INT, TRUE );
  if ( v2 == NULL )
  {
    ERROR;
    return 1;
  }
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
        for (t = 0; t < sizes[3]; t++)
          for (v = 0; v < sizes[4]; v++)
          {
            VIO_Real x1 = get_volume_voxel_value( v1, i, j, k, t, v );
            VIO_Real x2 = get_volume_voxel_value( v2, i, j, k, t, v );
            
            if ( x2 != x1 )
            {
              ERROR;
              fprintf(stderr, "%d %d %d %d %d: %f %f\n", 
                      i, j, k, t, v, x1, x2 );
              return 1;
            }

            set_volume_voxel_value( v2, i, j, k, t, v, x1 * 1000 );
          }

  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
        for (t = 0; t < sizes[3]; t++)
          for (v = 0; v < sizes[4]; v++)
          {
            VIO_Real x1 = get_volume_voxel_value( v1, i, j, k, t, v );
            VIO_Real x2 = get_volume_voxel_value( v2, i, j, k, t, v );
            
            if ( x2 != x1 * 1000 )
            {
              ERROR;
              fprintf(stderr, "%d %d %d %d %d: %f %f\n", 
                      i, j, k, t, v, x1, x2 );
              return 1;
            }
          }
  
  delete_volume( v2 );

  delete_volume( v1 );
  return 0;
}

int
test2(void)
{
  VIO_Volume v1;
  VIO_Volume v2;
  int sizes[VIO_MAX_DIMENSIONS] = { 172, 256, 256, 3, 3 };
  int read_sizes[VIO_MAX_DIMENSIONS] = { 1, 1, 1, 1, 1 };
  int i, j, k;
  char *dim_names[] = { 
    MIzspace, MIyspace, MIxspace, MItime, MIvector_dimension
  };

  v1 = create_volume(5, dim_names, NC_SHORT, TRUE, 0, 10000);

  if (v1 == NULL)
  {
    ERROR;
    return 1;
  }

  if ( get_volume_n_dimensions( v1 ) != 5 )
  {
    ERROR;
    return 1;
  }

  set_volume_n_dimensions( v1, 3 );

  if ( get_volume_n_dimensions( v1 ) != 3 )
  {
    ERROR;
    return 1;
  }

  if (volume_is_alloced( v1 ))
  {
    ERROR;
    return 1;
  }

  get_volume_sizes( v1, read_sizes );

  if (get_volume_total_n_voxels( v1 ) != 0)
  {
    ERROR;
    return 1;
  }

  if (get_volume_data_type( v1 ) != VIO_SIGNED_SHORT )
  {
    ERROR;
    return 1;
  }

  set_volume_sizes( v1, sizes );

  if (get_volume_total_n_voxels( v1 ) != ((size_t) sizes[0] *
                                          (size_t) sizes[1] *
                                          (size_t) sizes[2]))

  {
    ERROR;
    return 1;
  }
  
  get_volume_sizes( v1, read_sizes );
  for (i = 0; i < get_volume_n_dimensions( v1 ); i++)
  {
    if (read_sizes[i] != sizes[i])
    {
      ERROR;
      return 1;
    }
  }

  alloc_volume_data( v1 );

  set_volume_n_dimensions( v1, 2 ); /* Should be ignored! */

  srand(0);
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
        set_volume_voxel_value( v1, i, j, k, 0, 0, rand() % 32768 );

  srand(0);
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
      {
        VIO_Real x1 = get_volume_voxel_value( v1, i, j, k, 0, 0 ) ;
        /* There is no range check even if we set the max and min voxel
         * values, so the only constraint on values is the precision of
         * the signed short.
         */
        int expected_value = (short)(rand() % 32768);
        if ( x1 != expected_value )
        {
          ERROR;
          fprintf(stderr, "%d %d %d: %f %d\n", 
                  i, j, k, x1, expected_value );
          return 1;
        }
      }

  v2 = copy_volume_new_type( v1, NC_FLOAT, TRUE );
  if (v2 == NULL)
  {
    ERROR;
    return 1;
  }

  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
      {
        VIO_Real x1 = get_volume_voxel_value( v1, i, j, k, 0, 0 ) ;
        VIO_Real x2 = get_volume_voxel_value( v2, i, j, k, 0, 0 ) ;

        if ( x1 != x2 )
        {
          ERROR;
          fprintf(stderr, "%d %d %d: %f %f\n", 
                  i, j, k, x1, x2 );
          return 1;
        }

        set_volume_voxel_value( v2, i, j, k, 0, 0, x1 * 1000.0 );
      }

  
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++)
      {
        VIO_Real x1 = get_volume_voxel_value( v1, i, j, k, 0, 0 ) ;
        VIO_Real x2 = get_volume_voxel_value( v2, i, j, k, 0, 0 ) ;

        if ( x1 * 1000.0 != x2 )
        {
          ERROR;
          fprintf(stderr, "%d %d %d: %f %f\n", 
                  i, j, k, x1, x2 );
          return 1;
        }
      }

  
  delete_volume( v2 );
  delete_volume( v1 );
  return 0;
}

int
test3(void)
{
  VIO_Volume v1;
  VIO_Volume v2;
  int sizes[VIO_MAX_DIMENSIONS] = { 1000, 1000, 10, 3, 3 };
  int read_sizes[VIO_MAX_DIMENSIONS] = { 1, 1, 1, 1, 1 };
  int i, j;

  v1 = create_volume(2, NULL, NC_DOUBLE, TRUE, 0, 1.0);
  if (v1 == NULL)
  {
    ERROR;
    return 1;
  }

  if ( get_volume_n_dimensions( v1 ) != 2 )
  {
    ERROR;
    return 1;
  }

  if (volume_is_alloced( v1 ))
  {
    ERROR;
    return 1;
  }

  get_volume_sizes( v1, read_sizes );

  if (get_volume_total_n_voxels( v1 ) != 0)
  {
    ERROR;
    return 1;
  }

  if (get_volume_data_type( v1 ) != VIO_DOUBLE )
  {
    ERROR;
    return 1;
  }

  set_volume_sizes( v1, sizes );

  if (get_volume_total_n_voxels( v1 ) != ((size_t) sizes[0] *
                                          (size_t) sizes[1]))

  {
    ERROR;
    return 1;
  }
  
  get_volume_sizes( v1, read_sizes );
  for (i = 0; i < get_volume_n_dimensions( v1 ); i++)
  {
    if (read_sizes[i] != sizes[i])
    {
      ERROR;
      return 1;
    }
  }

  alloc_volume_data( v1 );

  set_volume_n_dimensions( v1, 3 ); /* Should be ignored! */

  srand(0);
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      set_volume_voxel_value( v1, i, j, 0, 0, 0, (double)rand()/RAND_MAX );

  srand(0);
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
    {
      VIO_Real x1 = get_volume_voxel_value( v1, i, j, 0, 0, 0 ) ;
      /* There is no range check even if we set the max and min voxel
       * values, so the only constraint on values is the precision of
       * the unsigned short.
       */
      double x2 = (double) rand() / RAND_MAX;
      if ( x1 != x2 )
      {
        ERROR;
        fprintf(stderr, "%d %d: %f %f\n", i, j, x1, x2 );
          return 1;
        }
      }

  /* Copy to a less precise type. Perhaps we should disallow this.
   */
  v2 = copy_volume_new_type( v1, NC_FLOAT, TRUE );
  if (v2 == NULL)
  {
    ERROR;
    return 1;
  }

  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
    {
      VIO_Real x1 = get_volume_voxel_value( v1, i, j, 0, 0, 0 ) ;
      VIO_Real x2 = get_volume_voxel_value( v2, i, j, 0, 0, 0 ) ;

      if ( fabs(x1 - x2) > 1.0e-7 )
      {
        ERROR;
        fprintf(stderr, "%d %d: %f %f %g\n", i, j, x1, x2, fabs(x1 - x2) );
        return 1;
        }

      set_volume_voxel_value( v2, i, j, 0, 0, 0, x1 * 1000.0 );
    }

  
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
    {
      VIO_Real x1 = get_volume_voxel_value( v1, i, j, 0, 0, 0 ) ;
      VIO_Real x2 = get_volume_voxel_value( v2, i, j, 0, 0, 0 ) ;

      if ( fabs(x1 * 1000.0 - x2) > 1e-7 * 1000.0 )
      {
        ERROR;
        fprintf(stderr, "%d %d: %f %f\n", i, j, x1, x2 );
        return 1;
      }
    }

  delete_volume( v2 );
  delete_volume( v1 );

  return 0;
}

int
main(int argc, char **argv)
{
  int errors = 0;

  errors += test1();
  errors += test2();
  errors += test3();

  if (errors != 0)
  {
    fprintf(stderr, "%s exiting with %d error%s.\n", argv[0], errors, 
            errors == 1 ? "" : "s");
  }
  else
  {
    fprintf(stdout, "OK\n");
  }
  return errors;
}
