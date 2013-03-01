/* ----------------------------------------------------------------------------
@COPYRIGHT  :
              Copyright 1993,1994,1995 David MacDonald,
              McConnell Brain Imaging Centre,
              Montreal Neurological Institute, McGill University.
              Permission to use, copy, modify, and distribute this
              software and its documentation for any purpose and without
              fee is hereby granted, provided that the above copyright
              notice appear in all copies.  The author and McGill University
              make no representations about the suitability of this
              software for any purpose.  It is provided "as is" without
              express or implied warranty.
---------------------------------------------------------------------------- */

#include  <internal_volume_io.h>


VIOAPI  void  convert_voxels_to_values(
    Volume   volume,
    int      n_voxels,
    Real     voxels[],
    Real     values[] )
{
    int    v;
    Real   scale, trans;

    if( !volume->real_range_set )
    {
        if( voxels != values )
        {
            for_less( v, 0, n_voxels )
                values[v] = voxels[v];
        }
        return;
    }

    scale = volume->real_value_scale;
    trans = volume->real_value_translation;

    for_less( v, 0, n_voxels )
        values[v] = scale * voxels[v] + trans;
}

VIOAPI  void  get_volume_value_hyperslab(
    Volume   volume,
    int      v0,
    int      v1,
    int      v2,
    int      v3,
    int      v4,
    int      n0,
    int      n1,
    int      n2,
    int      n3,
    int      n4,
    Real     values[] )
{
    switch( get_volume_n_dimensions(volume) )
    {
    case 1:
        get_volume_value_hyperslab_1d( volume, v0, n0, values );
        break;
    case 2:
        get_volume_value_hyperslab_2d( volume, v0, v1, n0, n1, values );
        break;
    case 3:
        get_volume_value_hyperslab_3d( volume, v0, v1, v2, n0, n1, n2, values );
        break;
    case 4:
        get_volume_value_hyperslab_4d( volume, v0, v1, v2, v3,
                                       n0, n1, n2, n3, values );
        break;
    case 5:
        get_volume_value_hyperslab_5d( volume, v0, v1, v2, v3, v4,
                                       n0, n1, n2, n3, n4, values );
        break;
    }
}

VIOAPI  void  get_volume_value_hyperslab_5d(
    Volume   volume,
    int      v0,
    int      v1,
    int      v2,
    int      v3,
    int      v4,
    int      n0,
    int      n1,
    int      n2,
    int      n3,
    int      n4,
    Real     values[] )
{
    get_volume_voxel_hyperslab_5d( volume, v0, v1, v2, v3, v4,
                                   n0, n1, n2, n3, n4, values );

    convert_voxels_to_values( volume, n0 * n1 * n2 * n3 * n4, values, values );
}

VIOAPI  void  get_volume_value_hyperslab_4d(
    Volume   volume,
    int      v0,
    int      v1,
    int      v2,
    int      v3,
    int      n0,
    int      n1,
    int      n2,
    int      n3,
    Real     values[] )
{
    get_volume_voxel_hyperslab_4d( volume, v0, v1, v2, v3,
                                   n0, n1, n2, n3, values );

    convert_voxels_to_values( volume, n0 * n1 * n2 * n3, values, values );
}

VIOAPI  void  get_volume_value_hyperslab_3d(
    Volume   volume,
    int      v0,
    int      v1,
    int      v2,
    int      n0,
    int      n1,
    int      n2,
    Real     values[] )
{
    get_volume_voxel_hyperslab_3d( volume, v0, v1, v2, n0, n1, n2, values );

    convert_voxels_to_values( volume, n0 * n1 * n2, values, values );
}

VIOAPI  void  get_volume_value_hyperslab_2d(
    Volume   volume,
    int      v0,
    int      v1,
    int      n0,
    int      n1,
    Real     values[] )
{
    get_volume_voxel_hyperslab_2d( volume, v0, v1, n0, n1, values );

    convert_voxels_to_values( volume, n0 * n1, values, values );
}

VIOAPI  void  get_volume_value_hyperslab_1d(
    Volume   volume,
    int      v0,
    int      n0,
    Real     values[] )
{
    get_volume_voxel_hyperslab_1d( volume, v0, n0, values );

    convert_voxels_to_values( volume, n0, values, values );
}

static  void  slow_get_volume_voxel_hyperslab(
    Volume   volume,
    int      v0,
    int      v1,
    int      v2,
    int      v3,
    int      v4,
    int      n0,
    int      n1,
    int      n2,
    int      n3,
    int      n4,
    Real     values[] )
{
    int    i0, i1, i2, i3, i4, n_dims;

    n_dims = get_volume_n_dimensions( volume );

    if( n_dims < 5 )
        n4 = 1;
    if( n_dims < 4 )
        n3 = 1;
    if( n_dims < 3 )
        n2 = 1;
    if( n_dims < 2 )
        n1 = 1;
    if( n_dims < 1 )
        n0 = 1;

    for_less( i0, 0, n0 )
    for_less( i1, 0, n1 )
    for_less( i2, 0, n2 )
    for_less( i3, 0, n3 )
    for_less( i4, 0, n4 )
    {
        *values = get_volume_voxel_value( volume, v0 + i0, v1 + i1, v2 + i2,
                                          v3 + i3, v4 + i4 );
        ++values;
    }
}

static  Real  *int_to_real_conversion = NULL;

static  void  check_real_conversion_lookup( void )
{
    Real   min_value1, max_value1, min_value2, max_value2;
    long   i, long_min, long_max;

    if( int_to_real_conversion != NULL )
        return;

    get_type_range( UNSIGNED_SHORT, &min_value1, &max_value1 );
    get_type_range( SIGNED_SHORT, &min_value2, &max_value2 );

    long_min = (long) MIN( min_value1, min_value2 );
    long_max = (long) MAX( max_value1, max_value2 );

    ALLOC( int_to_real_conversion, long_max - long_min + 1 );
#ifndef  NO_DEBUG_ALLOC
    (void) unrecord_ptr_alloc_check( int_to_real_conversion,
                                     __FILE__, __LINE__ );
#endif

    int_to_real_conversion -= long_min;

    for_inclusive( i, long_min, long_max )
        int_to_real_conversion[i] = (Real) i;
}

VIOAPI  void  get_voxel_values_5d(
    Data_types  data_type,
    void        *void_ptr,
    int         steps[],
    int         counts[],
    Real        values[] )
{
    int              step0, step1, step2, step3, step4;
    int              i0, i1, i2, i3, i4;
    int              n0, n1, n2, n3, n4;
    unsigned  char   *unsigned_byte_ptr;
    signed  char     *signed_byte_ptr;
    unsigned  short  *unsigned_short_ptr;
    signed  short    *signed_short_ptr;
    unsigned  int    *unsigned_int_ptr;
    signed  int      *signed_int_ptr;
    float            *float_ptr;
    double           *double_ptr;

    n0 = counts[0];
    n1 = counts[1];
    n2 = counts[2];
    n3 = counts[3];
    n4 = counts[4];
    step0 = steps[0];
    step1 = steps[1];
    step2 = steps[2];
    step3 = steps[3];
    step4 = steps[4];
    step0 -= n1 * step1;
    step1 -= n2 * step2;
    step2 -= n3 * step3;
    step3 -= n4 * step4;

    check_real_conversion_lookup();

    switch( data_type )
    {
    case UNSIGNED_BYTE:
        ASSIGN_PTR(unsigned_byte_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        for_less( i4, 0, n4 )
                        {
                            *values = int_to_real_conversion[
                                                (long) *unsigned_byte_ptr];
                            ++values;
                            unsigned_byte_ptr += step4;
                        }
                        unsigned_byte_ptr += step3;
                    }
                    unsigned_byte_ptr += step2;
                }
                unsigned_byte_ptr += step1;
            }
            unsigned_byte_ptr += step0;
        }
        break;

    case SIGNED_BYTE:
        ASSIGN_PTR(signed_byte_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        for_less( i4, 0, n4 )
                        {
                            *values = int_to_real_conversion[
                                                (long) *signed_byte_ptr];
                            ++values;
                            signed_byte_ptr += step4;
                        }
                        signed_byte_ptr += step3;
                    }
                    signed_byte_ptr += step2;
                }
                signed_byte_ptr += step1;
            }
            signed_byte_ptr += step0;
        }
        break;

    case UNSIGNED_SHORT:
        ASSIGN_PTR(unsigned_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        for_less( i4, 0, n4 )
                        {
                            *values = int_to_real_conversion[
                                               (long) *unsigned_short_ptr];
                            ++values;
                            unsigned_short_ptr += step4;
                        }
                        unsigned_short_ptr += step3;
                    }
                    unsigned_short_ptr += step2;
                }
                unsigned_short_ptr += step1;
            }
            unsigned_short_ptr += step0;
        }
        break;

    case SIGNED_SHORT:
        ASSIGN_PTR(signed_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        for_less( i4, 0, n4 )
                        {
                            *values = int_to_real_conversion[
                                                (long) *signed_short_ptr];
                            ++values;
                            signed_short_ptr += step4;
                        }
                        signed_short_ptr += step3;
                    }
                    signed_short_ptr += step2;
                }
                signed_short_ptr += step1;
            }
            signed_short_ptr += step0;
        }
        break;

    case UNSIGNED_INT:
        ASSIGN_PTR(unsigned_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        for_less( i4, 0, n4 )
                        {
                            *values = (Real) *unsigned_int_ptr;
                            ++values;
                            unsigned_int_ptr += step4;
                        }
                        unsigned_int_ptr += step3;
                    }
                    unsigned_int_ptr += step2;
                }
                unsigned_int_ptr += step1;
            }
            unsigned_int_ptr += step0;
        }
        break;

    case SIGNED_INT:
        ASSIGN_PTR(signed_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        for_less( i4, 0, n4 )
                        {
                            *values = (Real) *signed_int_ptr;
                            ++values;
                            signed_int_ptr += step4;
                        }
                        signed_int_ptr += step3;
                    }
                    signed_int_ptr += step2;
                }
                signed_int_ptr += step1;
            }
            signed_int_ptr += step0;
        }
        break;

    case FLOAT:
        ASSIGN_PTR(float_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        for_less( i4, 0, n4 )
                        {
                            *values = (Real) *float_ptr;
                            ++values;
                            float_ptr += step4;
                        }
                        float_ptr += step3;
                    }
                    float_ptr += step2;
                }
                float_ptr += step1;
            }
            float_ptr += step0;
        }
        break;

    case DOUBLE:
        ASSIGN_PTR(double_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        for_less( i4, 0, n4 )
                        {
                            *values = (Real) *double_ptr;
                            ++values;
                            double_ptr += step4;
                        }
                        double_ptr += step3;
                    }
                    double_ptr += step2;
                }
                double_ptr += step1;
            }
            double_ptr += step0;
        }
        break;
    }
}

VIOAPI  void  get_voxel_values_4d(
    Data_types  data_type,
    void        *void_ptr,
    int         steps[],
    int         counts[],
    Real        values[] )
{
    int              step0, step1, step2, step3;
    int              i0, i1, i2, i3;
    int              n0, n1, n2, n3;
    unsigned  char   *unsigned_byte_ptr;
    signed  char     *signed_byte_ptr;
    unsigned  short  *unsigned_short_ptr;
    signed  short    *signed_short_ptr;
    unsigned  int    *unsigned_int_ptr;
    signed  int      *signed_int_ptr;
    float            *float_ptr;
    double           *double_ptr;

    n0 = counts[0];
    n1 = counts[1];
    n2 = counts[2];
    n3 = counts[3];
    step0 = steps[0];
    step1 = steps[1];
    step2 = steps[2];
    step3 = steps[3];
    step0 -= n1 * step1;
    step1 -= n2 * step2;
    step2 -= n3 * step3;
    check_real_conversion_lookup();

    switch( data_type )
    {
    case UNSIGNED_BYTE:
        ASSIGN_PTR(unsigned_byte_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        *values = int_to_real_conversion[
                                                (long) *unsigned_byte_ptr];
                        ++values;
                        unsigned_byte_ptr += step3;
                    }
                    unsigned_byte_ptr += step2;
                }
                unsigned_byte_ptr += step1;
            }
            unsigned_byte_ptr += step0;
        }
        break;

    case SIGNED_BYTE:
        ASSIGN_PTR(signed_byte_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        *values = int_to_real_conversion[(long) *signed_byte_ptr];
                        ++values;
                        signed_byte_ptr += step3;
                    }
                    signed_byte_ptr += step2;
                }
                signed_byte_ptr += step1;
            }
            signed_byte_ptr += step0;
        }
        break;

    case UNSIGNED_SHORT:
        ASSIGN_PTR(unsigned_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        *values = int_to_real_conversion[
                                                (long) *unsigned_short_ptr];
                        ++values;
                        unsigned_short_ptr += step3;
                    }
                    unsigned_short_ptr += step2;
                }
                unsigned_short_ptr += step1;
            }
            unsigned_short_ptr += step0;
        }
        break;

    case SIGNED_SHORT:
        ASSIGN_PTR(signed_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        *values = int_to_real_conversion[
                                                (long) *signed_short_ptr];
                        ++values;
                        signed_short_ptr += step3;
                    }
                    signed_short_ptr += step2;
                }
                signed_short_ptr += step1;
            }
            signed_short_ptr += step0;
        }
        break;

    case UNSIGNED_INT:
        ASSIGN_PTR(unsigned_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        *values = (Real) *unsigned_int_ptr;
                        ++values;
                        unsigned_int_ptr += step3;
                    }
                    unsigned_int_ptr += step2;
                }
                unsigned_int_ptr += step1;
            }
            unsigned_int_ptr += step0;
        }
        break;

    case SIGNED_INT:
        ASSIGN_PTR(signed_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        *values = (Real) *signed_int_ptr;
                        ++values;
                        signed_int_ptr += step3;
                    }
                    signed_int_ptr += step2;
                }
                signed_int_ptr += step1;
            }
            signed_int_ptr += step0;
        }
        break;

    case FLOAT:
        ASSIGN_PTR(float_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        *values = (Real) *float_ptr;
                        ++values;
                        float_ptr += step3;
                    }
                    float_ptr += step2;
                }
                float_ptr += step1;
            }
            float_ptr += step0;
        }
        break;

    case DOUBLE:
        ASSIGN_PTR(double_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    for_less( i3, 0, n3 )
                    {
                        *values = (Real) *double_ptr;
                        ++values;
                        double_ptr += step3;
                    }
                    double_ptr += step2;
                }
                double_ptr += step1;
            }
            double_ptr += step0;
        }
        break;
    }
}

VIOAPI  void  get_voxel_values_3d(
    Data_types  data_type,
    void        *void_ptr,
    int         steps[],
    int         counts[],
    Real        values[] )
{
    int              step0, step1, step2;
    int              i0, i1, i2;
    int              n0, n1, n2;
    unsigned  char   *unsigned_byte_ptr;
    signed  char     *signed_byte_ptr;
    unsigned  short  *unsigned_short_ptr;
    signed  short    *signed_short_ptr;
    unsigned  int    *unsigned_int_ptr;
    signed  int      *signed_int_ptr;
    float            *float_ptr;
    double           *double_ptr;

    check_real_conversion_lookup();

    n0 = counts[0];
    n1 = counts[1];
    n2 = counts[2];
    step0 = steps[0];
    step1 = steps[1];
    step2 = steps[2];

    /*--- special case, for speed */

    if( data_type == UNSIGNED_BYTE && n0 == 2 && n1 == 2 && n2 == 2 &&
        step2 == 1 )
    {
        step0 -= step1 + 1;
        step1 -= 1;

        ASSIGN_PTR(unsigned_byte_ptr) = void_ptr;

        values[0] = int_to_real_conversion[(unsigned long) *unsigned_byte_ptr];
        ++unsigned_byte_ptr;
        values[1] = int_to_real_conversion[(unsigned long) *unsigned_byte_ptr];
        unsigned_byte_ptr += step1;
        values[2] = int_to_real_conversion[(unsigned long) *unsigned_byte_ptr];
        ++unsigned_byte_ptr;
        values[3] = int_to_real_conversion[(unsigned long) *unsigned_byte_ptr];
        unsigned_byte_ptr += step0;

        values[4] = int_to_real_conversion[(unsigned long) *unsigned_byte_ptr];
        ++unsigned_byte_ptr;
        values[5] = int_to_real_conversion[(unsigned long) *unsigned_byte_ptr];
        unsigned_byte_ptr += step1;
        values[6] = int_to_real_conversion[(unsigned long) *unsigned_byte_ptr];
        ++unsigned_byte_ptr;
        values[7] = int_to_real_conversion[(unsigned long) *unsigned_byte_ptr];

        return;
    }

    step0 -= n1 * step1;
    step1 -= n2 * step2;

    switch( data_type )
    {
    case UNSIGNED_BYTE:
        ASSIGN_PTR(unsigned_byte_ptr) = void_ptr;

        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    *values = int_to_real_conversion[(long) *unsigned_byte_ptr];
                    ++values;
                    unsigned_byte_ptr += step2;
                }
                unsigned_byte_ptr += step1;
            }
            unsigned_byte_ptr += step0;
        }
        break;

    case SIGNED_BYTE:
        ASSIGN_PTR(signed_byte_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    *values = int_to_real_conversion[(long) *signed_byte_ptr];
                    ++values;
                    signed_byte_ptr += step2;
                }
                signed_byte_ptr += step1;
            }
            signed_byte_ptr += step0;
        }
        break;

    case UNSIGNED_SHORT:
        ASSIGN_PTR(unsigned_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    *values = int_to_real_conversion[(long) *unsigned_short_ptr];
                    ++values;
                    unsigned_short_ptr += step2;
                }
                unsigned_short_ptr += step1;
            }
            unsigned_short_ptr += step0;
        }
        break;

    case SIGNED_SHORT:
        ASSIGN_PTR(signed_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    *values = int_to_real_conversion[(long) *signed_short_ptr];
                    ++values;
                    signed_short_ptr += step2;
                }
                signed_short_ptr += step1;
            }
            signed_short_ptr += step0;
        }
        break;

    case UNSIGNED_INT:
        ASSIGN_PTR(unsigned_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    *values = (Real) *unsigned_int_ptr;
                    ++values;
                    unsigned_int_ptr += step2;
                }
                unsigned_int_ptr += step1;
            }
            unsigned_int_ptr += step0;
        }
        break;

    case SIGNED_INT:
        ASSIGN_PTR(signed_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    *values = (Real) *signed_int_ptr;
                    ++values;
                    signed_int_ptr += step2;
                }
                signed_int_ptr += step1;
            }
            signed_int_ptr += step0;
        }
        break;

    case FLOAT:
        ASSIGN_PTR(float_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    *values = (Real) *float_ptr;
                    ++values;
                    float_ptr += step2;
                }
                float_ptr += step1;
            }
            float_ptr += step0;
        }
        break;

    case DOUBLE:
        ASSIGN_PTR(double_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                for_less( i2, 0, n2 )
                {
                    *values = (Real) *double_ptr;
                    ++values;
                    double_ptr += step2;
                }
                double_ptr += step1;
            }
            double_ptr += step0;
        }
        break;
    }
}

VIOAPI  void  get_voxel_values_2d(
    Data_types  data_type,
    void        *void_ptr,
    int         steps[],
    int         counts[],
    Real        values[] )
{
    int              step0, step1;
    int              i0, i1;
    int              n0, n1;
    unsigned  char   *unsigned_byte_ptr;
    signed  char     *signed_byte_ptr;
    unsigned  short  *unsigned_short_ptr;
    signed  short    *signed_short_ptr;
    unsigned  int    *unsigned_int_ptr;
    signed  int      *signed_int_ptr;
    float            *float_ptr;
    double           *double_ptr;

    n0 = counts[0];
    n1 = counts[1];
    step0 = steps[0];
    step1 = steps[1];
    step0 -= n1 * step1;
    check_real_conversion_lookup();

    switch( data_type )
    {
    case UNSIGNED_BYTE:
        ASSIGN_PTR(unsigned_byte_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                *values = int_to_real_conversion[(long) *unsigned_byte_ptr];
                ++values;
                unsigned_byte_ptr += step1;
            }
            unsigned_byte_ptr += step0;
        }
        break;

    case SIGNED_BYTE:
        ASSIGN_PTR(signed_byte_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                *values = int_to_real_conversion[(long) *signed_byte_ptr];
                ++values;
                signed_byte_ptr += step1;
            }
            signed_byte_ptr += step0;
        }
        break;

    case UNSIGNED_SHORT:
        ASSIGN_PTR(unsigned_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                *values = int_to_real_conversion[(long) *unsigned_short_ptr];
                ++values;
                unsigned_short_ptr += step1;
            }
            unsigned_short_ptr += step0;
        }
        break;

    case SIGNED_SHORT:
        ASSIGN_PTR(signed_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                *values = int_to_real_conversion[(long) *signed_short_ptr];
                ++values;
                signed_short_ptr += step1;
            }
            signed_short_ptr += step0;
        }
        break;

    case UNSIGNED_INT:
        ASSIGN_PTR(unsigned_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                *values = (Real) *unsigned_int_ptr;
                ++values;
                unsigned_int_ptr += step1;
            }
            unsigned_int_ptr += step0;
        }
        break;

    case SIGNED_INT:
        ASSIGN_PTR(signed_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                *values = (Real) *signed_int_ptr;
                ++values;
                signed_int_ptr += step1;
            }
            signed_int_ptr += step0;
        }
        break;

    case FLOAT:
        ASSIGN_PTR(float_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                *values = (Real) *float_ptr;
                ++values;
                float_ptr += step1;
            }
            float_ptr += step0;
        }
        break;

    case DOUBLE:
        ASSIGN_PTR(double_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            for_less( i1, 0, n1 )
            {
                *values = (Real) *double_ptr;
                ++values;
                double_ptr += step1;
            }
            double_ptr += step0;
        }
        break;
    }
}

VIOAPI  void  get_voxel_values_1d(
    Data_types  data_type,
    void        *void_ptr,
    int         step0,
    int         n0,
    Real        values[] )
{
    int              i0;
    unsigned  char   *unsigned_byte_ptr;
    signed  char     *signed_byte_ptr;
    unsigned  short  *unsigned_short_ptr;
    signed  short    *signed_short_ptr;
    unsigned  int    *unsigned_int_ptr;
    signed  int      *signed_int_ptr;
    float            *float_ptr;
    double           *double_ptr;

    check_real_conversion_lookup();

    switch( data_type )
    {
    case UNSIGNED_BYTE:
        ASSIGN_PTR(unsigned_byte_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            *values = int_to_real_conversion[(long) *unsigned_byte_ptr];
            ++values;
            unsigned_byte_ptr += step0;
        }
        break;

    case SIGNED_BYTE:
        ASSIGN_PTR(signed_byte_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            *values = int_to_real_conversion[(long) *signed_byte_ptr];
            ++values;
            signed_byte_ptr += step0;
        }
        break;

    case UNSIGNED_SHORT:
        ASSIGN_PTR(unsigned_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            *values = int_to_real_conversion[(long) *unsigned_short_ptr];
            ++values;
            unsigned_short_ptr += step0;
        }
        break;

    case SIGNED_SHORT:
        ASSIGN_PTR(signed_short_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            *values = int_to_real_conversion[(long) *signed_short_ptr];
            ++values;
            signed_short_ptr += step0;
        }
        break;

    case UNSIGNED_INT:
        ASSIGN_PTR(unsigned_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            *values = (Real) *unsigned_int_ptr;
            ++values;
            unsigned_int_ptr += step0;
        }
        break;

    case SIGNED_INT:
        ASSIGN_PTR(signed_int_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            *values = (Real) *signed_int_ptr;
            ++values;
            signed_int_ptr += step0;
        }
        break;

    case FLOAT:
        ASSIGN_PTR(float_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            *values = (Real) *float_ptr;
            ++values;
            float_ptr += step0;
        }
        break;

    case DOUBLE:
        ASSIGN_PTR(double_ptr) = void_ptr;
        for_less( i0, 0, n0 )
        {
            *values = (Real) *double_ptr;
            ++values;
            double_ptr += step0;
        }
        break;
    }
}

static  void  get_voxel_values(
    Volume   volume,
    void     *void_ptr,
    int      n_dims,
    int      steps[],
    int      counts[],
    Real     values[] )
{
    Data_types  data_type;

    data_type = get_volume_data_type( volume );
    switch( n_dims )
    {
    case 0:
        get_voxel_values_1d( data_type, void_ptr, 1, 1, values );
        break;
    case 1:
        get_voxel_values_1d( data_type, void_ptr, steps[0], counts[0], values );
        break;
    case 2:
        get_voxel_values_2d( data_type, void_ptr, steps, counts, values );
        break;
    case 3:
        get_voxel_values_3d( data_type, void_ptr, steps, counts, values );
        break;
    case 4:
        get_voxel_values_4d( data_type, void_ptr, steps, counts, values );
        break;
    case 5:
        get_voxel_values_5d( data_type, void_ptr, steps, counts, values );
        break;
    }
}

VIOAPI  void  get_volume_voxel_hyperslab_5d(
    Volume   volume,
    int      v0,
    int      v1,
    int      v2,
    int      v3,
    int      v4,
    int      n0,
    int      n1,
    int      n2,
    int      n3,
    int      n4,
    Real     values[] )
{
    int         steps[MAX_DIMENSIONS];
    int         counts[MAX_DIMENSIONS];
    int         sizes[MAX_DIMENSIONS];
    int         dim, stride;
    void        *void_ptr;

    if( volume->is_cached_volume )
    {
        slow_get_volume_voxel_hyperslab( volume, v0, v1, v2, v3, v4,
                                         n0, n1, n2, n3, n4, values );
        return;
    }

    get_volume_sizes( volume, sizes );

    GET_MULTIDIM_PTR_5D( void_ptr, volume->array, v0, v1, v2, v3, v4 )

    stride = 1;
    dim = 5;

    if( n4 > 1 )
    {
        --dim;
        counts[dim] = n4;
        steps[dim] = stride;
    }
    stride *= sizes[4];

    if( n3 > 1 )
    {
        --dim;
        counts[dim] = n3;
        steps[dim] = stride;
    }
    stride *= sizes[3];

    if( n2 > 1 )
    {
        --dim;
        counts[dim] = n2;
        steps[dim] = stride;
    }
    stride *= sizes[2];

    if( n1 > 1 )
    {
        --dim;
        counts[dim] = n1;
        steps[dim] = stride;
    }
    stride *= sizes[1];

    if( n0 > 1 )
    {
        --dim;
        counts[dim] = n0;
        steps[dim] = stride;
    }

    get_voxel_values( volume, void_ptr, 5 - dim, &steps[dim], &counts[dim],
                      values );
}

VIOAPI  void  get_volume_voxel_hyperslab_4d(
    Volume   volume,
    int      v0,
    int      v1,
    int      v2,
    int      v3,
    int      n0,
    int      n1,
    int      n2,
    int      n3,
    Real     values[] )
{
    int         steps[MAX_DIMENSIONS];
    int         counts[MAX_DIMENSIONS];
    int         sizes[MAX_DIMENSIONS];
    int         dim, stride;
    void        *void_ptr;

    if( volume->is_cached_volume )
    {
        slow_get_volume_voxel_hyperslab( volume, v0, v1, v2, v3, 0,
                                         n0, n1, n2, n3, 0, values );
        return;
    }

    get_volume_sizes( volume, sizes );

    GET_MULTIDIM_PTR_4D( void_ptr, volume->array, v0, v1, v2, v3 )

    stride = 1;
    dim = 4;

    if( n3 > 1 )
    {
        --dim;
        counts[dim] = n3;
        steps[dim] = stride;
    }
    stride *= sizes[3];

    if( n2 > 1 )
    {
        --dim;
        counts[dim] = n2;
        steps[dim] = stride;
    }
    stride *= sizes[2];

    if( n1 > 1 )
    {
        --dim;
        counts[dim] = n1;
        steps[dim] = stride;
    }
    stride *= sizes[1];

    if( n0 > 1 )
    {
        --dim;
        counts[dim] = n0;
        steps[dim] = stride;
    }

    get_voxel_values( volume, void_ptr, 4 - dim, &steps[dim], &counts[dim],
                      values );
}

VIOAPI  void  get_volume_voxel_hyperslab_3d(
    Volume   volume,
    int      v0,
    int      v1,
    int      v2,
    int      n0,
    int      n1,
    int      n2,
    Real     values[] )
{
    int         steps[MAX_DIMENSIONS];
    int         counts[MAX_DIMENSIONS];
    int         sizes[MAX_DIMENSIONS];
    int         dim, stride;
    void        *void_ptr;

    if( volume->is_cached_volume )
    {
        slow_get_volume_voxel_hyperslab( volume, v0, v1, v2, 0, 0,
                                         n0, n1, n2, 0, 0, values );
        return;
    }

    get_volume_sizes( volume, sizes );

    GET_MULTIDIM_PTR_3D( void_ptr, volume->array, v0, v1, v2 )

    stride = 1;
    dim = 3;

    if( n2 > 1 )
    {
        --dim;
        counts[dim] = n2;
        steps[dim] = stride;
    }
    stride *= sizes[2];

    if( n1 > 1 )
    {
        --dim;
        counts[dim] = n1;
        steps[dim] = stride;
    }
    stride *= sizes[1];

    if( n0 > 1 )
    {
        --dim;
        counts[dim] = n0;
        steps[dim] = stride;
    }

    get_voxel_values( volume, void_ptr, 3 - dim, &steps[dim], &counts[dim],
                      values );
}

VIOAPI  void  get_volume_voxel_hyperslab_2d(
    Volume   volume,
    int      v0,
    int      v1,
    int      n0,
    int      n1,
    Real     values[] )
{
    int         steps[MAX_DIMENSIONS];
    int         counts[MAX_DIMENSIONS];
    int         sizes[MAX_DIMENSIONS];
    int         dim, stride;
    void        *void_ptr;

    if( volume->is_cached_volume )
    {
        slow_get_volume_voxel_hyperslab( volume, v0, v1, 0, 0, 0,
                                         n0, n1, 0, 0, 0, values );
        return;
    }

    get_volume_sizes( volume, sizes );

    GET_MULTIDIM_PTR_2D( void_ptr, volume->array, v0, v1 )

    stride = 1;
    dim = 2;

    if( n1 > 1 )
    {
        --dim;
        counts[dim] = n1;
        steps[dim] = stride;
    }
    stride *= sizes[1];

    if( n0 > 1 )
    {
        --dim;
        counts[dim] = n0;
        steps[dim] = stride;
    }

    get_voxel_values( volume, void_ptr, 2 - dim, &steps[dim], &counts[dim],
                      values );
}

VIOAPI  void  get_volume_voxel_hyperslab_1d(
    Volume   volume,
    int      v0,
    int      n0,
    Real     values[] )
{
    int         steps[MAX_DIMENSIONS];
    int         counts[MAX_DIMENSIONS];
    int         sizes[MAX_DIMENSIONS];
    int         dim;
    void        *void_ptr;

    if( volume->is_cached_volume )
    {
        slow_get_volume_voxel_hyperslab( volume, v0, 0, 0, 0, 0,
                                         n0, 0, 0, 0, 0, values );
        return;
    }

    get_volume_sizes( volume, sizes );

    GET_MULTIDIM_PTR_1D( void_ptr, volume->array, v0 )

    dim = 1;

    if( n0 > 1 )
    {
        --dim;
        counts[dim] = n0;
        steps[dim] = 1;
    }

    get_voxel_values( volume, void_ptr, 1 - dim, &steps[dim], &counts[dim],
                      values );
}

VIOAPI  void  get_volume_voxel_hyperslab(
    Volume   volume,
    int      v0,
    int      v1,
    int      v2,
    int      v3,
    int      v4,
    int      n0,
    int      n1,
    int      n2,
    int      n3,
    int      n4,
    Real     voxels[] )
{
    switch( get_volume_n_dimensions(volume) )
    {
    case 1:
        get_volume_voxel_hyperslab_1d( volume, v0, n0, voxels );
        break;
    case 2:
        get_volume_voxel_hyperslab_2d( volume, v0, v1, n0, n1, voxels );
        break;
    case 3:
        get_volume_voxel_hyperslab_3d( volume, v0, v1, v2, n0, n1, n2, voxels );
        break;
    case 4:
        get_volume_voxel_hyperslab_4d( volume, v0, v1, v2, v3,
                                       n0, n1, n2, n3, voxels );
        break;
    case 5:
        get_volume_voxel_hyperslab_5d( volume, v0, v1, v2, v3, v4,
                                       n0, n1, n2, n3, n4, voxels );
        break;
    }
}

#ifdef HAVE_MINC1

#endif /*HAVE_MINC1*/