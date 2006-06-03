
macro_optional_find_package(KROSS)

add_subdirectory( store )
add_subdirectory( kwmf )
add_subdirectory( kofficecore )
add_subdirectory( kofficeui )
add_subdirectory( koproperty )
add_subdirectory( kotext )
add_subdirectory( kformula )
add_subdirectory( kopalette )
add_subdirectory( kopainter )
add_subdirectory( flake )
add_subdirectory( pigment )
# Laurent: need to detect ruby/python
MESSAGE(STATUS "KOFFICE_ENABLE_SCRIPTING :<${KOFFICE_ENABLE_SCRIPTING}>")
if(KOFFICE_ENABLE_SCRIPTING)
	add_subdirectory(kross)
endif(KOFFICE_ENABLE_SCRIPTING)

########### install files ###############


#kde4_create_doxygen_docs( REFERENCES dcop kprint kdeui kparts  )





#original Makefile.am contents follow:

#if compile_lib_KOPAINTER
#KOPAINTERDIR = kopainter
#endif
#
#if compile_kross
#KROSSDIR = kross
#endif
#
#if compile_kexionly
#SUBDIRS = store kwmf kofficecore kofficeui koproperty $(KROSSDIR)
#else
#SUBDIRS = store kwmf kofficecore kofficeui kotext \
#          $(KOPAINTERDIR) kformula koproperty kopalette $(KROSSDIR)
#endif
#
#messages:
##	$(MAKE) -C kformula -f Makefile.am symbolnames.cc
#	EXCLUDE="-path ./koproperty"; \
#	LIST=`find . \( \( $$EXCLUDE \) -prune -o -name \*.ui -o -name \*.rc \) -type f | grep -v -e '/\.'`; \
#	$(EXTRACTRC) $$LIST > rc.cpp; \
#	LIST=`find . \( \( $$EXCLUDE \) -prune -o -name \*.cpp -o -name \*.cc \) -type f | grep -v -e '/\.'`; \
#	$(XGETTEXT) $$LIST -o $(podir)/koffice.pot
##	$(EXTRACTRC) */*.rc */*.ui > rc.cpp
##	$(XGETTEXT) kotext/kohyphen/*.cpp */*.cc */*.cpp -o $(podir)/koffice.pot
##	rm -f kformula/symbolnames.cc
#
#DOXYGEN_REFERENCES = dcop kprint kdeui kparts
#include ../admin/Doxyfile.am
