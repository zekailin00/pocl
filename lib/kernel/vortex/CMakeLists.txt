#=============================================================================
#   CMake build system files
#
#   Copyright (c) 2014-2018 pocl developers
#
#   Permission is hereby granted, free of charge, to any person obtaining a copy
#   of this software and associated documentation files (the "Software"), to deal
#   in the Software without restriction, including without limitation the rights
#   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#   copies of the Software, and to permit persons to whom the Software is
#   furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included in
#   all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#   THE SOFTWARE.
#
#=============================================================================

include("bitcode_rules")

set(LLVM_TARGET riscv32)

set(KERNEL_SOURCES ${SOURCES_GENERIC})

foreach(FILE atomics.cl)
  list(REMOVE_ITEM KERNEL_SOURCES "${FILE}")
endforeach()

set(CLANG_FLAGS "-ffreestanding" "-target" "${LLVM_TARGET}" "-emit-llvm" "-D_CL_DISABLE_HALF")

set(KERNEL_CL_FLAGS "-Xclang" "-cl-std=CL${VORTEX_DEVICE_CL_STD}" "-D__OPENCL_C_VERSION__=${VORTEX_DEVICE_CL_VERSION}" ${KERNEL_CL_FLAGS})

set(LLC_FLAGS "")

set(DEVICE_CL_FLAGS "-D__OPENCL_VERSION__=${VORTEX_DEVICE_CL_VERSION} -DPOCL_DEVICE_ADDRESS_BITS=32")
separate_arguments(VORTEX_DEVICE_EXTENSIONS)
foreach(EXT ${VORTEX_DEVICE_EXTENSIONS})
  set(DEVICE_CL_FLAGS "${DEVICE_CL_FLAGS} -D${EXT}")
endforeach()
separate_arguments(DEVICE_CL_FLAGS)

make_kernel_bc(KERNEL_BC "${LLVM_TARGET}" "BCs" 0 0 0 ${KERNEL_SOURCES})

# just debug
message(STATUS "${LLVM_TARGET} Kernel BC: ${KERNEL_BC}")

list(APPEND KERNEL_BC_LIST "${KERNEL_BC}")
set(KERNEL_BC_LIST "${KERNEL_BC_LIST}" PARENT_SCOPE)

# a target is needed...
add_custom_target("kernel_${LLVM_TARGET}" DEPENDS ${KERNEL_BC})

list(APPEND KERNEL_TARGET_LIST "kernel_${LLVM_TARGET}")
set(KERNEL_TARGET_LIST "${KERNEL_TARGET_LIST}" PARENT_SCOPE)

install(FILES "${KERNEL_BC}" DESTINATION "${POCL_INSTALL_PRIVATE_DATADIR}" COMPONENT "lib")
