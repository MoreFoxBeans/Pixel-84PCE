# ----------------------------
# Makefile Options
# ----------------------------

NAME = PIXEL84
ICON = icon.png
DESCRIPTION = "Simple pixel art editor"
COMPRESSED = NO
ARCHIVED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
