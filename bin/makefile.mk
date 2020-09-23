VPATH=..\src

include tools.mk
include extra.mk
include rules.mk

build: test.img
	$(call msg,$@,$^)
	
run: build exec.test.img
	$(call msg,$@,$^)
	
exec.%: %
	$(call msg,$@,$^)
	qemu-system-x86_64 -drive if=floppy,format=raw,file=$^

test.img: STARTUP.BIN
test.img: BOOTSECTOR=../tools/flp144.bin

STARTUP.BIN: test.com
	$(call msg,$@,$^)
	copy $< $@

LINK_OPTIONS=-mt -os -q -bcl=dos

test.com: $(wildcard ../src/*.c ../src/*.h)

clean::
	$(call msg,$@,$^)
	$(call RM,STARTUP.BIN)

.PHONY: build run clean
