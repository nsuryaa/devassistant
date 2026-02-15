############################################################
# DevAssistant Qt Creator Plugin Project File
# Qt Creator Version  : 4.11.1
# Qt Version          : 5.14.x
# Compiler            : MSVC 2017 (32-bit)
############################################################

# Enable Qt Network module
# Required for HTTP / REST communication (e.g., AI backend, Ollama)
QT += network

############################################################
# Plugin build configuration
############################################################

# Placeholder for plugin dependencies
# Actual dependencies are defined later using QTC_PLUGIN_DEPENDS +=
QTC_PLUGIN_DEPENDS +=

# Define macro for DLL export/import handling
# Used in devassistant_global.h
DEFINES += DEVASSISTANT_LIBRARY

############################################################
# Source files for the DevAssistant plugin
############################################################

SOURCES += \
    devassistant_cdstdlder.cpp \
    devassistant_diffviewer.cpp \
    devassistant_ollamamanager.cpp \
    devassistant_ollamaservice.cpp \
    devassistant_suggestor.cpp \
    devassistantplugin.cpp \      # Plugin entry point and lifecycle handling
    devassistant_client.cpp \     # Client logic (AI / backend communication)
    devassistant_mainai.cpp \     # Main widget implementation
    devassistant_modeai.cpp \        # Custom Qt Creator mode integration
    devassistant_editorutils.cpp \
    devassistant_rvwprptbdr.cpp

############################################################
# Header files for the DevAssistant plugin
############################################################

HEADERS += \
    devassistant_cdstdlder.h \
    devassistant_diffviewer.h \
    devassistant_ollamamanager.h \
    devassistant_ollamaservice.h \
    devassistant_suggestor.h \
    devassistantplugin.h \        # Plugin class declaration
    devassistant_global.h \       # Export/import macros for Windows DLL
    devassistantconstants.h \     # Plugin-wide constants and identifiers
    devassistant_client.h \       # Backend / AI client interface
    devassistant_mainai.h \       # Main widget declarations
    devassistant_modeai.h \          # Custom mode class declaration
    devassistant_editorutils.h \
    devassistant_rvwprptbdr.h

# Qt Creator linking

## Either set the IDE_SOURCE_TREE when running qmake,
## or set the QTC_SOURCE environment variable, to override the default setting
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = $$(QTC_SOURCE)
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = "E:/QtSource/origin/qt-creator-opensource-src-4.11.1"

## Either set the IDE_BUILD_TREE when running qmake,
## or set the QTC_BUILD environment variable, to override the default setting
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = "E:/QtSource/origin/qt-creator-opensource-src-4.11.1/build"

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%QtProjectqtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on OS X
USE_USER_DESTDIR = yes

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

# Name of the Qt Creator plugin
# Must match plugin metadata and binary naming
QTC_PLUGIN_NAME = DevAssistant

# Qt Creator internal library dependencies
# Keep empty unless linking directly to Qt Creator libraries
QTC_LIB_DEPENDS += # nothing here at this time

# Required Qt Creator plugin dependencies
# These plugins must be present and loaded for DevAssistant to work
QTC_PLUGIN_DEPENDS += \
    coreplugin \    # Core Qt Creator APIs
    texteditor \    # Text editor integration
    cppeditor       # C/C++ editor and language services

QTC_PLUGIN_RECOMMENDS += # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

############################################################
# Qt Creator plugin build rules
############################################################

# Include Qt Creator plugin build configuration
# This file provides:
# - Plugin build macros
# - Compiler and linker flags
# - Correct install paths
# - Version handling
#
# Without this include, the plugin will NOT build correctly
include($$IDE_SOURCE_TREE/src/qtcreatorplugin.pri)

############################################################
# Qt resource files
############################################################

# Resource file containing icons and other embedded assets
RESOURCES += \
    devassistant.qrc
