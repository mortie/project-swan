OUT ?= ./build
MESON ?= meson

.PHONY: all
all: build

$(OUT)/build.ninja:
	$(MESON) setup $(OUT) \
		-Dbuildtype=debug -Ddebug=true -Doptimization=1 \
		-Dprefix=$(abspath $(OUT)/pfx)

$(OUT)/swan: $(OUT)/build.ninja phony
	ninja -C $(OUT) swan core.mod/mod.so

$(OUT)/libswan/libswan_test: $(OUT)/build.ninja phony
	ninja -C $(OUT) libswan/libswan_test

SRCS = $(shell find include src core.mod libswan libcygnet -name '*.cc' -or -name '*.h')

.PHONY: llrun
llrun:
	${MAKE} run CMD='lldb --'

.PHONY: run
run: pfx
	cd $(OUT)/pfx && $(CMD) ./bin/swan --mod $(abspath core.mod) --world default.swan

.PHONY: launcher
launcher: pfx
	cd $(OUT)/pfx && $(CMD) ./bin/swan-launcher

.PHONY: build
build: $(OUT)/build.ninja
	ninja -C $(OUT)

.PHONY: pfx
pfx: build
	ninja -C $(OUT) install >/dev/null

.PHONY: core.mod
core.mod: pfx
	cd $(OUT)/pfx && ./bin/swan-build $(abspath core.mod) .

.PHONY: setup
setup: $(OUT)/build.ninja

.PHONY: check
check: $(OUT)/libswan/libswan_test
	cd $(OUT) && ./libswan/libswan_test

.PHONY: clean
clean:
	ninja -C $(OUT) clean

.PHONY: cleanall
cleanall:
	rm -rf $(OUT)

.PHONY: format
format:
	uncrustify -c uncrustify.cfg -l cpp --replace --no-backup $(SRCS)

.PHONY: clock
cloc:
	cloc $(SRCS)

.PHONY: phony
phony:
