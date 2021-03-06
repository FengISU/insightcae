# Try to find OpenFOAM-1.6-ext
# Once done this will define
#
# Fx40_FOUND          - system has foam-extend-4.0 installed
# Fx40_LIBRARIES      - all OpenFOAM libraries
# Fx40_INCLUDE_PATHS  - all OpenFOAM include paths

#FIND_PATH(Fx40_DIR NAMES etc/bashrc
FIND_FILE(Fx40_BASHRC NAMES bashrc
  HINTS
  $ENV{HOME}/foam/foam-extend-4.0/etc
  $ENV{HOME}/OpenFOAM/foam-extend-4.0/etc
  /opt/foam/foam-extend-4.0/etc
)
message(STATUS ${Fx40_BASHRC})

macro(setOFlibvar prefix) 
  SET(${prefix}_LIBRARIES "")
   FOREACH(f ${ARGN})
    IF (EXISTS "${${prefix}_FOAM_LIBBIN}/lib${f}.so")
      LIST(APPEND ${prefix}_LIBRARIES "${${prefix}_FOAM_LIBBIN}/lib${f}.so")
    endif()
   ENDFOREACH(f)
   set (${prefix}_LIBRARIES ${${prefix}_LIBRARIES})
endmacro()

SET(Fx40_FOUND FALSE)
IF(Fx40_BASHRC)
  #set(Fx40_BASHRC "${Fx40_DIR}/etc/bashrc")
  GET_FILENAME_COMPONENT(Fx40_ETC_DIR ${Fx40_BASHRC} PATH)
  GET_FILENAME_COMPONENT(Fx40_DIR ${Fx40_ETC_DIR} PATH)

  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-c++FLAGS OUTPUT_VARIABLE Fx40_CXX_FLAGS)
  set(Fx40_CXX_FLAGS "${Fx40_CXX_FLAGS} -DFx40 -DOF16ext")

  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-WM_OPTIONS OUTPUT_VARIABLE Fx40_WM_OPTIONS)
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-METIS_LIB_DIR OUTPUT_VARIABLE Fx40_METIS_LIB_DIR)
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-MESQUITE_LIB_DIR OUTPUT_VARIABLE Fx40_MESQUITE_LIB_DIR)
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-PARMETIS_LIB_DIR OUTPUT_VARIABLE Fx40_PARMETIS_LIB_DIR)
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-SCOTCH_LIB_DIR OUTPUT_VARIABLE Fx40_SCOTCH_LIB_DIR)
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-FOAM_APPBIN OUTPUT_VARIABLE Fx40_FOAM_APPBIN)
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-FOAM_LIBBIN OUTPUT_VARIABLE Fx40_FOAM_LIBBIN)

  #execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/printOFLibs ${Fx40_BASHRC} OUTPUT_VARIABLE Fx40_LIBRARIES)
  setOFlibvar(Fx40 
engine
solids
basicThermophysicalModels
laminarFlameSpeedModels
radiation
solidMixture
barotropicCompressibilityModel
liquids
specie
reactionThermophysicalModels
thermophysicalFunctions
chemistryModel
liquidMixture
pdf
MGridGenGAMGAgglomeration
# metisDecomp
decompositionMethods
# parMetisDecomp
# scotchDecomp
incompressibleRASModels
incompressibleTurbulenceModel
incompressibleLESModels
LESfilters
LESdeltas
compressibleRASModels
compressibleTurbulenceModel
compressibleLESModels
edgeMesh
blockMesh
autoMesh
extrudeModel
cfMesh
meshTools
errorEstimation
dbns
finiteVolume
equationReader
dynamicTopoFvMesh
topoChangerFvMesh
dynamicMesh
dynamicFvMesh
RBFMotionSolver
tetMotionSolver
fvMotionSolver
mesquiteMotionSolver
solidBodyMotion
finiteArea
POD
coalCombustion
molecule
potential
molecularMeasurements
lagrangian
solidParticle
lagrangianIntermediate
dieselSpray
dsmc
solidModels
coupledLduMatrix
lduSolvers
conversion
foam
sampling
systemCall
checkFunctionObjects
utilityFunctionObjects
forces
fieldFunctionObjects
IOFunctionObjects
foamCalcFunctions
# immersedBoundaryForceFunctionObject
immersedBoundaryDynamicFvMesh
immersedBoundary
immersedBoundaryTurbulence
surfMesh
multiSolver
ODE
incompressibleTransportModels
interfaceProperties
viscoelasticTransportModels
tetFiniteElement
randomProcesses
)
  message(STATUS "FX40_LIBS: " ${Fx40_LIBRARIES})
  
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/printOFincPath ${Fx40_BASHRC} OUTPUT_VARIABLE Fx40_INCLUDE_PATHS)
  message(STATUS "FX40_INCS: " ${Fx40_INCLUDE_PATHS})

  set(Fx40_LIBSRC_DIR "${Fx40_DIR}/src")
  set(Fx40_LIB_DIR "${Fx40_DIR}/lib/${Fx40_WM_OPTIONS}")
  
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-LINKLIBSO OUTPUT_VARIABLE Fx40_LINKLIBSO_full)
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-LINKEXE OUTPUT_VARIABLE Fx40_LINKEXE_full)
  string(REGEX REPLACE "^[^ ]+" "" Fx40_LINKLIBSO ${Fx40_LINKLIBSO_full})
  string(REGEX REPLACE "^[^ ]+" "" Fx40_LINKEXE ${Fx40_LINKEXE_full})
  set(Fx40_LINKLIBSO "${Fx40_LINKLIBSO} -Xlinker --as-needed")
  set(Fx40_LINKEXE "${Fx40_LINKEXE} -Xlinker --as-needed")
  message(STATUS "libso link flags = "  ${Fx40_LINKLIBSO})
  message(STATUS "exe link flags = "  ${Fx40_LINKEXE})
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${Fx40_BASHRC} print-FOAM_MPI_LIBBIN OUTPUT_VARIABLE Fx40_FOAM_MPI_LIBBIN)

  set(Fx40_INSIGHT_BIN "${CMAKE_BINARY_DIR}/bin/foam-extend-4.0")
  set(Fx40_INSIGHT_LIB "${CMAKE_BINARY_DIR}/lib/foam-extend-4.0")
  
  addOFConfig(Fx40 fx40 163)

  macro (setup_exe_target_Fx40 targetname sources exename includes)
    add_executable(${targetname} ${sources})
    set(allincludes ${includes})
    LIST(APPEND allincludes "${Fx40_INCLUDE_PATHS}")
#    LIST(APPEND allincludes "${Fx40_LIBSRC_DIR}/foam/lnInclude")
    #set_property(TARGET ${targetname} PROPERTY INCLUDE_DIRECTORIES ${allincludes})
    set_target_properties(${targetname} PROPERTIES INCLUDE_DIRECTORIES "${allincludes}")
    set_target_properties(${targetname} PROPERTIES COMPILE_FLAGS ${Fx40_CXX_FLAGS})
    set_target_properties(${targetname} PROPERTIES LINK_FLAGS "${Fx40_LINKEXE} ${LIB_SEARCHFLAGS}")
    set_target_properties(${targetname} PROPERTIES OUTPUT_NAME ${exename})
    set_target_properties(${targetname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${Fx40_INSIGHT_BIN})
    target_link_libraries(${targetname} 
#      ${Fx40_LIB_DIR}/libfoam.so 
      #${Fx40_FOAM_MPI_LIBBIN}/libPstream.so 
      #${Fx40_METIS_LIB_DIR}/libmetis.a
      ${Fx40_LIBRARIES}
      ${ARGN}
      ${Fx40_PARMETIS_LIB_DIR}/libparmetis.a
      ${Fx40_SCOTCH_LIB_DIR}/libscotch.so
      ${Fx40_SCOTCH_LIB_DIR}/libscotcherr.so
      ${Fx40_MESQUITE_LIB_DIR}/libmesquite.so
     )
     install(TARGETS ${targetname} RUNTIME DESTINATION ${Fx40_FOAM_APPBIN})
  endmacro()
  
  macro (setup_lib_target_Fx40 targetname sources exename includes)
    get_directory_property(temp LINK_DIRECTORIES)
    
    SET(LIB_SEARCHFLAGS "-L${Fx40_LIB_DIR} -L${Fx40_FOAM_MPI_LIBBIN} -L${Fx40_METIS_LIB_DIR} -L${Fx40_PARMETIS_LIB_DIR} -L${Fx40_SCOTCH_LIB_DIR} -L${Fx40_MESQUITE_LIB_DIR}")
    add_library(${targetname} SHARED ${sources})
    set(allincludes ${includes})
    LIST(APPEND allincludes "${Fx40_INCLUDE_PATHS}")
#    LIST(APPEND allincludes "${Fx40_LIBSRC_DIR}/foam/lnInclude")
#    set_property(TARGET ${targetname} PROPERTY INCLUDE_DIRECTORIES ${allincludes})
    set_target_properties(${targetname} PROPERTIES INCLUDE_DIRECTORIES "${allincludes}")
    set_target_properties(${targetname} PROPERTIES COMPILE_FLAGS ${Fx40_CXX_FLAGS})
    set_target_properties(${targetname} PROPERTIES LINK_FLAGS "${Fx40_LINKLIBSO} ${LIB_SEARCHFLAGS}")
    set_target_properties(${targetname} PROPERTIES OUTPUT_NAME ${exename})
    set_target_properties(${targetname} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${Fx40_INSIGHT_LIB})
    target_link_libraries(${targetname} ${Fx40_LIBRARIES} ${ARGN}) 
    install(TARGETS ${targetname} LIBRARY DESTINATION ${Fx40_FOAM_LIBBIN})
    
    set_directory_properties(LINK_DIRECTORIES ${temp})
  endmacro()
  
  SET(Fx40_FOUND TRUE)
ENDIF(Fx40_BASHRC)
