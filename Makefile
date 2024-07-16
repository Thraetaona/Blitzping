# ----------------------------------------------------------------------

# Target name
TARGET = blitzping

# Directories
SRCDIR = ./src
OBJDIR = ./build
OUTDIR = ./out

# Files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS = $(OBJS:.o=.d)

# Compiler
CC = clang

# Strip tool
STRIP = llvm-strip

# Compiler options
#
# GCC is so clunky when it comes to cross-compiling, and it's not worth
# the effort to maintain multi-architecture dependencies and toolchain;
# LLVM's target triplets are far more straightforward.
#
# For that, llvm, llvm-binutils, clang, and lld will be required.
#-D_POSIX_C_SOURCE=200809L
CCOPT = \
	-std=c11 -D_DEFAULT_SOURCE \
	-Wall -Wextra -Werror -pedantic-errors \
	-MMD -MP \
	-Ofast -flto -march=armv8-a \
	--target=aarch64-openwrt-linux-musl \

# Linker options
#
# NOTE: You can substitute rtlib and unwindlib for "libgcc," if you
# cannot find LLVM's alternatives for your target architecture.
# Depending on your target's libc (e.g., glibc vs. musl), you might
# also want to statistically link libgcc, in that case.
#
# LLVM's libclang-rt-dev:{arch} and libunwind-dev:{arch} are used here.
LDOPT = \
	-fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind \
	#-static -lpthread

#
# Targets
#
.PHONY: all strip clean help
all: $(OUTDIR)/$(TARGET)

help:
	@echo "Targets ~"
	@echo "  all   : Build the Program."
	@echo "  strip : Strip debug info from the built program."
	@echo "  clean : Remove all built artefacts."
	@echo "  help  : Print this help message."

$(OUTDIR)/$(TARGET): $(OBJS)
	$(CC) $(CCOPT) $(LDOPT) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CCOPT) -c $< -o $@

strip: $(OUTDIR)/$(TARGET)
	@ls -l $(OUTDIR)/$(TARGET)
	$(STRIP) $(OUTDIR)/$(TARGET)
	@ls -l $(OUTDIR)/$(TARGET)

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d $(OUTDIR)/$(TARGET)

-include $(DEPS)

# ----------------------------------------------------------------------
