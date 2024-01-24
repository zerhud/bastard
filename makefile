mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))
path := $(mkfile_dir)
dep := g++ -MMD -MP -pipe -std=gnu++23
gcc := g++ -MMD -MP -pipe -fwhole-program -march=native -std=gnu++23 -fdiagnostics-color=always
clang := clang++ -MMD -pipe -march=native -std=gnu++23 -fdiagnostics-color=always
odir := tests

.PHONY: all clean directories

all: $(odir)/absd_test $(odir)/absd_test_clang $(odir)/jiexpr_ct_test
clean:
	rm -fr $(odir)


$(odir)/absd_test: makefile $(path)absd/test.cpp | directories
	$(gcc) -I$(path) $(path)/absd/test.cpp -o $(odir)/absd_test
	./$(odir)/absd_test
$(odir)/absd_test_clang: makefile $(path)absd/test.cpp | directories
	$(clang) -I$(path) $(path)/absd/test.cpp -o $(odir)/absd_test_clang
	./$(odir)/absd_test_clang

$(odir)/jiexpr_ct_test: makefile $(path)/jiexpr/test_ct.cpp | directories
	$(gcc) -I$(path) -I$(path)absd $(path)/jiexpr/test_ct.cpp -o $(odir)/jiexpr_ct_test
	./$(odir)/jiexpr_ct_test
$(odir)/jiexpr_ct_test_clang: makefile $(path)/jiexpr/test_ct.cpp | directories
	$(clang) -I$(path) -I$(path)absd $(path)/jiexpr/test_ct.cpp -o $(odir)/jiexpr_ct_test_clang
	./$(odir)/jiexpr_ct_test_clang


-include $(odir)/*.d
directories: $(odir)
$(odir):
	mkdir -p $(odir)
