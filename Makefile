# ----------------------------
# Makefile Options
# ----------------------------

NAME = NEONIDE
ICON = icon.png
DESCRIPTION = "Neon IDE"
COMPRESSED = YES
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)