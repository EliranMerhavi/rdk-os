target=stdlib.elf 

includes= -I./src

flags= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

src=src
obj=obj
bin=bin

asm_sources := $(wildcard $(src)/*.asm)

c_sources   := $(wildcard $(src)/*.c)

objects 	:= $(addprefix $(obj)/, $(addsuffix .asm.o, $(basename $(notdir $(asm_sources))))) \
		   	   $(addprefix $(obj)/, $(addsuffix .o, 	$(basename $(notdir $(c_sources)))))

$(bin)/$(target): $(objects)
	i686-elf-ld -m elf_i386 -relocatable $(objects) -o $@

$(obj)/%.o: $(src)/%.c
	@echo "compiling $<..."
	i686-elf-gcc -I./ $(flags) -std=gnu99 -c $< -o $@

$(obj)/%.asm.o: $(src)/%.asm
	@echo "assembling $<..."	
	@nasm -f elf -g $< -o $@

clean:
	rm -rf ./obj/* ./bin/*
