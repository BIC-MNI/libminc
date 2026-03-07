/**
 * \file minc2-scalar-test.c
 * \brief Test scalar dataset creation in MINC2 files
 *
 * This test verifies that scalar datasets (0-dimensional variables) 
 * can be created and have attributes attached to them.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minc2.h"

static int test_scalar_creation(const char *filename);
static int test_scalar_with_type(const char *filename);

int main(int argc, char **argv)
{
  int result;
  const char *filename;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return 1;
  }
  filename = argv[1];

  printf("Starting MINC2 scalar dataset tests with file: %s\n", filename);

  result = test_scalar_creation(filename);
  if (result != 0) {
    fprintf(stderr, "FAILED: test_scalar_creation\n");
    return 1;
  }

  result = test_scalar_with_type(filename);
  if (result != 0) {
    fprintf(stderr, "FAILED: test_scalar_with_type\n");
    return 1;
  }

  printf("SUCCESS: All scalar dataset tests passed\n");
  return 0;
}

/**
 * Test basic scalar dataset creation
 */
static int test_scalar_creation(const char *filename)
{
  mihandle_t volume;
  mihandle_t volume_read;
  int result;
  char attr_value[256];
  mitype_t attr_type;
  size_t attr_length;
  midimhandle_t hdim[1];
  misize_t start[1], count[1];
  short data[10];
  int i;

  printf("  Test 1: Creating scalar dataset with micreate_scalar_dataset...\n");

  /* Create a dimension for the volume */
  result = micreate_dimension("xspace", MI_DIMCLASS_SPATIAL, 
                              MI_DIMATTR_REGULARLY_SAMPLED, 10, &hdim[0]);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to create dimension\n");
    return 1;
  }

  /* Create volume with 1 dimension */
  result = micreate_volume(filename, 1, hdim, MI_TYPE_SHORT, 
                           MI_CLASS_REAL, NULL, &volume);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to create volume\n");
    return 1;
  }

  /* Create volume image */
  result = micreate_volume_image(volume);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to create volume image\n");
    miclose_volume(volume);
    return 1;
  }

  /* Write some dummy data */
  for (i = 0; i < 10; i++) {
    data[i] = (short)i;
  }
  start[0] = 0;
  count[0] = 10;
  result = miset_voxel_value_hyperslab(volume, MI_TYPE_SHORT, start, count, data);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to write voxel data\n");
    miclose_volume(volume);
    return 1;
  }

  /* Create a scalar dataset using the new API */
  result = micreate_scalar_dataset(volume, "test_scalar_var");
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to create scalar dataset (error code %d)\n", result);
    miclose_volume(volume);
    return 1;
  }

  /* Add an attribute to the scalar dataset */
  result = miset_attr_values(volume, MI_TYPE_STRING, 
                             "test_scalar_var", "test_attr",
                             strlen("hello") + 1, "hello");
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to set attribute on scalar\n");
    miclose_volume(volume);
    return 1;
  }

  result = miclose_volume(volume);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to close volume\n");
    return 1;
  }

  /* Reopen the file and verify the attribute was written correctly */
  printf("    Reopening file to verify attribute...\n");
  result = miopen_volume(filename, MI2_OPEN_READ, &volume_read);
  if (result < 0) {
    fprintf(stderr, "    Failed to reopen volume for reading\n");
    return 1;
  }

  /* Check attribute type */
  result = miget_attr_type(volume_read, "test_scalar_var", "test_attr", &attr_type);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to get attribute type from reopened file\n");
    miclose_volume(volume_read);
    return 1;
  }
  if (attr_type != MI_TYPE_STRING) {
    fprintf(stderr, "    Attribute type mismatch: expected %d, got %d\n", 
            MI_TYPE_STRING, attr_type);
    miclose_volume(volume_read);
    return 1;
  }

  /* Check attribute length */
  result = miget_attr_length(volume_read, "test_scalar_var", "test_attr", &attr_length);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to get attribute length from reopened file\n");
    miclose_volume(volume_read);
    return 1;
  }
  if (attr_length != strlen("hello") + 1) {
    fprintf(stderr, "    Attribute length mismatch: expected %zu, got %zu\n", 
            strlen("hello") + 1, attr_length);
    miclose_volume(volume_read);
    return 1;
  }

  /* Check attribute value */
  result = miget_attr_values(volume_read, MI_TYPE_STRING, 
                             "test_scalar_var", "test_attr",
                             sizeof(attr_value), attr_value);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to get attribute value from reopened file\n");
    miclose_volume(volume_read);
    return 1;
  }
  if (strcmp(attr_value, "hello") != 0) {
    fprintf(stderr, "    Attribute value mismatch: expected 'hello', got '%s'\n", 
            attr_value);
    miclose_volume(volume_read);
    return 1;
  }

  result = miclose_volume(volume_read);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to close reopened volume\n");
    return 1;
  }

  printf("    PASSED\n");
  return 0;
}

/**
 * Test scalar dataset creation with explicit type
 */
static int test_scalar_with_type(const char *filename)
{
  mihandle_t volume;
  mihandle_t volume_read;
  int result;
  double test_value = 3.14159;
  double read_value;
  mitype_t attr_type;
  midimhandle_t hdim[1];
  misize_t start[1], count[1];
  short data[5];
  int i;

  printf("  Test 2: Creating scalar dataset with micreate_scalar_dataset_typed...\n");

  /* Create a dimension for the volume */
  result = micreate_dimension("xspace", MI_DIMCLASS_SPATIAL, 
                              MI_DIMATTR_REGULARLY_SAMPLED, 5, &hdim[0]);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to create dimension\n");
    return 1;
  }

  /* Create volume with 1 dimension */
  result = micreate_volume(filename, 1, hdim, MI_TYPE_SHORT, 
                           MI_CLASS_REAL, NULL, &volume);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to create volume\n");
    return 1;
  }

  /* Create volume image */
  result = micreate_volume_image(volume);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to create volume image\n");
    miclose_volume(volume);
    return 1;
  }

  /* Write some dummy data */
  for (i = 0; i < 5; i++) {
    data[i] = (short)(i * 10);
  }
  start[0] = 0;
  count[0] = 5;
  result = miset_voxel_value_hyperslab(volume, MI_TYPE_SHORT, start, count, data);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to write voxel data\n");
    miclose_volume(volume);
    return 1;
  }

  /* Create scalar with specific type */
  result = micreate_scalar_dataset_typed(volume, "typed_scalar", MI_TYPE_DOUBLE);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to create typed scalar dataset\n");
    miclose_volume(volume);
    return 1;
  }

  /* Add double attribute */
  result = miset_attr_values(volume, MI_TYPE_DOUBLE, 
                             "typed_scalar", "pi_value",
                             1, &test_value);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to set double attribute\n");
    miclose_volume(volume);
    return 1;
  }

  result = miclose_volume(volume);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to close volume\n");
    return 1;
  }

  /* Reopen the file and verify the attribute was written correctly */
  printf("    Reopening file to verify typed attribute...\n");
  result = miopen_volume(filename, MI2_OPEN_READ, &volume_read);
  if (result < 0) {
    fprintf(stderr, "    Failed to reopen volume for reading\n");
    return 1;
  }

  /* Check attribute type */
  result = miget_attr_type(volume_read, "typed_scalar", "pi_value", &attr_type);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to get attribute type from reopened file\n");
    miclose_volume(volume_read);
    return 1;
  }
  if (attr_type != MI_TYPE_DOUBLE) {
    fprintf(stderr, "    Attribute type mismatch: expected %d, got %d\n", 
            MI_TYPE_DOUBLE, attr_type);
    miclose_volume(volume_read);
    return 1;
  }

  /* Check attribute value */
  result = miget_attr_values(volume_read, MI_TYPE_DOUBLE, 
                             "typed_scalar", "pi_value",
                             1, &read_value);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to get double attribute value from reopened file\n");
    miclose_volume(volume_read);
    return 1;
  }
  if (read_value != test_value) {
    fprintf(stderr, "    Attribute value mismatch: expected %f, got %f\n", 
            test_value, read_value);
    miclose_volume(volume_read);
    return 1;
  }

  result = miclose_volume(volume_read);
  if (result != MI_NOERROR) {
    fprintf(stderr, "    Failed to close reopened volume\n");
    return 1;
  }

  printf("    PASSED\n");
  return 0;
}
