

set(PROGRAMS_TO_BUILD test_clFinish test_clGetDeviceInfo test_clGetEventInfo
  test_clCreateProgramWithBinary test_clGetSupportedImageFormats
  test_clSetEventCallback test_clEnqueueNativeKernel test_clBuildProgram
  test_clCreateKernelsInProgram test_version)

#EXTRA_DIST= \
# test_kernel_src_in_pwd.h \
# test_clCreateKernelsInProgram.cl \
# test_data/test_kernel_src_in_another_dir.h

#AM_LDFLAGS = @OPENCL_LIBS@ ../../lib/poclu/libpoclu.la
# POCLU_LINK_OPTIONS

#AM_CPPFLAGS = -I$(top_srcdir)/fix-include -I$(top_srcdir)/include @OPENCL_CFLAGS@
add_compile_options(${OPENCL_CFLAGS})

foreach(PROG ${PROGRAMS_TO_BUILD})
  add_executable("${PROG}" "${PROG}.c")
  target_link_libraries("${PROG}" ${POCLU_LINK_OPTIONS})
endforeach()

#######################################################################


add_test("runtime/clGetDeviceInfo" "test_clGetDeviceInfo")

add_test("runtime/clEnqueueNativeKernel" "test_clEnqueueNativeKernel")

add_test("runtime/clGetEventInfo" "test_clGetEventInfo")

add_test("runtime/clCreateProgramWithBinary" "test_clCreateProgramWithBinary")

add_test(NAME "runtime/clBuildProgram"
         WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
         COMMAND "${CMAKE_CURRENT_BINARY_DIR}/test_clBuildProgram")

add_test("runtime/clFinish" "test_clFinish")

add_test("runtime/clSetEventCallback" "test_clSetEventCallback")

add_test("runtime/clGetSupportedImageFormats" "test_clGetSupportedImageFormats")

add_test("runtime/clCreateKernelsInProgram" "test_clCreateKernelsInProgram")

set_tests_properties( "runtime/clGetDeviceInfo" "runtime/clEnqueueNativeKernel"
  "runtime/clGetEventInfo" "runtime/clCreateProgramWithBinary"
  "runtime/clBuildProgram" "runtime/clFinish" "runtime/clSetEventCallback"
  "runtime/clGetSupportedImageFormats" "runtime/clCreateKernelsInProgram"
  PROPERTIES
    COST 2.0
    PROCESSORS 1
    DEPENDS "pocl_version_check")

set_tests_properties("runtime/clGetSupportedImageFormats"
  PROPERTIES
    ENVIRONMENT "POCL_DEVICES=pthread\ pthread")

set_tests_properties("runtime/clCreateKernelsInProgram"
  PROPERTIES
    PASS_REGULAR_EXPRESSION "Hello\nWorld")

set_tests_properties("runtime/clSetEventCallback"
  PROPERTIES
    PASS_REGULAR_EXPRESSION "kernel in execution
Callback function: event status: CL_SUBMITTED
Callback function: event status: CL_RUNNING
Callback function: event status: CL_COMPLETE")


set_tests_properties("runtime/clFinish"
  PROPERTIES
    PASS_REGULAR_EXPRESSION "ABABC")
