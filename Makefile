# ----------------------------------------------------------------------

# Program name
TARGET = blitzping
# Target triplet (https://wiki.osdev.org/Target_Triplet)
TRIPLET ?= #mips-openwrt-linux-muslsf
# Target sub-architecture
SUBARCH ?= 


# LLVM's target triplets are more straightforward for cross-compiling;
# that is why Blitzping uses Clang by default.  However, this codebase
# is C11-compliant, and you can still use GCC if you prefer.
#
# For building, llvm, clang, lld, and llvm-binutils will be required.
#
# Compiler
CC = clang
# Linker
LD = lld
# Strip tool
STRIP = llvm-strip

# Directories
SRCDIR = ./src
OBJDIR = ./build
OUTDIR = ./out
# Files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS = $(OBJS:.o=.d)


# Compiler options
#
# NOTE: -Ofast can result in "illegal instructions" on some targets.
# TODO: -ffreestanding?
CCOPT = \
	-std=c11 -ffreestanding -D NDEBUG -D _POSIX_C_SOURCE=200809L \
	-Wall -Wextra -Wpedantic -Werror -pedantic-errors \
	-MMD -MP \
	-O3 -flto

# Add triple and architecture to CCOPT if they aren't empty.
ifneq ($(TRIPLET),)
	CCOPT += --target=$(TRIPLET)
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
LDOPT = \
	-flto \
	-fuse-ld=$(LD) --rtlib=platform --unwindlib=none \
	#-static -static-libgcc -lpthread


# NOTE: Make sure to review the following LLVM/Clang bug (by myself):
# https://github.com/llvm/llvm-project/issues/102259
# In short, soft-core MIPS targets need more work to get built properly;
# if TRIPLET ends with 'sf' then we have to apply additional patches.
#
# NOTE: Also, do NOT insert additional spaces after function commas;
# makefiles are very sensitive to whitespaces and tabs.

# Extract the parts from TRIPLET, separated by dashes.
PARTS := $(subst -, ,$(TRIPLET))

# Get the number of words in TRIPLET, which could actually be in a 
# quadruplet (e.g., mips-openwrt-linux-musl vs. mips-linux-musl) format.
TRIPLET_N := $(words $(PARTS))

# Extract the libc and arch from TRIPLET.
ARCH := $(word 1, $(PARTS))
LIBC := $(subst sf,,$(lastword $(PARTS)))

# Extract the architecture, [vendor], os, and libc from the "triplet."
ifeq ($(TRIPLET_N),3) # Triplet
	VENDOR := unknown
	OS := $(word 2,$(PARTS))
else ifeq ($(TRIPLET_N),4) # Quadruplet
	VENDOR := $(word 2,$(PARTS))
	OS := $(word 3,$(PARTS))
endif

# Check if TRIPLET ends with 'sf' (soft-float)
SOFT_FLOAT := $(if $(findstring sf,$(TRIPLET)),true,false)

# Patches for soft-core targets.
ifeq ($(SOFT_FLOAT),true)
	CCOPT += -msoft-float -flto
	# Append "-sf" to the libc name for the dynamic linker.
	LDOPT += -Wl,--dynamic-linker=/lib/ld-$(LIBC)-$(ARCH)-sf.so.1
endif


#
# Make Targets
#
.PHONY: all strip clean help
all: $(OUTDIR)/$(TARGET)

help:
	@echo "Targets ~"
	@echo "  help  : Print this help message."
	@echo "  all   : Build the Program."
	@echo "  strip : Strip debug info from the built program."
	@echo "  clean : Remove all built artefacts."

$(OUTDIR)/$(TARGET): $(OBJS)
	$(CC) $(CCOPT) $(LDOPT) -o $@ $^
	@file $(OUTDIR)/$(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CCOPT) -c $< -o $@

strip: $(OUTDIR)/$(TARGET)
	@ls -l $(OUTDIR)/$(TARGET)
	$(STRIP) $(OUTDIR)/$(TARGET)
	@ls -l $(OUTDIR)/$(TARGET)
	@file $(OUTDIR)/$(TARGET)

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d $(OUTDIR)/$(TARGET)

-include $(DEPS)

# ----------------------------------------------------------------------
