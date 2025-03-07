# # kernel_source_files := $(shell find src/impl/kernel -name *.c)
# # kernel_object_files := $(patsubst src/impl/kernel/%.c, build/kernel/%.o, $(kernel_source_files))

# # x86_64_c_source_files := $(shell find src/impl/x86_64 -name *.c)
# # x86_64_c_object_files := $(patsubst src/impl/x86_64/%.c, build/x86_64/%.o, $(x86_64_c_source_files))

# # x86_64_asm_source_files := $(shell find src/impl/x86_64 -name *.asm)
# # x86_64_asm_object_files := $(patsubst src/impl/x86_64/%.asm, build/x86_64/%.o, $(x86_64_asm_source_files))

# # x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)

# # $(kernel_object_files): build/kernel/%.o : src/impl/kernel/%.c
# # 	mkdir -p $(dir $@) && \
# # 	x86_64-elf-gcc -c -I src/intf -ffreestanding $(patsubst build/kernel/%.o, src/impl/kernel/%.c, $@) -o $@

# # $(x86_64_c_object_files): build/x86_64/%.o : src/impl/x86_64/%.c
# # 	mkdir -p $(dir $@) && \
# # 	x86_64-elf-gcc -c -I src/intf -ffreestanding $(patsubst build/x86_64/%.o, src/impl/x86_64/%.c, $@) -o $@

# # $(x86_64_asm_object_files): build/x86_64/%.o : src/impl/x86_64/%.asm
# # 	mkdir -p $(dir $@) && \
# # 	nasm -f elf64 $(patsubst build/x86_64/%.o, src/impl/x86_64/%.asm, $@) -o $@

# # .PHONY: build-x86_64
# # build-x86_64: $(kernel_object_files) $(x86_64_object_files)
# # 	mkdir -p dist/x86_64 && \
# # 	x86_64-elf-ld -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld $(kernel_object_files) $(x86_64_object_files) && \
# # 	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin && \
# # 	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso

# kernel_source_files := $(shell find src/kernel -name *.c)
# kernel_object_files := $(patsubst src/kernel/%.c, build/kernel/%.o, $(kernel_source_files))

# x86_64_c_source_files := $(shell find src/ -name *.c)
# x86_64_c_object_files := $(patsubst src/%.c, build/%.o, $(x86_64_c_source_files))

# x86_64_asm_source_files := $(shell find src/ -name *.asm)
# # Modified this line to append _asm to assembly object files
# x86_64_asm_object_files := $(patsubst src/%.asm, build/%_asm.o, $(x86_64_asm_source_files))

# x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)

# $(kernel_object_files): build/kernel/%.o : src/kernel/%.c
# 	mkdir -p $(dir $@) && \
# 	x86_64-elf-gcc -c -I src/ -ffreestanding $(patsubst build/kernel/%.o, src/kernel/%.c, $@) -o $@

# $(x86_64_c_object_files): build/%.o : src/%.c
# 	mkdir -p $(dir $@) && \
# 	x86_64-elf-gcc -c -I src/ -ffreestanding $(patsubst build/%.o, src/%.c, $@) -o $@

# # Modified this rule to match the _asm.o pattern
# $(x86_64_asm_object_files): build/%_asm.o : src/%.asm
# 	mkdir -p $(dir $@) && \
# 	nasm -f elf64 $(patsubst build/%_asm.o, src/%.asm, $@) -o $@

# .PHONY: build-x86_64
# build-x86_64: $(kernel_object_files) $(x86_64_object_files)
# 	mkdir -p dist/x86_64 && \
# 	x86_64-elf-ld -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld $(kernel_object_files) $(x86_64_object_files) && \
# 	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin && \
# 	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso

CFLAGS = -I src/ -ffreestanding -Wall -Wextra

kernel_source_files := $(shell find src/kernel -name *.c)
kernel_object_files := $(patsubst src/kernel/%.c, build/kernel/%.o, $(kernel_source_files))

libs_source_files := $(shell find src/libs -name *.c)
libs_object_files := $(patsubst src/libs/%.c, build/libs/%.o, $(libs_source_files))

cmds_source_files := $(shell find src/cmds -name *.c)
cmds_object_files := $(patsubst src/cmds/%.c, build/cmds/%.o, $(cmds_source_files))

asm_source_files := $(shell find src/ -name *.asm)
asm_object_files := $(patsubst src/%.asm, build/%_asm.o, $(asm_source_files))

object_files := $(kernel_object_files) $(libs_object_files) $(cmds_object_files) $(asm_object_files)

$(kernel_object_files): build/kernel/%.o : src/kernel/%.c
	mkdir -p $(dir $@) && \
	x86_64-elf-gcc $(CFLAGS) -c -ffreestanding $(patsubst build/kernel/%.o, src/kernel/%.c, $@) -o $@

$(libs_object_files): build/libs/%.o : src/libs/%.c
	mkdir -p $(dir $@) && \
	x86_64-elf-gcc $(CFLAGS) -c -ffreestanding $(patsubst build/libs/%.o, src/libs/%.c, $@) -o $@

$(cmds_object_files): build/cmds/%.o : src/cmds/%.c
	mkdir -p $(dir $@) && \
	x86_64-elf-gcc $(CFLAGS) -c -ffreestanding $(patsubst build/cmds/%.o, src/cmds/%.c, $@) -o $@

$(asm_object_files): build/%_asm.o : src/%.asm
	mkdir -p $(dir $@) && \
	nasm -f elf64 $(patsubst build/%_asm.o, src/%.asm, $@) -o $@

.PHONY: build-x86_64
build-x86_64: $(object_files)
	mkdir -p dist/x86_64 && \
	x86_64-elf-ld -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld $(object_files) && \
	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin && \
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso
