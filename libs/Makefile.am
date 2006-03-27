if compile_lib_KOPAINTER
KOPAINTERDIR = kopainter
endif

if compile_kross
KROSSDIR = kross
endif

if compile_kexionly
SUBDIRS = store kwmf kofficecore kofficeui koproperty $(KROSSDIR)
else
SUBDIRS = store kwmf kofficecore kofficeui kotext \
          $(KOPAINTERDIR) kformula koproperty kopalette $(KROSSDIR)
endif

messages:
#	$(MAKE) -C kformula -f Makefile.am symbolnames.cc
	EXCLUDE="-path ./koproperty"; \
	LIST=`find . \( \( $$EXCLUDE \) -prune -o -name \*.ui -o -name \*.rc \) -type f | grep -v -e '/\.'`; \
	$(EXTRACTRC) $$LIST > rc.cpp; \
	LIST=`find . \( \( $$EXCLUDE \) -prune -o -name \*.cpp -o -name \*.cc \) -type f | grep -v -e '/\.'`; \
	$(XGETTEXT) $$LIST -o $(podir)/koffice.pot
#	$(EXTRACTRC) */*.rc */*.ui > rc.cpp
#	$(XGETTEXT) kotext/kohyphen/*.cpp */*.cc */*.cpp -o $(podir)/koffice.pot
#	rm -f kformula/symbolnames.cc

DOXYGEN_REFERENCES = dcop kprint kdeui kparts
include ../admin/Doxyfile.am
