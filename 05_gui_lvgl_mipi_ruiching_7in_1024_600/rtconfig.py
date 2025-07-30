#-*- encoding: utf-8 -*-
#-------------------------------------------------------------------------------
# SConscript
# Copyright (c) Shanghai Real-Thread Electronic Technology Co., Ltd.
#-------------------------------------------------------------------------------
import os, sys
import platform
RTT_ROOT = 'rt-thread' if os.path.isdir('rt-thread') \
    else os.path.join(os.getcwd(), '..', '..', '..', '..', '..')
sys.path = sys.path + [os.path.abspath(os.path.join(RTT_ROOT, 'scripts'))]
from cic import cic_gen_parameters
from cic import cic_add_linked_resource

#-------------------------------------------------------------------------------
# System variables
#-------------------------------------------------------------------------------
TARGET_NAME         = 'app'
TARGET_EXT          = 'elf'
TARGET              = TARGET_NAME + '.' + TARGET_EXT
# BUILD             = 'debug'
BUILD               = 'release'

#-------------------------------------------------------------------------------
# Architecture configuration
#-------------------------------------------------------------------------------
CHIP                = 'rk3506'
ARCH                = 'arm'
CPU                 = 'cortex-a'
CROSS_TOOL          = 'gcc'
PLATFORM            = 'gcc'
EXEC_PATH           = os.getenv('RTT_EXEC_PATH') or '/usr/bin'

#-------------------------------------------------------------------------------
# Toolchain configuration
#-------------------------------------------------------------------------------
PREFIX              = 'arm-none-eabi-'
CC                  = PREFIX + 'gcc'
CXX                 = PREFIX + 'g++'
CPP                 = PREFIX + 'cpp'
AS                  = PREFIX + 'gcc'
AR                  = PREFIX + 'ar'
LINK                = PREFIX + 'gcc'
LD                  = PREFIX + 'ld'
SIZE                = PREFIX + 'size'
STRIP               = PREFIX + 'strip'
OBJDUMP             = PREFIX + 'objdump'
OBJCPY              = PREFIX + 'objcopy'
DTC                 = 'dtc'

#-------------------------------------------------------------------------------
# Target processor
#-------------------------------------------------------------------------------
TARGET_PROCESSOR    = '-march=armv7-a '
TARGET_PROCESSOR   += '-mtune=cortex-a7 '
TARGET_PROCESSOR   += '-mfpu=vfpv4 '
TARGET_PROCESSOR   += '-mfloat-abi=soft '

#-------------------------------------------------------------------------------
# Common parameter
#-------------------------------------------------------------------------------
OPTIMIZATION        = '-ftree-vectorize '
OPTIMIZATION       += '-ffast-math '
WARNINGS            = '-Wall'
DEBUGGINGS          = {'debug': '-O0 -g -gdwarf-2', 'release': '-O2'}[BUILD]
COMMON_PREPROCESSOR = ''

#-------------------------------------------------------------------------------
# Assembler parameter
#-------------------------------------------------------------------------------
ASM_PREPROCESSOR    = COMMON_PREPROCESSOR + ''
ASM_INCLUDES        = []
ASM_FLAGS           = '-c -x assembler-with-cpp -D__ASSEMBLY__ '

#-------------------------------------------------------------------------------
# C compiler parameter
#-------------------------------------------------------------------------------
C_PREPROCESSOR      = COMMON_PREPROCESSOR + ''
C_INCLUDES          = []
C_FLAGS             = ''

#-------------------------------------------------------------------------------
# C++ compiler parameter
#-------------------------------------------------------------------------------
CXX_PREPROCESSOR    = COMMON_PREPROCESSOR + ''
CXX_INCLUDES        = []
CXX_FLAGS           = ''

#-------------------------------------------------------------------------------
# Linker parameter
#-------------------------------------------------------------------------------
LINKER_GENERAL      = '-Wl,--gc-sections,-cref,-u,system_vectors '
LINKER_GENERAL     += '-T board/link.lds'

LINKER_LIBRARIES    = ''
LINKER_MISCELLANEOUS = '-Wl,-Map=build/%s.map' % (TARGET_NAME)

#-------------------------------------------------------------------------------
# Image parameter
#-------------------------------------------------------------------------------
FLASH_IMAGE_GENERAL = '-O binary'

#-------------------------------------------------------------------------------
# Print Size parameter
#-------------------------------------------------------------------------------
PRINT_SIZE_GENERAL  = '--format=berkeley'

#-------------------------------------------------------------------------------
# CIC Parameter handling
#-------------------------------------------------------------------------------
COMMON = [TARGET_PROCESSOR, WARNINGS, DEBUGGINGS]
COMPILER_CONFIGS = {
    'ASM'  : COMMON + [ASM_PREPROCESSOR, ASM_INCLUDES, ASM_FLAGS],
    'C'    : COMMON + [OPTIMIZATION, C_PREPROCESSOR, C_INCLUDES, C_FLAGS],
    'CXX'  : COMMON + [OPTIMIZATION, CXX_PREPROCESSOR, CXX_INCLUDES, CXX_FLAGS],
    'Link' : [TARGET_PROCESSOR,
        LINKER_GENERAL, LINKER_LIBRARIES, LINKER_MISCELLANEOUS]
}

AFLAGS, CFLAGS, CXXFLAGS, LFLAGS = (
    cic_gen_parameters(config)
    for config in COMPILER_CONFIGS.values()
)

#-------------------------------------------------------------------------------
# IDE linked resources
#-------------------------------------------------------------------------------
LINKEDRESOURCES = []

#-------------------------------------------------------------------------------
# Post-compile behavior
#-------------------------------------------------------------------------------
POST_ACTION  = '%s -O binary build/%s build/%s\n' % \
    (OBJCPY, TARGET, TARGET_NAME + '.bin')
POST_ACTION += '%s build/%s\n' % (SIZE, TARGET)
POST_ACTION_IDE = 'scons -C ../ --app_pkg=${ConfigName}'
