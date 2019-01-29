# ################################################################
# Copyright (c) 2015-present, Yann Collet, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under both the BSD-style license (found in the
# LICENSE file in the root directory of this source tree) and the GPLv2 (found
# in the COPYING file in the root directory of this source tree).
# ################################################################
include ${R_HOME}/etc${R_ARCH}/Makeconf

PRGDIR   = programs
ZSTDDIR  = lib
BUILDIR  = build
ZWRAPDIR = zlibWrapper
TESTDIR  = tests
FUZZDIR  = $(TESTDIR)/fuzz

# Define nul output
VOID = /dev/null

ifneq (,$(filter Windows%,$(OS)))
EXT =.exe
else
EXT =
endif

## default: Build lib-release and zstd-release
.PHONY: default
default: lib-release zstd-release

.PHONY: all
all: allmost examples manual contrib

.PHONY: allmost
allmost: allzstd zlibwrapper

# skip zwrapper, can't build that on alternate architectures without the proper zlib installed
.PHONY: allzstd
allzstd: lib
	$(MAKE) -C $(PRGDIR) all
	$(MAKE) -C $(TESTDIR) all

.PHONY: all32
all32:
	$(MAKE) -C $(PRGDIR) zstd32
	$(MAKE) -C $(TESTDIR) all32

.PHONY: lib lib-release libzstd.a
lib lib-release :
	@$(MAKE) -C $(ZSTDDIR) $@

.PHONY: zstd zstd-release
zstd zstd-release:
	@$(MAKE) -C $(PRGDIR) $@
	cp $(PRGDIR)/zstd$(EXT) .

.PHONY: zstdmt
zstdmt:
	@$(MAKE) -C $(PRGDIR) $@
	cp $(PRGDIR)/zstd$(EXT) ./zstdmt$(EXT)

.PHONY: zlibwrapper
zlibwrapper: lib
	$(MAKE) -C $(ZWRAPDIR) all

## test: run long-duration tests
.PHONY: test
DEBUGLEVEL ?= 1
test: MOREFLAGS += -g -DDEBUGLEVEL=$(DEBUGLEVEL) -Werror
test:
	MOREFLAGS="$(MOREFLAGS)" $(MAKE) -j -C $(PRGDIR) allVariants
	$(MAKE) -C $(TESTDIR) $@

## shortest: same as `make check`
.PHONY: shortest
shortest:
	$(MAKE) -C $(TESTDIR) $@

## check: run basic tests for `zstd` cli
.PHONY: check
check: shortest

## examples: build all examples in `/examples` directory
.PHONY: examples
examples: lib
	CPPFLAGS=-I../lib LDFLAGS=-L../lib $(MAKE) -C examples/ all

## manual: generate API documentation in html format
.PHONY: manual
manual:
	$(MAKE) -C contrib/gen_html $@

## man: generate man page
.PHONY: man
man:
	$(MAKE) -C programs $@

## contrib: build all supported projects in `/contrib` directory
.PHONY: contrib
contrib: lib
	$(MAKE) -C contrib/pzstd all
	$(MAKE) -C contrib/seekable_format/examples all
	$(MAKE) -C contrib/adaptive-compression all
	$(MAKE) -C contrib/largeNbDicts all

.PHONY: cleanTabs
cleanTabs:
	cd contrib; ./cleanTabs

.PHONY: clean
clean:
	@$(MAKE) -C $(ZSTDDIR) $@ > $(VOID)
	@$(MAKE) -C $(PRGDIR) $@ > $(VOID)
#	@$(MAKE) -C $(TESTDIR) $@ > $(VOID)
#	@$(MAKE) -C $(ZWRAPDIR) $@ > $(VOID)
#	@$(MAKE) -C examples/ $@ > $(VOID)
#	@$(MAKE) -C contrib/gen_html $@ > $(VOID)
#	@$(MAKE) -C contrib/pzstd $@ > $(VOID)
#	@$(MAKE) -C contrib/seekable_format/examples $@ > $(VOID)
#	@$(MAKE) -C contrib/adaptive-compression $@ > $(VOID)
#	@$(MAKE) -C contrib/largeNbDicts $@ > $(VOID)
	@$(RM) zstd$(EXT) zstdmt$(EXT) tmp*
	@$(RM) -r lz4
	@echo Cleaning completed
