# ======================================================================
# NOTE: make all
# -----
#
# ======================================================================
.PHONY: prep clean vars

# -- external dependency, cf https://github.com/ursl/util
UTIL = /Users/ursl/macros/ana/util
# -- rpath solutions.
PWD  = $(shell pwd)
BASE = $(abspath $(dir $(PWD)))

ROOTCINT      = $(ROOTSYS)/bin/rootcint
ROOTCFLAGS    = $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTGLIBS     = $(shell $(ROOTSYS)/bin/root-config --glibs)

ifdef SCRAM_ARCH
  CXX         := $(shell scramv1 tool info gcc-cxxcompiler | grep CXX= | sed s/CXX=//)
else
  ifdef CXXCOMPILER
    CXX         := $(CXXCOMPILER)
  else
    CXX         := c++
  endif
endif

ROOT6 = $(shell $(ROOTSYS)/bin/root-config --version)
ifneq ($(findstring 6.,$(ROOT6)),)
ROOT6         = "yes"
else
ROOT6         = "no"
endif

CXXFLAGS      = -g -O0 -Wall -fPIC -pipe
LD            = $(CXX)
LDFLAGS       = -g -rpath $(PWD)/lib
SOFLAGS       = -shared

CXXFLAGS     += $(ROOTCFLAGS)
GLIBS         = $(filter-out -lz, $(ROOTGLIBS)) -lTMVA -lTMVAGui -lRooFitCore -lRooFit
EXTHEADERS    = -I.. -I$(UTIL)


# -- Files
#CSTUFF =
READER = treeReader01.o plotResults.o # plotClass.o

DICT = ${READER:.o=Dict.o}
DICTHEADERS = ${READER:.o=Dict.h}
DICTSOURCES = ${READER:.o=Dict.cc}

# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

%Dict.cc : %.hh %LinkDef.h
	$(ROOTCINT) -f $@ -c $(EXTHEADERS) $^
ifeq ($(ROOT6),"yes")
	mv $(subst Dict.cc,Dict_rdict.pcm,$@) lib
endif

%Dict.cc : %.hh
	$(ROOTCINT) -f $@ -c $(EXTHEADERS) $<
ifeq ($(ROOT6),"yes")
	mv $(subst Dict.cc,Dict_rdict.pcm,$@) lib
endif

$(addprefix lib/,%.pcm) :
	cd lib && ln -s $< && cd -


# -- Targets
all: vars prep lib bin

lib: prep $(addprefix obj/,$(CSTUFF) $(READER)  $(DICT))
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(CSTUFF) $(READER) $(DICT)) -o $(PWD)/lib/libAnaClasses.so $(GLIBS) -lMinuit $(PWD)/lib/libClassesDict.so $(UTIL)/lib/libAnaUtil.so

bin: lib obj/runTreeReader01.o  obj/runPlot.o
	$(LD) $(LDFLAGS) -o bin/runTreeReader01 obj/runTreeReader01.o $(GLIBS) $(PWD)/lib/libAnaClasses.so $(PWD)/lib/libClassesDict.so $(UTIL)/lib/libAnaUtil.so
	$(LD) $(LDFLAGS) -o bin/runPlot obj/runPlot.o $(GLIBS) $(PWD)/lib/libAnaClasses.so  $(PWD)/lib/libClassesDict.so $(UTIL)/lib/libAnaUtil.so

# -- preparatory setup
prep:
	mkdir -p obj bin lib
	cd lib && ln -f -s $(BASE)/_build/libClassesDict.so && cd -
	cd lib && ln -f -s $(UTIL)/lib/libAnaUtil.so && cd -

# -- clean up
clean:
	rm -f $(addprefix obj/,$(READER) $(DICT) $(CSTUFF) runTreeReader01.o runPlot.o)
	rm -f $(DICTHEADERS)  $(DICTSOURCES)
	rm -f bin/*
	rm -f lib/*

cleanresults:
	rm -f results/*.root results/*.pdf results/*.tex results/*.tex.old
