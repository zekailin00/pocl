/* poclu_misc - misc generic OpenCL helper functions

   Copyright (c) 2013 Pekka Jääskeläinen / Tampere University of Technology
   Copyright (c) 2014 Kalle Raiskila

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include "poclu.h"
#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

cl_context
poclu_create_any_context ()
{
  cl_uint i;
  cl_platform_id platform;

  clGetPlatformIDs (1, &platform, &i);
  if (i == 0)
    return (cl_context) 0;

  cl_context_properties properties[] =
    {CL_CONTEXT_PLATFORM,
     (cl_context_properties)platform,
     0};

  // create the OpenCL context on any available OCL device
  cl_context context = clCreateContextFromType (properties,
                                                CL_DEVICE_TYPE_ALL,
                                                NULL, NULL, NULL);

  return context;
}

cl_int
poclu_get_any_device (cl_context *context, cl_device_id *device,
                      cl_command_queue *queue)
{
  cl_platform_id temp;
  return poclu_get_any_device2 (context, device, queue, &temp);
}

cl_int
poclu_get_any_device2 (cl_context *context, cl_device_id *device,
                       cl_command_queue *queue, cl_platform_id *platform)
{
  cl_int err;

  if (context == NULL ||
      device  == NULL ||
      queue   == NULL ||
      platform == NULL)
    return CL_INVALID_VALUE;

  err = clGetPlatformIDs (1, platform, NULL);
  if (err != CL_SUCCESS)
    return err;

  err = clGetDeviceIDs (*platform, CL_DEVICE_TYPE_ALL, 1, device, NULL);
  if (err != CL_SUCCESS)
    return err;

  *context = clCreateContext (NULL, 1, device, NULL, NULL, &err);
  if (err != CL_SUCCESS)
    return err;

  *queue = clCreateCommandQueue (*context, *device,
                                  CL_QUEUE_PROFILING_ENABLE, &err);
  if (err != CL_SUCCESS)
    return err;

  return CL_SUCCESS;
}

cl_int
poclu_get_multiple_devices (cl_platform_id *platform, cl_context *context,
                            cl_uint *num_devices, cl_device_id **devices,
                            cl_command_queue **queues)
{
  cl_int err;
  *num_devices = 0;
  size_t i;

  if (context == NULL || devices == NULL || queues == NULL || platform == NULL)
    return CL_INVALID_VALUE;

  err = clGetPlatformIDs (1, platform, NULL);
  if (err != CL_SUCCESS)
    return err;

  err = clGetDeviceIDs (*platform, CL_DEVICE_TYPE_ALL, 0, NULL, num_devices);
  if (err != CL_SUCCESS)
    return err;

  cl_device_id *devs = calloc (*num_devices, sizeof (cl_device_id));
  cl_command_queue *ques = calloc (*num_devices, sizeof (cl_command_queue));

  err = clGetDeviceIDs (*platform, CL_DEVICE_TYPE_ALL, *num_devices, devs,
                        NULL);
  if (err != CL_SUCCESS)
    return err;

  *context = clCreateContext (NULL, *num_devices, devs, NULL, NULL, &err);
  if (err != CL_SUCCESS)
    return err;

  for (i = 0; i < *num_devices; ++i)
    {
      ques[i] = clCreateCommandQueue (*context, devs[i],
                                      CL_QUEUE_PROFILING_ENABLE, &err);
      if (err != CL_SUCCESS)
        return err;
    }

  *devices = devs;
  *queues = ques;
  return CL_SUCCESS;
}

char *
poclu_read_binfile (const char *filename, size_t *len)
{
  FILE *file;
  char *src;

  file = fopen (filename, "r");
  if (file == NULL)
    return NULL;

  fseek (file, 0, SEEK_END);
  *len = ftell (file);
  src = (char*)malloc (*len + 1);
  if (src == NULL)
    {
      fclose (file);
      return NULL;
    }

  fseek (file, 0, SEEK_SET);
  fread (src, *len, 1, file);
  src[*len] = '\0';
  fclose (file);

  return src;
}

char *
poclu_read_file (const char *filename)
{
  size_t size;
  char *res = poclu_read_binfile (filename, &size);
  if (res)
    res[size] = 0;
  return res;
}

int
poclu_write_file (const char *filemane, char *content, size_t size)
{
  FILE *file;

  file = fopen (filemane, "w");
  if (file == NULL)
    return -1;

  if (fwrite (content, sizeof (char), size, file) < size)
  {
    fclose (file);
    return -1;
  }

  if (fclose (file))
    return -1;

  return 0;
}

#define OPENCL_ERROR_CASE(ERR) \
  case ERR:                                                             \
  { fprintf (stderr, "" #ERR " in %s on line %i\n", func_name, line);   \
    return 1; }

int
check_cl_error (cl_int cl_err, int line, const char* func_name) {

  switch (cl_err)
    {
    case CL_SUCCESS: return 0;

      OPENCL_ERROR_CASE (CL_DEVICE_NOT_FOUND)
        OPENCL_ERROR_CASE (CL_DEVICE_NOT_AVAILABLE)
        OPENCL_ERROR_CASE (CL_COMPILER_NOT_AVAILABLE)
        OPENCL_ERROR_CASE (CL_MEM_OBJECT_ALLOCATION_FAILURE)
        OPENCL_ERROR_CASE (CL_OUT_OF_RESOURCES)
        OPENCL_ERROR_CASE (CL_OUT_OF_HOST_MEMORY)
        OPENCL_ERROR_CASE (CL_PROFILING_INFO_NOT_AVAILABLE)
        OPENCL_ERROR_CASE (CL_MEM_COPY_OVERLAP)
        OPENCL_ERROR_CASE (CL_IMAGE_FORMAT_MISMATCH)
        OPENCL_ERROR_CASE (CL_IMAGE_FORMAT_NOT_SUPPORTED)
        OPENCL_ERROR_CASE (CL_BUILD_PROGRAM_FAILURE)
        OPENCL_ERROR_CASE (CL_MAP_FAILURE)
        OPENCL_ERROR_CASE (CL_MISALIGNED_SUB_BUFFER_OFFSET)
        OPENCL_ERROR_CASE (CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        OPENCL_ERROR_CASE (CL_COMPILE_PROGRAM_FAILURE)
        OPENCL_ERROR_CASE (CL_LINKER_NOT_AVAILABLE)
        OPENCL_ERROR_CASE (CL_LINK_PROGRAM_FAILURE)
        OPENCL_ERROR_CASE (CL_DEVICE_PARTITION_FAILED)
        OPENCL_ERROR_CASE (CL_KERNEL_ARG_INFO_NOT_AVAILABLE)
        OPENCL_ERROR_CASE (CL_INVALID_VALUE)
        OPENCL_ERROR_CASE (CL_INVALID_DEVICE_TYPE)
        OPENCL_ERROR_CASE (CL_INVALID_PLATFORM)
        OPENCL_ERROR_CASE (CL_INVALID_DEVICE)
        OPENCL_ERROR_CASE (CL_INVALID_CONTEXT)
        OPENCL_ERROR_CASE (CL_INVALID_QUEUE_PROPERTIES)
        OPENCL_ERROR_CASE (CL_INVALID_COMMAND_QUEUE)
        OPENCL_ERROR_CASE (CL_INVALID_HOST_PTR)
        OPENCL_ERROR_CASE (CL_INVALID_MEM_OBJECT)
        OPENCL_ERROR_CASE (CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        OPENCL_ERROR_CASE (CL_INVALID_IMAGE_SIZE)
        OPENCL_ERROR_CASE (CL_INVALID_SAMPLER)
        OPENCL_ERROR_CASE (CL_INVALID_BINARY)
        OPENCL_ERROR_CASE (CL_INVALID_BUILD_OPTIONS)
        OPENCL_ERROR_CASE (CL_INVALID_PROGRAM)
        OPENCL_ERROR_CASE (CL_INVALID_PROGRAM_EXECUTABLE)
        OPENCL_ERROR_CASE (CL_INVALID_KERNEL_NAME)
        OPENCL_ERROR_CASE (CL_INVALID_KERNEL_DEFINITION)
        OPENCL_ERROR_CASE (CL_INVALID_KERNEL)
        OPENCL_ERROR_CASE (CL_INVALID_ARG_INDEX)
        OPENCL_ERROR_CASE (CL_INVALID_ARG_VALUE)
        OPENCL_ERROR_CASE (CL_INVALID_ARG_SIZE)
        OPENCL_ERROR_CASE (CL_INVALID_KERNEL_ARGS)
        OPENCL_ERROR_CASE (CL_INVALID_WORK_DIMENSION)
        OPENCL_ERROR_CASE (CL_INVALID_WORK_GROUP_SIZE)
        OPENCL_ERROR_CASE (CL_INVALID_WORK_ITEM_SIZE)
        OPENCL_ERROR_CASE (CL_INVALID_GLOBAL_OFFSET)
        OPENCL_ERROR_CASE (CL_INVALID_EVENT_WAIT_LIST)
        OPENCL_ERROR_CASE (CL_INVALID_EVENT)
        OPENCL_ERROR_CASE (CL_INVALID_OPERATION)
        OPENCL_ERROR_CASE (CL_INVALID_GL_OBJECT)
        OPENCL_ERROR_CASE (CL_INVALID_BUFFER_SIZE)
        OPENCL_ERROR_CASE (CL_INVALID_MIP_LEVEL)
        OPENCL_ERROR_CASE (CL_INVALID_GLOBAL_WORK_SIZE)
        OPENCL_ERROR_CASE (CL_INVALID_PROPERTY)
        OPENCL_ERROR_CASE (CL_INVALID_IMAGE_DESCRIPTOR)
        OPENCL_ERROR_CASE (CL_INVALID_COMPILER_OPTIONS)
        OPENCL_ERROR_CASE (CL_INVALID_LINKER_OPTIONS)
        OPENCL_ERROR_CASE (CL_INVALID_DEVICE_PARTITION_COUNT)

    default:
      printf ("Unknown OpenCL error %i in %s on line %i\n", cl_err, func_name,
              line);
      return 1;
    }
}

int
poclu_load_program_multidev (cl_context context, cl_device_id *devices,
                             cl_uint num_devices, const char *basename,
                             int spir, int spirv, int poclbin,
                             const char *explicit_binary,
                             const char *extra_build_opts, cl_program *p)
{
#if defined(OCS_AVAILABLE) || (!defined(BUILD_NEWLIB) && !defined(BUILD_VORTEX_NEWLIB))
  cl_bool little_endian = 0;
  cl_uint address_bits = 0;
  char extensions[1024];
  char path[1024];
  const char *ext;
  char final_opts[2048];
  size_t binary_size = 0;
  char *binary = NULL;
  cl_program program;
  cl_int err = CL_SUCCESS;

  int from_source = (!spir && !spirv && !poclbin);
  TEST_ASSERT (num_devices > 0);
  cl_device_id device = devices[0];
  if (num_devices > 1)
    TEST_ASSERT (from_source);

  *p = NULL;
  final_opts[0] = 0;
  if (extra_build_opts != NULL)
    strcat(final_opts, extra_build_opts);

  if (spir || spirv)
    {
      TEST_ASSERT (device != NULL);
      strcat (final_opts, " -x spir -spir-std=1.2");
      err = clGetDeviceInfo (device, CL_DEVICE_EXTENSIONS, 1024, extensions,
                             NULL);
      CHECK_OPENCL_ERROR_IN ("clGetDeviceInfo extensions");

      if (strstr (extensions, "cl_khr_spir") == NULL)
        {
          printf ("SPIR not supported, cannot run the test\n");
          return -1;
        }

      err = clGetDeviceInfo (device, CL_DEVICE_ENDIAN_LITTLE, sizeof (cl_bool),
                             &little_endian, NULL);
      CHECK_OPENCL_ERROR_IN ("clGetDeviceInfo endianness");

      if (little_endian == CL_FALSE)
        {
          fprintf (stderr,
                   "SPIR can only be tested on little-endian devices\n");
          return 1;
        }

      err = clGetDeviceInfo (device, CL_DEVICE_ADDRESS_BITS, sizeof (cl_uint),
                             &address_bits, NULL);
      CHECK_OPENCL_ERROR_IN ("clGetDeviceInfo addr bits");

      if (spirv)
      {
        if (address_bits < 64)
          ext = ".spirv32";
        else
          ext = ".spirv64";
      }
      else
      {
        if (address_bits < 64)
          ext = ".spir32";
        else
          ext = ".spir64";
      }
    }

  if (poclbin)
    {
      ext = ".poclbin";
    }

  if (from_source)
    {
      ext = ".cl";
    }

  if (explicit_binary)
    snprintf (path, 1024, "%s", explicit_binary);
  else
    {
      snprintf (path, 1024, "%s%s", basename, ext);

      if (access (path, F_OK))
        {
          snprintf (path, 1024, "%s/examples/%s/%s%s", SRCDIR, basename,
                    basename, ext);
          if (access (path, F_OK))
            {
              fprintf (stderr, "Can't find %s%s SPIR / POCLBIN file anywhere\n",
                       basename, ext);
              return 1;
            }
        }
    }

  if (from_source)
    {
      char *src = poclu_read_file (path);
      TEST_ASSERT (src != NULL);

      program = clCreateProgramWithSource (context, 1, (const char **)&src,
                                           NULL, &err);
      CHECK_OPENCL_ERROR_IN ("clCreateProgramWithSource");

      err = clBuildProgram (program, num_devices, devices, final_opts, NULL,
                            NULL);
      CHECK_OPENCL_ERROR_IN ("clBuildProgram");
      free (src);
    }
  else if (spirv)
    {
      TEST_ASSERT (device != NULL);
      binary = poclu_read_binfile (path, &binary_size);
      TEST_ASSERT (binary != NULL);

      program = clCreateProgramWithIL (context, (const void *)binary,
                                       binary_size, &err);
      CHECK_OPENCL_ERROR_IN ("clCreateProgramWithIL");

      err = clBuildProgram (program, 0, NULL, final_opts, NULL, NULL);
      CHECK_OPENCL_ERROR_IN ("clBuildProgram");
      free (binary);
    }
  else
    {
      TEST_ASSERT (device != NULL);
      binary = poclu_read_binfile (path, &binary_size);
      TEST_ASSERT (binary != NULL);

      program = clCreateProgramWithBinary (context, 1, &device, &binary_size,
                                           (const unsigned char **)&binary,
                                           NULL, &err);
      CHECK_OPENCL_ERROR_IN ("clCreateProgramWithBinary");

      err = clBuildProgram (program, 0, NULL, final_opts, NULL, NULL);
      CHECK_OPENCL_ERROR_IN ("clBuildProgram");
      free (binary);
    }

  *p = program;
  return err;
#else 
  return 0;
#endif
}

int
poclu_load_program (cl_context context, cl_device_id device,
                    const char *basename, int spir, int spirv, int poclbin,
                    const char *explicit_binary, const char *extra_build_opts,
                    cl_program *p)
{
  return poclu_load_program_multidev (context, &device, 1, basename, spir,
                                      spirv, poclbin, explicit_binary,
                                      extra_build_opts, p);
}
