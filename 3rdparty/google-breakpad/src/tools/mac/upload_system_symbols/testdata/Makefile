all: libarchtest.dylib archtest.exe

archtest32.exe: archtest.c
	clang -m32 $< -o $@

archtest64.exe: archtest.c
	clang -m64 $< -o $@

archtest.exe: archtest32.exe archtest64.exe
	lipo $^ -create -output $@

libarchtest32.dylib: archtest.c
	clang -m32 -dynamiclib $< -o $@

libarchtest64.dylib: archtest.c
	clang -m64 -dynamiclib $< -o $@

libarchtest.dylib: libarchtest32.dylib libarchtest64.dylib
	lipo $^ -create -output $@

clean:
	rm -f *.dylib *.exe
