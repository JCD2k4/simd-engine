# Top-level driver. The benchmark sources live in the variant subdirectories
# (experiments/, naive/, optimized/); each has its own makefile that compiles
# ../parquet_loader.cpp alongside its own main.cpp. The shared loader sources
# and the parquet file live here.

CXX=g++
# Arrow's headers require C++20, so the loader gets its own standard. Nothing in
# parquet_loader.hpp leaks Arrow types, so the variant dirs stay C++17.
PARQUET_CXXFLAGS=-Wall -O2 -march=native -std=c++20 -I$(HOME)/.local/include

# Every subdirectory with a makefile...
SUBDIRS := $(patsubst %/makefile,%,$(wildcard */makefile))
# ...but only build the ones whose main.cpp actually has something in it, so an
# empty placeholder doesn't fail the whole tree. A dir joins in automatically
# once you write its main.cpp.
BUILDABLE := $(foreach d,$(SUBDIRS),$(if $(shell test -s $(d)/main.cpp && echo y),$(d)))

.PHONY: all clean loader $(SUBDIRS)

all: $(BUILDABLE)

$(SUBDIRS):
	$(MAKE) -C $@

# Compile the loader on its own -- a quick syntax check without a main.cpp.
loader: parquet_loader.o

parquet_loader.o: parquet_loader.cpp parquet_loader.hpp
	$(CXX) $(PARQUET_CXXFLAGS) -c parquet_loader.cpp -o parquet_loader.o

clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done
	rm -f parquet_loader.o
