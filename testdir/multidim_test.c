#include <volume_io.h>

#define TEST1_N_DIMENSIONS 3

static int
test1(void)
{
  VIO_multidim_array array = {0};
  int sizes[TEST1_N_DIMENSIONS] = { 1, 1, 1 };
  int read_sizes[VIO_MAX_DIMENSIONS] = { 5, 5, 5, 5, 5 };
  int i;
  int value;

  printf("test1()... ");

  if (multidim_array_is_alloced(&array))
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  create_empty_multidim_array(&array, TEST1_N_DIMENSIONS, VIO_UNSIGNED_BYTE);
  if (multidim_array_is_alloced(&array))
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  set_multidim_sizes(&array, sizes);
  if (multidim_array_is_alloced(&array))
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  alloc_multidim_array(&array);

  if (!multidim_array_is_alloced(&array))
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }

  SET_MULTIDIM(array, 0, 0, 0, 0, 0, 10);
  GET_MULTIDIM(value, (int), array, 0, 0, 0, 0, 0);
  if (value != 10)
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  SET_MULTIDIM(array, 0, 0, 0, 0, 0, 20);
  GET_MULTIDIM(value, (int), array, 0, 0, 0, 0, 0);
  if (value != 20)
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  if (get_multidim_n_dimensions(&array) != TEST1_N_DIMENSIONS)
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  if (get_multidim_data_type(&array) != VIO_UNSIGNED_BYTE)
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  get_multidim_sizes(&array, read_sizes);
  for (i = 0; i < TEST1_N_DIMENSIONS; i++)
  {
    if (read_sizes[i] != 1)
    {
      printf("%s: %d\n", __func__, __LINE__);
      return 1;
    }
  }
  while (i < VIO_MAX_DIMENSIONS)
  {
    if (read_sizes[i] != 5)
    {
      printf("%s: %d\n", __func__, __LINE__);
      return 1;
    }
    i++;
  }
  delete_multidim_array(&array);
  printf("OK\n");
  return 0;
}

#define TEST2_N_DIMENSIONS 5

static int
test2(void)
{
  VIO_multidim_array array = { 0 };
  int sizes[TEST2_N_DIMENSIONS] = { 61, 67, 73, 51, 3 };
  int read_sizes[VIO_MAX_DIMENSIONS] = { 5, 5, 5, 5, 5 };
  int value;
  int i, j, k, t, v;
  int n = 0;

  printf("test2()... ");

  if (multidim_array_is_alloced(&array))
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  create_empty_multidim_array(&array, 1, VIO_UNSIGNED_BYTE);
  if (multidim_array_is_alloced(&array))
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }

  set_multidim_data_type(&array, VIO_SIGNED_INT);
  
  set_multidim_n_dimensions(&array, TEST2_N_DIMENSIONS);

  set_multidim_sizes(&array, sizes);

  if (multidim_array_is_alloced(&array))
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }

  alloc_multidim_array(&array);

  if (!multidim_array_is_alloced(&array))
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }

  if (get_multidim_n_dimensions(&array) != TEST2_N_DIMENSIONS)
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  if (get_multidim_data_type(&array) != VIO_SIGNED_INT)
  {
    printf("%s: %d\n", __func__, __LINE__);
    return 1;
  }
  get_multidim_sizes(&array, read_sizes);
  for (i = 0; i < TEST2_N_DIMENSIONS; i++)
  {
    if (read_sizes[i] != sizes[i])
    {
      printf("%s: %d\n", __func__, __LINE__);
      return 1;
    }
  }

  srand(0);
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++) 
        for (t = 0; t < sizes[3]; t++)
          for (v = 0; v < sizes[4]; v++)
            SET_MULTIDIM(array, i, j, k, t, v, rand());

  srand(0);
  for (i = 0; i < sizes[0]; i++)
    for (j = 0; j < sizes[1]; j++) 
      for (k = 0; k < sizes[2]; k++) 
        for (t = 0; t < sizes[3]; t++)
          for (v = 0; v < sizes[4]; v++)
          {
            GET_MULTIDIM(value, (int), array, i, j, k, t, v);
            if (value != rand())
            {
              printf("%s: %d\n", __func__, __LINE__);
              return 1;
            }
            n++;
          }

  printf("%d values checked... ", n);
  delete_multidim_array(&array);
  printf("OK\n");
  return 0;
}

static int
test3(VIO_Data_types data_type, int n_dimensions, int sizes[])
{
  VIO_multidim_array array = {0};
  int index[VIO_MAX_DIMENSIONS];
  int i;
  int n;
  int value;
  int carry;

  printf("%s(%d, %d, [", __func__, data_type, n_dimensions);
  for (i = 0; i < n_dimensions; i++) {
    printf("%d", sizes[i]);
    if (i < n_dimensions - 1)
      printf(", ");
  }
  printf("])\n");

  create_multidim_array(&array, n_dimensions, sizes, data_type);

  if (!multidim_array_is_alloced(&array))
  {
    fprintf(stderr, "Failure\n");
  }

  for (i = 0; i < n_dimensions; i++) {
    index[i] = 0;
  }

  n = 1;
  do {
    carry = 1;
    value = n++;

    SET_MULTIDIM(array, index[0], index[1], index[2], index[3], index[4],
                 value);

    for (i = 0; carry && i < n_dimensions; i++) {
      if (index[i] + carry >= sizes[i]) {
        index[i] = 0;
      }
      else {
        index[i] += carry;
        carry = 0;
      }
    }
  } while (!carry);

  printf("Set %d values... ", n - 1);

  n = 1;
  for (i = 0; i < n_dimensions; i++)
    index[i] = 0;

  do {
    carry = 1;

    GET_MULTIDIM(value, (int), array, index[0], index[1], index[2], index[3],
                 index[4]);
    if (value != n) {
      printf("ERROR %d %d %d %d %d\n", value, n, index[0], index[1], index[2]);
    }
    n++;
    for (i = 0; carry && i < n_dimensions; i++) {
      if (index[i] + carry >= sizes[i]) {
        index[i] = 0;
      }
      else {
        index[i] += carry;
        carry = 0;
      }
    }
  } while (!carry);

  printf("OK\n");
  return 0;
}

int
main(int argc, char **argv)
{
  // these sizes affect which types we can use lower down.
  int sizes[] = { 13, 16, 17, 11, 23 };
  int errors = 0;

  printf("Testing multidim_arrays...\n");

  errors += test1();
  errors += test2();

  errors += test3(VIO_SIGNED_BYTE, 1, sizes);
  errors += test3(VIO_FLOAT, 1, sizes);

  errors += test3(VIO_UNSIGNED_BYTE, 2, sizes);
  errors += test3(VIO_SIGNED_INT, 2, sizes);

  errors += test3(VIO_SIGNED_SHORT, 3, sizes);
  errors += test3(VIO_UNSIGNED_SHORT, 3, sizes);

  errors += test3(VIO_UNSIGNED_SHORT, 4, sizes);
  errors += test3(VIO_SIGNED_INT, 4, sizes);

  errors += test3(VIO_UNSIGNED_INT, 5, sizes);
  errors += test3(VIO_DOUBLE, 5, sizes);

  if (errors == 0)
    printf("All tests completed without errors.\n");
  else 
    printf("Detected %d error%s.\n", errors, errors == 1 ? "" : "s");
  return errors;
}
