# m4xdevOS

x86_64 kernel with a goal of getting to userspace and porting Doom. Further plans include making small games for the OS and treating it like a custom game console OS.

### Development Resources:
- [AMD Manual](https://www.amd.com/content/dam/amd/en/documents/processor-tech-docs/programmer-references/24593.pdf)
- Intel Manual
- [OS Dev Wiki](https://wiki.osdev.org/Expanded_Main_Page)
- [Limine Protocol](https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md)
- https://cs61.seas.harvard.edu/wiki/2017/Kernel2/
- https://cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
- [Ascii Table](https://www.ascii-code.com/)
- [My first kernel](https://github.com/maxdevos49/MadOS)
- https://os.phil-opp.com/paging-implementation/#recursive-page-tables
- https://gcc.gnu.org/wiki/FAQ#configure
- https://roscopeco.com/2018/11/25/using-gcc-as-cross-compiler-with-x86_64-target/
- https://www.minix3.org/theses/gerofi-minix-vfs.pdf
- http://kos.enix.org/pub/sunos-vi.pdf
- https://maups.github.io/papers/tcc_004.pdf
- https://www.sunhelp.org/history/pdf/unix_filesys_extent_like_perf.pdf

## How to use this?

### Dependencies

Any `make` command depends on GNU make (`gmake`) and is expected to be run using it. This usually means using `make` on most GNU/Linux distros, or `gmake` on other non-GNU systems.

It is recommended to build this project using a standard UNIX-like system, using a Clang/LLVM toolchain capable of cross compilation (the default, unless `KCC` and/or `KLD` are explicitly set).

Additionally, building an ISO with `make all` requires `xorriso`, and building a HDD/USB image with `make all-hdd` requires `sgdisk` (usually from `gdisk` or `gptfdisk` packages) and `mtools`.

### Makefile targets

Running `make all` will compile the kernel (from the `kernel/` directory) and then generate a bootable ISO image.

Running `make all-hdd` will compile the kernel and then generate a raw image suitable to be flashed onto a USB stick or hard drive/SSD.

Running `make run` will build the kernel and a bootable ISO (equivalent to make all) and then run it using `qemu` (if installed).

Running `make run-hdd` will build the kernel and a raw HDD image (equivalent to make all-hdd) and then run it using `qemu` (if installed).

For x86_64, the `run-bios` and `run-hdd-bios` targets are equivalent to their non `-bios` counterparts except that they boot `qemu` using the default SeaBIOS firmware instead of OVMF.
