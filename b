# if youre not on linux... good luck i guess

if [ "$1" = 'cln' ]; then
	rm nacp xmpnx* main.o main.d libxmp/obj/* libxmp/libxmp.o
	exit
fi

if [ "$1" = 'dep' ]; then
	cd libxmp
	export CC="/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE"
	./build
	cd ..
fi

ARCHFLAGS="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE"
EFLAGS="-g"

/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc \
-r main.c libxmp/libxmp.o \
$ARCHFLAGS \
$EFLAGS \
-MMD \
-MP \
-Wall \
-O3 \
-ffunction-sections \
-I/opt/devkitpro/portlibs/switch/include \
-I/opt/devkitpro/libnx/include \
-Ilibxmp \
-D__SWITCH__ \
-o main.o

/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc main.o \
$ARCHFLAGS \
-specs=/opt/devkitpro/libnx/switch.specs \
$EFLAGS \
-Wl,-Map,xmpnx.map \
-L/opt/devkitpro/portlibs/switch/lib \
-L/opt/devkitpro/libnx/lib \
-lnx \
-o xmpnx.elf \
`/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-pkg-config --libs SDL2_mixer`

# idk why this is needed
/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc-nm -CSn xmpnx.elf > xmpnx.lst

/opt/devkitpro/tools/bin/nacptool --create "xmpnx" "mothcompute" "0" nacp
/opt/devkitpro/tools/bin/elf2nro xmpnx.elf xmpnx.nro --nacp=nacp
