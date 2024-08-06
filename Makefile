# ----------------------------------------------------------------------

# Target name
TARGET = blitzping
# Target triplet
TRIPLET ?= aarch64-openwrt-linux-musl
# Target architecture
ARCH ?= armv8

# Directories
SRCDIR = ./src
OBJDIR = ./build
OUTDIR = ./out
# Files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS = $(OBJS:.o=.d)


# LLVM's target triplets are more straightforward for cross-compiling;
# that is why Blitzping uses Clang by default.  However, its codebase
# is C11-compliant, and you can still use GCC if you prefer.
#
# For building, llvm, llvm-binutils, clang, and lld will be required.
#
# Compiler
CC = clang
# Linker
LD = lld
# Strip tool
STRIP = llvm-strip

# Compiler options
#
# TODO: -ffreestanding?
CCOPT = \
	-std=c11 -ffreestanding -D NDEBUG -D _POSIX_C_SOURCE=200809L \
	-Wall -Wextra -Wpedantic -Werror -pedantic-errors \
	-MMD -MP \
	-Ofast -flto \
	--target=$(TRIPLET) -march=$(ARCH) \

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

#
# Targets
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
