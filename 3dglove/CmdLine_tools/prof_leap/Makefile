OS := $(shell uname)
ARCH := $(shell uname -m)

ifeq ($(OS), Linux)
  ifeq ($(ARCH), x86_64)
    LEAP_LIBRARY := ../../lib/x64/libLeap.so -Wl,-rpath,../lib/x64
  else
    LEAP_LIBRARY := ../../lib/x86/libLeap.so -Wl,-rpath,../lib/x86
  endif
else
  # OS X
  LEAP_LIBRARY := ../../libLeap.dylib
endif

Sample: Sample.cpp
	$(CXX) -Wall -g -I../../leap Sample.cpp -o Sample $(LEAP_LIBRARY)
ifeq ($(OS), Darwin)
	install_name_tool -change @loader_path/libLeap.dylib ../../libLeap.dylib Sample
endif

clean:
	rm -rf Sample Sample.dSYM
