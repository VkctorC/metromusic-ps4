TITLE       := MetroMusic CustomMusicCore Probe
VERSION     := 0.13
TITLE_ID    := BREW00991
CONTENT_ID  := IV0000-BREW00991_00-METROMUSICPROBE0

LIBS        := -lc -lkernel

TOOLCHAIN   := $(OO_PS4_TOOLCHAIN)
INTDIR      := build
CFILES      := $(wildcard *.c)
OBJS        := $(patsubst %.c,$(INTDIR)/%.o,$(CFILES))

CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include
LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crt1.o

UNAME_S     := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
CC          := clang
LD          := ld.lld
CDIR        := linux
endif
ifeq ($(UNAME_S),Darwin)
CC          := /usr/local/opt/llvm/bin/clang
LD          := /usr/local/opt/llvm/bin/ld.lld
CDIR        := macos
endif

all: $(CONTENT_ID).pkg

eboot.bin: $(OBJS)
	$(LD) $(OBJS) -o $(INTDIR)/metro_music_probe.elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself -in=$(INTDIR)/metro_music_probe.elf -out=$(INTDIR)/metro_music_probe.oelf --eboot eboot.bin --paid 0x3800000000000011

$(INTDIR)/%.o: %.c
	@mkdir -p $(INTDIR)
	$(CC) $(CFLAGS) -o $@ $<

sce_module/libc.prx:
	@mkdir -p sce_module
	cp $(TOOLCHAIN)/bin/data/modules/libc.prx $@

sce_module/libSceFios2.prx:
	@mkdir -p sce_module
	cp $(TOOLCHAIN)/bin/data/modules/libSceFios2.prx $@

sce_sys/param.sfo: Makefile
	@mkdir -p sce_sys
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_new $@
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ APP_TYPE --type Integer --maxsize 4 --value 1
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ APP_VER --type Utf8 --maxsize 8 --value '$(VERSION)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ ATTRIBUTE --type Integer --maxsize 4 --value 0
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ CATEGORY --type Utf8 --maxsize 4 --value 'gd'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ CONTENT_ID --type Utf8 --maxsize 48 --value '$(CONTENT_ID)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ SYSTEM_VER --type Integer --maxsize 4 --value 0
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value '$(TITLE)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ TITLE_ID --type Utf8 --maxsize 12 --value '$(TITLE_ID)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ VERSION --type Utf8 --maxsize 8 --value '$(VERSION)'

pkg.gp4: eboot.bin sce_sys/param.sfo sce_sys/icon0.png sce_module/libc.prx sce_module/libSceFios2.prx
	$(TOOLCHAIN)/bin/$(CDIR)/create-gp4 -out $@ --content-id=$(CONTENT_ID) --files "$^"

$(CONTENT_ID).pkg: pkg.gp4
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core pkg_build $< .

clean:
	rm -rf build eboot.bin pkg.gp4 sce_module $(CONTENT_ID).pkg
