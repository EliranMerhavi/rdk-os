target=OS.bin

includes= -I./src -I./src/lib/

flags= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-unsused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

src=src
obj=obj
bin=bin

os_asm_sources  :=  $(wildcard $(src)/os/*.asm)         \
                    $(wildcard $(src)/os/*/*.asm)       \
                    $(wildcard $(src)/os/*/*/*.asm)     \
                    $(wildcard $(src)/os/*/*/*/*.asm)   \

os_cpp_sources  :=  $(wildcard $(src)/os/*.cpp)         \
                    $(wildcard $(src)/os/*/*.cpp)       \
                    $(wildcard $(src)/os/*/*/*.cpp)     \
                    $(wildcard $(src)/os/*/*/*/*.cpp)   \
                    $(wildcard $(src)/os/*/*/*/*/*.cpp) \

lib_cpp_sources :=  $(wildcard $(src)/lib/*.cpp)        \
                    $(wildcard $(src)/lib/*/*.cpp)      \
                    $(wildcard $(src)/lib/*/*/*.cpp)    \
                    $(wildcard $(src)/lib/*/*/*/*.cpp)

os_objects := $(addprefix $(obj)/,  $(addsuffix .asm.o, $(basename $(notdir $(os_asm_sources))))) \
		      $(obj)/lib.o \
			  $(addprefix $(obj)/,  $(addsuffix .o,     $(basename $(notdir $(os_cpp_sources)))))

lib_objects := $(addprefix $(obj)/, $(addsuffix .o,    $(basename $(notdir $(lib_cpp_sources)))))

bins := $(bin)/boot.bin $(bin)/kernel.bin

$(bin)/$(target): $(bins) user_programs
	dd if=$(bin)/boot.bin >> $@
	dd if=$(bin)/kernel.bin >> $@
	dd if=/dev/zero bs=1048576 count=16 >> $@
	sudo mount -t vfat $@ ./mnt/drive0
	@sudo mkdir ./mnt/drive0/testing
	@echo "hello in folder" | sudo tee ./mnt/drive0/testing/hello.txt > /dev/null
	@echo "hello " 			| sudo tee ./mnt/drive0/hello.txt 		  > /dev/null
	@sudo cp $(src)/programs/shell/bin/shell.elf ./mnt/drive0 
	@sudo umount ./mnt/drive0


$(bin)/kernel.bin: $(os_objects)
	@echo "linking to $@..."
	@i686-elf-ld -g -relocatable $(os_objects) -o $(obj)/full_kernel.o
	@i686-elf-g++ $(flags) -T $(src)/linker.ld -o $@ $(obj)/full_kernel.o


$(bin)/boot.bin: $(src)/boot/boot.asm
	@echo "assembling to binary $<..."
	@nasm -f bin $< -o $@	


$(obj)/lib.o: $(lib_objects)
	@echo "linking $@..."
	@i686-elf-ld -g -relocatable $(lib_objects) -o $@


$(obj)/%.o: $(src)/lib/%.cpp
	@echo "compiling $<..."
	@i686-elf-g++ $(includes) $(flags) -c $< -o $@  


$(obj)/%.o: $(src)/lib/*/%.cpp
	@echo "compiling $<..."
	@i686-elf-g++ $(includes) $(flags) -c $< -o $@  


$(obj)/%.o: $(src)/lib/*/*/%.cpp
	@echo "compiling $<..."
	@i686-elf-g++ $(includes) $(flags) -c $< -o $@  


$(obj)/%.o: $(src)/os/%.cpp
	@echo "compiling $<..."
	@i686-elf-g++ $(includes) $(flags) -c $< -o $@  


$(obj)/%.o: $(src)/os/*/%.cpp
	@echo "compiling $<..."
	@i686-elf-g++ $(includes) $(flags) -c $< -o $@  


$(obj)/%.o: $(src)/os/*/*/%.cpp
	@echo "compiling $<..."
	@i686-elf-g++ $(includes) $(flags) -c $< -o $@  


$(obj)/%.o: $(src)/os/*/*/*/%.cpp
	@echo "compiling $<..."
	@i686-elf-g++ $(includes) $(flags) -c $< -o $@  


$(obj)/%.o: $(src)/os/*/*/*/*/%.cpp
	@echo "compiling $<..."
	@i686-elf-g++ $(includes) $(flags) -c $< -o $@  


$(obj)/%.o: $(src)/os/*/*/*/*/*/%.cpp
	@echo "compiling $<..."
	@i686-elf-g++ $(includes) $(flags) -c $< -o $@  


$(obj)/%.asm.o: $(src)/os/*/%.asm
	@echo "assembling $<..."	
	@nasm -f elf -g $< -o $@


$(obj)/%.asm.o: $(src)/os/*/*/%.asm
	@echo "assembling $<..."
	@nasm -f elf -g $< -o $@


$(obj)/%.asm.o: $(src)/os/*/*/*/%.asm
	@echo "assembling $<..."
	@nasm -f elf -g $< -o $@


$(obj)/%.asm.o: $(src)/os/*/*/*/*/%.asm
	@echo "assembling $<..."
	@nasm -f elf -g $< -o $@


user_programs:
	cd $(src)/programs/stdlib/ && make
	cd $(src)/programs/shell/ && make 


user_programs_clean:
	cd $(src)/programs/stdlib/ && make clean
	cd $(src)/programs/shell/ && make clean 


clean: user_programs_clean
	rm -rf $(bin)/*
	rm -rf $(obj)/*


print_objects:
	@echo $(lib_objects)
	@echo $(os_objects)


run:
	qemu-system-x86_64 -drive file=$(bin)/$(target),format=raw,index=0,media=disk
	@#qemu-system-x86_64 -hda $(bin)/$(target)


debug:
	@echo "commands to copy: "
	@echo "target remote | qemu-system-x86_64 -hda $(bin)/$(target) -S -gdb stdio"
	@echo "add-symbol-file $(obj)/full_kernel.o 0x100000"
	gdb 
