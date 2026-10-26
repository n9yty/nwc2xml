# Makefile for nwc2xml (wxWidgets-free version)
#
# Dependencies: a C++11 compiler, zlib, and iconv.
#   - Linux: zlib development headers (e.g. zlib1g-dev); iconv is in glibc.
#   - macOS: zlib and libiconv ship with the system; no extra packages.

CXX      ?= c++
CXXFLAGS ?= -O2 -std=c++11 -Wall -Wformat
CXXFLAGS += -Wno-invalid-offsetof
LDLIBS    = -lz

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LDLIBS += -liconv
endif

OBJS = nwc2xml.o nwcfile.o nwcobj.o xmlsaver.o xmlwriter.o wxshim.o

HEADERS = Precompile.h wxshim.h TVector.h nwcendian.h nwcobj.h nwcfile.h \
          XMLSaver.h XMLWriter.h nwc2xml-version.h

all: nwc2xml

nwc2xml: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f nwc2xml $(OBJS)

.PHONY: all clean
