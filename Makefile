# Makefile generated by smake.

PROJNAME = cpplat
PROJTYPE = exe

SRCS = src/Body.cc src/Game.cc src/main.cc src/Player.cc src/WorldPlane.cc
HDRS = src/Body.h src/common.h src/Game.h src/Player.h src/WorldPlane.h
OBJS = $(patsubst src/%,$(BUILD)/obj/%.o,$(SRCS))
DEPS = $(patsubst src/%,$(BUILD)/dep/%.d,$(SRCS))
PUBLICHDRS =

# Defaults
PKGS =
EXTRADEPS =
EXTRAPUBLICDEPS =
CONFIG ?= release
BUILDDIR ?= build
BUILD ?= $(BUILDDIR)/$(CONFIG)
WARNINGS = -Wall -Wextra -Wno-unused-parameter
SMAKEFILE ?= Smakefile
DESTDIR ?=
PREFIX ?= /usr/local
PHONIES = dumpdeps dumppublicdeps dumpprojtype clean cleanall
CONFIGS = release sanitize debug

PKG_CONFIG ?= pkg-config
AR ?= ar

CCOPTS = -Iinclude
CCOPTS_release = -O2 -flto
CCOPTS_debug = -g
CCOPTS_sanitize = -fsanitize=address -fsanitize=undefined $(CCOPTS_debug)

LDOPTS =
LDOPTS_release = -flto
LDOPTS_debug =
LDOPTS_sanitize = -fsanitize=address -fsanitize=undefined $(LDOPTS_debug)

runpfx = @echo $(1) $(2) && $(2)

.PHONY: all
all: $(BUILD)/$(PROJNAME)

-include $(SMAKEFILE)

ifeq ($(filter $(CONFIGS),$(CONFIG)),)
ifeq ($(filter cleanall,$(MAKECMDGOALS)),)
$(error Unknown config '$(CONFIG)'. Supported configs: $(CONFIGS))
endif
endif

ifneq ($(PKGS),)
CCOPTS += $(shell $(PKG_CONFIG) --cflags $(PKGS))
LDOPTS += $(shell $(PKG_CONFIG) --libs $(PKGS))
endif

CFLAGS := $(CCOPTS_$(CONFIG)) $(CCOPTS) $(WARNINGS) $(CFLAGS)
CXXFLAGS := $(CCOPTS_$(CONFIG)) $(CCOPTS) $(WARNINGS) $(CXXFLAGS)
LDFLAGS := $(LDOPTS_$(CONFIG)) $(LDOPTS) $(LDFLAGS)

$(BUILD)/$(PROJNAME):  $(OBJS)
	@mkdir -p $(@D)
	$(call runpfx,'(LD)',$(CC) -o $@ $(OBJS) $(LDFLAGS))
	touch $(BUILD)/.built
	@echo '(OK)' Created $@.
$(PROJNAME): $(BUILD)/$(PROJNAME)
	cp $< $@
	@echo '(OK)' Created $@.

$(BUILD)/obj/%.c.o: src/%.c $(EXTRAPUBLICDEPS)
	@mkdir -p $(@D)
	$(call runpfx,'(CC)',$(CC) -o $@ -c $< $(CFLAGS))
$(BUILD)/obj/%.cc.o: src/%.cc $(EXTRAPUBLICDEPS)
	@mkdir -p $(@D)
	$(call runpfx,'(CXX)',$(CXX) -o $@ -c $< $(CXXFLAGS))

$(BUILD)/dep/%.c.d: src/%.c $(HDRS)
	@mkdir -p $(@D)
	$(call runpfx,'(DEP)',$(CC) -o $@ -MM $< -MT $(patsubst src/%,$(BUILD)/obj/%.o,$<) $(CFLAGS))
$(BUILD)/dep/%.cc.d: src/%.cc $(HDRS)
	@mkdir -p $(@D)
	$(call runpfx,'(DEP)',$(CXX) -o $@ -MM $< -MT $(patsubst src/%,$(BUILD)/obj/%.o,$<) $(CXXFLAGS))

.PHONY: install
install: $(BUILD)/$(PROJNAME)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $^ $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(PROJNAME)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(PROJNAME)

.PHONY: clean
clean:
	rm -rf $(PROJNAME) $(BUILD)

.PHONY: cleanall
cleanall:
	rm -rf $(BUILDDIR)

.PHONY: dumpdeps
dumpdeps:
	@echo $(addprefix $(PREPEND),$(SRCS) $(HDRS) $(EXTRADEPS))

.PHONY: dumppublicdeps
dumppublicdeps:
	@echo $(addprefix $(PREPEND),$(PUBLICHDRS) $(EXTRAPUBLICDEPS))

.PHONY: dumpprojtype
dumpprojtype:
	@echo $(PROJTYPE)

ifeq ($(filter $(PHONIES),$(MAKECMDGOALS)),)
-include $(DEPS)
endif
