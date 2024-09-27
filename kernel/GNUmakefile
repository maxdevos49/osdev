# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

# This is the name that our final executable will have.
# Change as needed.
override OUTPUT := kernel

override KCC := x86_64-elf-gcc
override KLD := x86_64-elf-ld

# Internal C flags that should not be changed by the user.
override KCFLAGS += \
	-Wall \
	-Wextra \
	-Werror \
	-std=gnu11 \
	-nostdinc \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-fno-PIC \
	-ffunction-sections \
	-fdata-sections \
	-g \
	-pipe \
	-m64 \
	-march=x86-64 \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone \
	-mcmodel=kernel

# Internal C preprocessor flags that should not be changed by the user.
override KCPPFLAGS := \
	-I src \
	-isystem freestanding-headers \
	-MMD \
	-MP

# Internal nasm flags that should not be changed by the user.
override KNASMFLAGS += \
	-Wall \
	-gdwarf \
	-f elf64\
	-w-reloc-rel-dword \
	-w-reloc-abs-dword

# Internal linker flags that should not be changed by the user.
override KLDFLAGS += \
	-m elf_x86_64\
	-nostdlib \
	-static \
	-g \
	-z max-page-size=0x1000 \
	-gc-sections \
	-T linker-x86_64.ld

# Use "find" to glob all *.c, *.S, and *.asm files in the tree and obtain the
# object and header dependency file names.
override CFILES := $(shell cd src && find -L * -type f -name '*.c' | LC_ALL=C sort)
override ASFILES := $(shell cd src && find -L * -type f -name '*.S' | LC_ALL=C sort)
override NASMFILES := $(shell cd src && find -L * -type f -name '*.asm' | LC_ALL=C sort)
override OBJ := $(addprefix obj-x86_64/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o))
override OBJ += $(addprefix obj-x86_64/,$(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix obj-x86_64/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Ensure the dependencies have been obtained.
override MISSING_DEPS := $(shell if ! test -d freestanding-headers || ! test -f src/cc-runtime.c || ! test -f src/limine.h; then echo 1; fi)
ifeq ($(MISSING_DEPS),1)
	$(error Please run the ./get-deps script first)
endif

# Default target.
.PHONY: all
all: bin-x86_64/$(OUTPUT)

# Link rules for the final executable.
bin-x86_64/$(OUTPUT): GNUmakefile linker-x86_64.ld $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(KLD) $(OBJ) $(KLDFLAGS) -o $@

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files.
obj-x86_64/%.c.o: src/%.c GNUmakefile
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
obj-x86_64/%.S.o: src/%.S GNUmakefile
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
obj-x86_64/%.asm.o: src/%.asm GNUmakefile
	mkdir -p "$$(dirname $@)"
	nasm $(KNASMFLAGS) $< -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf bin-x86_64 obj-x86_64

# Remove everything built and generated including downloaded dependencies.
.PHONY: distclean
distclean:
	rm -rf bin-* obj-* freestanding-headers src/cc-runtime.c src/limine.h

# Install the final built executable to its final on-root location.
.PHONY: install
install: all
	install -d "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"
	install -m 644 bin-x86_64/$(OUTPUT) "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-x86_64"
