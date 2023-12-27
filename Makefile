OUT ?= ./build
PREFIX ?= $(abspath $(OUT)/pfx)

all: $(OUT)/swan

$(OUT)/build.ninja:
	meson setup $(OUT) -Dprefix=$(PREFIX) -Dbuildtype=debugoptimized

$(OUT)/swan: $(OUT)/build.ninja phony
	ninja -C $(OUT) swan

$(OUT)/libswan/libswan_test: $(OUT)/build.ninja phony
	ninja -C $(OUT) libswan/libswan_test

SRCS = $(shell find include src core.mod libswan libcygnet -name '*.cc' -or -name '*.h')

.PHONY: run
run: $(OUT)/swan
	ninja -C $(OUT) install
	cd $(PREFIX) && $(CMD) ./bin/swan

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
	uncrustify -c uncrustify.cfg --replace --no-backup $(SRCS)

.PHONY: phony
phony:
