SRC_DIR=../src

build:

include tools.mk
include extra.mk
include rules.mk

build: rados.img
	$(call msg,$@,$^)
	
run: build exec.rados.img
	$(call msg,$@,$^)
	
exec.%: %
	$(call msg,$@,$^)
	qemu-system-x86_64 -rtc base=localtime -drive if=floppy,format=raw,file=$^

rados.img: STARTUP.BIN
rados.img: BOOTSECTOR=../tools/flp144.bin

STARTUP.BIN: rados.com
	$(call msg,$@,$^)
	copy $< $@

LINK_OPTIONS=-mt -os -q -s -bt=dos

rados.com: $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/**/*.c)

rados.c: $(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/**/*.h)

clean::
	$(call msg,$@,$^)
	$(call RM,STARTUP.BIN)

.PHONY: build run clean
