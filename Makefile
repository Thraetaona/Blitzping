# ----------------------------------------------------------------------

# Program name
NAME := blitzping
# Target triplet (https://wiki.osdev.org/Target_Triplet)
TARGET ?= #mips-openwrt-linux-muslsf
# Target sub-architecture
SUBARCH ?= 
# C version (c99 or c11)
C_STD ?= c11
# 200112L is POSIX.1-2001 (IEEE Std 1003.1-2001), a subset of the
# SUSv3 (UNIX 03) standard; Issue 6 (i.e., POSIX 2001) is the earliest
# standard in which <sys/socket.h> is defined.  Also, Issue 6 is
# "aligned" with and based on the C99 standard, and it seems to have
# the most widespread support.  (MacOS is SUSv3-certified.)
# For more information on these extremely confusing "standards,"
# check https://www.man7.org/linux/man-pages/man7/standards.7.html
POSIX_VER ?= 200112L

# LLVM's target triplets are more straightforward for cross-compiling;
# that is why Blitzping uses Clang by default.  However, this codebase
# is very portable, and you can still use GCC if you prefer.
#
# For building, llvm, clang, lld, and llvm-binutils will be required.
#
# Compiler (gcc or clang)
CC := clang
# Linker
LD := lld
# Strip tool
STRIP := llvm-strip
# Clang tidy tool
TIDY := clang-tidy


# Directories
SRCDIR := ./src
OBJDIR := ./build
OUTDIR := ./out
# Files
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) \
	$(filter $(subst *,%,$2),$d))
SRCS := $(call rwildcard,$(SRCDIR)/,*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# ----------------------------------------------------------------------

# NOTE: Do NOT insert additional spaces after function commas;
# makefiles are very sensitive to whitespaces and tabs.

# Compiler options
#
# NOTE: -Ofast can result in "illegal instructions" on some targets.
CCOPT = \
	-std=$(C_STD) -fhosted -D _POSIX_C_SOURCE=$(POSIX_VER) \
	-Wall -Wextra -Wpedantic -Werror -pedantic-errors \
	-MMD -MP \
	-O3 -flto -D NDEBUG

# Add additional compatibility options if C99 is specified
ifeq ($(C_STD),c99)
	CCOPT += -Wno-unknown-warning-option \
		-Wno-c99-c11-compat -Wno-c11-extensions
endif

# Add triple and architecture to CCOPT if they aren't empty.
ifneq ($(TARGET),)
	CCOPT += --target=$(TARGET)
endif
ifneq ($(SUBARCH),)
	CCOPT += -march=$(SUBARCH)
endif

# Linker options
#
# For the host machine's compiler runtime (i.e., --rtlib, which is
# different from the target's libc that could be musl, glibc, uclibc,
# msvcrt, etc.), you could use either LLVM's 'compiler-rt' (apt install
# libclang-rt-dev:{arch}) OR 'libgcc' (apt install libgcc1-{arch}-cross)
# The 'platform' option will automatically choose an available one.
# (GCC lacks this option and always uses libgcc.)
LDOPT = \
	#-static -static-libgcc -lpthread

ifneq (,$(findstring gcc,$(CC)))
	# NOTE: GCC only works with LLVM's lld in non-LTO mode.
	CCOPT += -ffat-lto-objects
	LDOPT += -fuse-ld=$(LD)
else
	LDOPT += -fuse-ld=$(LD) --rtlib=platform --unwindlib=none
endif

# NOTE: Make sure to review the following LLVM/Clang bug (by myself):
# https://github.com/llvm/llvm-project/issues/102259
# In short, soft-core MIPS targets need more work to get built properly;
# if TARGET ends with 'sf' then we have to apply additional patches.

# If TARGET hasn't been specified (i.e., no cross-compilation), fill
# it with the current/host machine's info from -dumpmachine.
ifeq ($(TARGET),)
	TARGET := $(shell $(CC) -dumpmachine)
endif

# Extract the parts from TARGET, separated by dashes.
PARTS := $(subst -, ,$(TARGET))

# Get the number of words in TARGET, which could actually be in a 
# quadruplet (e.g., mips-openwrt-linux-musl vs. mips-linux-musl) format.
TRIPLET_N := $(words $(PARTS))

# Extract the libc and arch from TARGET.
ARCH := $(word 1, $(PARTS))
LIBC := $(subst sf,,$(lastword $(PARTS)))

# Extract the architecture, [vendor], os, and libc from the TARGET.
ifeq ($(TRIPLET_N),3) # Triplet
	VENDOR := unknown
	OS := $(word 2,$(PARTS))
else ifeq ($(TRIPLET_N),4) # Quadruplet
	VENDOR := $(word 2,$(PARTS))
	OS := $(word 3,$(PARTS))
endif

# Check if TARGET ends with 'sf' (soft-float)
FLOAT_ABI := $(if $(findstring sf,$(TARGET)),soft,hard)

# Patches for soft-core targets.
ifeq ($(FLOAT_ABI),soft)
	CCOPT += -msoft-float -flto
	# Append "-sf" to the libc name for the dynamic linker.
	LDOPT += -Wl,--dynamic-linker=/lib/ld-$(LIBC)-$(ARCH)-sf.so.1
endif

# Make the aforementioned info available to the C sources.
CCOPT += -D TARGET_ARCH=\"$(ARCH)\" -D TARGET_LIBC=\"$(LIBC)\" \
	-D TARGET_VENDOR=\"$(VENDOR)\" -D TARGET_OS=\"$(OS)\" \
	-D TARGET_TRIPLET=\"$(TARGET)\" \
	-D TARGET_FLOAT_ABI=\"$(FLOAT_ABI)\"
ifneq ($(SUBARCH),)
	CCOPT += -D TARGET_SUBARCH=\"$(SUBARCH)\"
else
	CCOPT += -D TARGET_SUBARCH=\"generic\"
endif
ifeq ($(FLOAT_ABI),soft)
	CCOPT += -D TARGET_SOFT_FLOAT=1
else
	CCOPT += -D TARGET_SOFT_FLOAT=0
endif


#
# Make Targets
#
.PHONY: all strip clean help tidy
all: $(OUTDIR)/$(NAME)

help:
	@echo "Targets ~"
	@echo "  help  : Print this help message."
	@echo "  all   : Build the Program."
	@echo "  strip : Strip debug info from the built program."
	@echo "  clean : Remove all built artefacts."

$(OUTDIR)/$(NAME): $(OBJS)
	$(CC) $(CCOPT) $(LDOPT) -o $@ $^
	@file $(OUTDIR)/$(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CCOPT) -c $< -o $@

strip: $(OUTDIR)/$(NAME)
	@ls -l $(OUTDIR)/$(NAME)
	$(STRIP) $(OUTDIR)/$(NAME)
	@ls -l $(OUTDIR)/$(NAME)
	@file $(OUTDIR)/$(NAME)

tidy:
	$(TIDY) $(SRCS) --

clean:
	rm -rf $(OBJDIR)/* $(OUTDIR)/$(NAME)

-include $(DEPS)

# ----------------------------------------------------------------------
