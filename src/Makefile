#!/usr/bin/make -f
# Only to clean artifacts

# we don't want to do anything by default
all: 

clean:
	rm -rf *.d *.o
	$(MAKE) clean -C raylib/src
# special library name for web
	rm -f raylib/src/libraylib.web.a
