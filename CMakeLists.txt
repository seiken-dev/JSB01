cmake_minimum_required(VERSION 3.12)
include($ENV{AVR_FIND_ROOT_PATH}/cmake/avr-sdk.cmake)
set(AVR_UPLOADTOOL_PORT /dev/cu.usbserial-1430)
set(AVR_MCU attiny402)
set(AVR_H_FUSE 0xd9)
set(AVR_L_FUSE 0xc3)

##########################################################################
# name your project
##########################################################################
project(JSB01)

##################
# add AVR executable
##################################################################################
add_avr_executable(
   JSB01
   src/main.c
)

##################################################################################
# add AVR library
##################################################################################
#add_avr_library(
#   <library-name>
#   <sources> <headers>
#)

##################################################################################
# link library to executable
# NOTE: It needs to be the elf target.
##################################################################################
# target_link_libraries(<executable-name>-${AVR_MCU}.elf <library-name>-${AVR_MCU})
# OR easier
#target_link_libraries(<executable-name> <library-name> <some-external-libary>)
