# Install script for directory: C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/ANC_Lite")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/Program Files/JetBrains/CLion 2024.3.1.1/bin/mingw/bin/objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/cmake-build-debug/JUCE/tools/modules/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/cmake-build-debug/JUCE/tools/extras/Build/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-7.0.12" TYPE FILE FILES
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/cmake-build-debug/JUCE/tools/JUCEConfigVersion.cmake"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/cmake-build-debug/JUCE/tools/JUCEConfig.cmake"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/JUCECheckAtomic.cmake"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/JUCEHelperTargets.cmake"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/JUCEModuleSupport.cmake"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/JUCEUtils.cmake"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/JuceLV2Defines.h.in"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/LaunchScreen.storyboard"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/PIPAudioProcessor.cpp.in"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/PIPAudioProcessorWithARA.cpp.in"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/PIPComponent.cpp.in"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/PIPConsole.cpp.in"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/RecentFilesMenuTemplate.nib"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/UnityPluginGUIScript.cs.in"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/checkBundleSigning.cmake"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/copyDir.cmake"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/juce_runtime_arch_detection.cpp"
    "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/JUCE/extras/Build/CMake/juce_LinuxSubprocessHelper.cpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
  file(WRITE "C:/Users/SBNut/OOP/DigiPro-Noise-Canceling-/cmake-build-debug/JUCE/tools/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
