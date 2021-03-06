cmake_minimum_required(VERSION 3.0)

if (DEFINED BANG_PHYSX_BUILT)
    return()
endif()
set(BANG_PHYSX_BUILT ON)

# -----------------------------------------------------------------------------------------------------------------------------
# PROJECT
# -----------------------------------------------------------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PHYSX_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# -----------------------------------------------------------------------------------------------------------------------------
# SUB-DIRECTORIES
# -----------------------------------------------------------------------------------------------------------------------------

include_directories("${PHYSX_ROOT_DIR}/PhysX_3.4/Include")
include_directories("${PHYSX_ROOT_DIR}/PxShared/include")
link_directories("${PHYSX_ROOT_DIR}/lib")

set(PHYSX_INCLUDE_DIRECTORIES "")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/include/filebuf")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/include/foundation")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/include/foundation/unix")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/include/cudamanager")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/include/task")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/include/pvd")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/src/filebuf/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/src/fastxml/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/src/foundation/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/src/foundation/include/unix")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/src/cudamanager/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PxShared/src/pvd/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/geometry/")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/extensions/")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXGpu/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/common")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/common/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/common/include/collision")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/common/include/pipeline")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/common/include/utils")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/common/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/common/src/collision")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/common/src/pipeline")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/API/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/API/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/software/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevel/software/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelDynamics/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelDynamics/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelAABB/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelAABB/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/headers")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/ccd")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/gjk")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/convex")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/common")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/hf")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/pcm")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/distance")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/mesh")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/sweep")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/intersection")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/GeomUtils/src/contact")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelParticles/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelParticles/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXCooking/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXCooking/src/convex")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXCooking/src/mesh")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysX/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysX/src/device")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysX/src/device/linux")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysX/src/buffering")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysX/src/particles")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysX/src/cloth")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/Common/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/Common/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXCharacterKinematic/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/SceneQuery/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/SceneQuery/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/SimulationController/include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/SimulationController/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/SimulationController/src/particles")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/SimulationController/src/cloth")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/extensions")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/geometry")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/collision")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/GeomUtils")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/GeomUtil/src")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/vehicle")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/common")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/cooking")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/particles")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/cloth")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/characterkinematic")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/solver")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/gpu")
list(APPEND PHYSX_INCLUDE_DIRECTORIES "${PHYSX_ROOT_DIR}/PhysX_3.4/Include/pvd")

file(GLOB_RECURSE PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/*.cpp")

# Remove not needed sources
file(GLOB_RECURSE PHYSX_WINDOWS_SRC_FILES_1 "${PHYSX_ROOT_DIR}/*indows*")
file(GLOB_RECURSE PHYSX_WINDOWS_SRC_FILES_2 "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/windows/*.cpp")
remove_from_list_another_list(PHYSX_SRC_FILES "${PHYSX_WINDOWS_SRC_FILES_1}")
remove_from_list_another_list(PHYSX_SRC_FILES "${PHYSX_WINDOWS_SRC_FILES_2}")

file(GLOB_RECURSE PHYSX_VEHICLE_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXVehicle/src/*")
remove_from_list_another_list(PHYSX_SRC_FILES "${PHYSX_VEHICLE_SRC_FILES}")

file(GLOB_RECURSE PHYSX_GPU_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXGpu/*")
remove_from_list_another_list(PHYSX_SRC_FILES "${PHYSX_GPU_SRC_FILES}")

file(GLOB_RECURSE PHYSX_EXT_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXExtensions/*")
remove_from_list_another_list(PHYSX_SRC_FILES "${PHYSX_EXT_SRC_FILES}")
list(APPEND PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXExtensions/src/ExtTriangleMeshExt.cpp")
list(APPEND PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXExtensions/src/ExtRigidBodyExt.cpp")
list(APPEND PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXExtensions/src/ExtDefaultErrorCallback.cpp")
list(APPEND PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXExtensions/src/ExtSimpleFactory.cpp")
list(APPEND PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXExtensions/src/ExtDefaultCpuDispatcher.cpp")
list(APPEND PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXExtensions/src/ExtCpuWorkerThread.cpp")

file(GLOB_RECURSE PHYSX_IMM_MODE_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/ImmediateMode/*")
remove_from_list_another_list(PHYSX_SRC_FILES "${PHYSX_IMM_MODE_SRC_FILES}")

file(GLOB_RECURSE PHYSX_META_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/PhysXMetaData/*")
remove_from_list_another_list(PHYSX_SRC_FILES "${PHYSX_META_SRC_FILES}")

file(GLOB_RECURSE PHYSX_GPU_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/*gpu/*")
remove_from_list_another_list(PHYSX_SRC_FILES "${PHYSX_GPU_SRC_FILES}")

list(REMOVE_ITEM PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/avx/SwSolveConstraints.cpp")
list(REMOVE_ITEM PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/avx/SwSolveConstraints.cpp")
list(REMOVE_ITEM PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/neon/SwCollisionHelpers.h")
list(REMOVE_ITEM PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/neon/NeonCollision.cpp")
list(REMOVE_ITEM PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/neon/NeonSelfCollision.cpp")
list(REMOVE_ITEM PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/neon/NeonSolverKernel.cpp")
list(REMOVE_ITEM PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/scalar/SwCollisionHelpers.h")
list(REMOVE_ITEM PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/sse2/SwCollisionHelpers.h")
list(REMOVE_ITEM PHYSX_SRC_FILES "${PHYSX_ROOT_DIR}/PhysX_3.4/Source/LowLevelCloth/src/sse2/SwSolveConstraints.h")

add_library(PhysXLib ${PHYSX_SRC_FILES})

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

    add_definitions(-D_DEBUG)
    add_definitions(-DPX_DEBUG=1)
    add_definitions(-DPX_CHECKED=1)
    target_compile_options(PhysXLib PUBLIC "-O0")
    target_compile_options(PhysXLib PUBLIC "-g3")
    target_compile_options(PhysXLib PUBLIC "-ggdb")

elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")

    add_definitions(-DNDEBUG)
    target_compile_options(PhysXLib PUBLIC "-O3")

endif()

add_definitions(-DPX_SUPPORT_PVD=0)
target_compile_options(PhysXLib PUBLIC "-w")
target_compile_options(PhysXLib PUBLIC "-pipe")
target_compile_options(PhysXLib PUBLIC "-fPIC")
target_compile_options(PhysXLib PUBLIC "-msse2")
# target_compile_options(PhysXLib PUBLIC "-frtti")
target_compile_options(PhysXLib PUBLIC "-std=c++11")
target_compile_options(PhysXLib PUBLIC "-ffast-math")
target_compile_options(PhysXLib PUBLIC "-fno-exceptions")
# target_compile_options(PhysXLib PUBLIC "-fdata-sections")
target_compile_options(PhysXLib PUBLIC "-fstrict-aliasing")
# target_compile_options(PhysXLib PUBLIC "-fvisibility=hidden")
target_compile_options(PhysXLib PUBLIC "-fvisibility-inlines-hidden")
# target_compile_options(PhysXLib PUBLIC "-ffunction-sections")
target_include_directories(PhysXLib PUBLIC ${PHYSX_INCLUDE_DIRECTORIES})
