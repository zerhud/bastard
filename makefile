builddir := make_build
tests_excluded_for_clang := "absd/callable jiexpr/math jiexpr/objects jiexpr/methods jiexpr/logic ast_graph/query"
GCC := g++ -MMD -pipe -std=gnu++23 -fwhole-program -march=native -fdiagnostics-color=always
CLANG := clang++ -MMD -pipe -std=gnu++23 -march=native -fdiagnostics-color=always -ftemplate-backtrace-limit=0

tests_src := $(shell find . -ipath '*/tests/*.cpp' | sed 's/^..//g')

.PHONY: all

base = $(basename $(subst tests/,,$(1)))

all: $(foreach src_file,$(tests_src),$(call base,$(src_file)))
	@echo -e "\e[7;32mAll Done\e[0m"

define create_test_dir_template
$(1):
	mkdir -p $$@
endef

define create_test_template
-include $(builddir)/$(call base,$(1))_gcc.d
$(builddir)/$(call base,$(1))_gcc: makefile $(1) | $(builddir)/$(dir $(call base,$(1)))
	$(GCC) -I./absd -Ijiexpr -Iast_graph $(1) -o $$@
-include $(builddir)/$(call base,$(1))_clang.d
ifeq ($(findstring $(call base,$(1)),$(tests_excluded_for_clang)),)
$(builddir)/$(call base,$(1))_clang: makefile $(1) | $(builddir)/$(dir $(call base,$(1)))
	@echo build for $(call base,$(1))
	$(CLANG) -I./absd -Ijiexpr -Iast_graph $(1) -o $$@
else
$(builddir)/$(call base,$(1))_clang: makefile $(1)
	@echo -e "\033[0;31mskiping for \033[1;36m$(1)\033[0;31m for clang\033[0m"
endif
.PHONY: $(call base,$(1))
$(call base,$(1)): $(builddir)/$(call base,$(1))_gcc $(builddir)/$(call base,$(1))_clang
	$(builddir)/$(call base,$(1))_gcc
	test -f $(builddir)/$(call base,$(1))_clang && $(builddir)/$(call base,$(1))_clang || true
clean::
	rm -f $(builddir)/$(call base,$(1))_gcc{,.d}
	rm -f $(builddir)/$(call base,$(1))_clang{,.d}

endef

directories := $(sort $(foreach src_file,$(tests_src),$(builddir)/$(dir $(call base,$(src_file)))))
$(foreach src_file,$(directories), $(eval $(call create_test_dir_template,$(src_file))))
$(foreach src_file,$(tests_src),$(eval $(call create_test_template,$(src_file))))

