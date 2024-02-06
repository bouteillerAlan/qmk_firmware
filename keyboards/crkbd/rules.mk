# Build Options
#   change yes to no to disable
#

DEFAULT_FOLDER = crkbd/rev1
BOOTLOADER = atmel-dfu

RGBLIGHT_SUPPORTED = yes
RGB_MATRIX_SUPPORTED = yes

RGBLIGHT_ENABLE = no
RGB_MATRIX_ENABLE = yes
WPM_ENABLE = yes

SRC += ./graph.c \
