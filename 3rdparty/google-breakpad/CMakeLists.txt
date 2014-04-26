project(breakpad-client)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

message(STATUS ${CMAKE_SYSTEM_PROCESSOR})

INCLUDE_DIRECTORIES(
	src/
)

set(SOURCES )

if (UNIX) 
	ADD_DEFINITIONS(-Wall)
	set(SOURCES ${SOURCES}
		src/client/minidump_file_writer.cc
		src/common/convert_UTF.c
		src/common/string_conversion.cc
	)
endif(UNIX)

if (WIN32) 
	SET(SOURCES  ${SOURCES}
		src/common/windows/guid_string.cc
		src/client/windows/crash_generation/crash_generation_client.cc
		src/client/windows/crash_generation/client_info.cc
		src/client/windows/crash_generation/minidump_generator.cc
		src/client/windows/handler/exception_handler.cc
	   )
   	#ADD_DEFINITIONS(/Zc:wchar_t-) 	
	ADD_DEFINITIONS(/GL /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "_UNICODE" /D "UNICODE" /EHsc )
	#INCLUDE_DIRECTORIES(c:/progra~1/micros~1.0/vc/atlmfc)
	SET(EXTRA_LIBS ole32)
endif (WIN32)

if (UNIX)
	if (APPLE) 
        INCLUDE_DIRECTORIES(/Developer/Headers/FlatCarbon)
		FIND_LIBRARY(COCOA_LIBRARY COCOA)
		MARK_AS_ADVANCED(COCOA_LIBRARY)
		SET(EXTRA_LIBS crypto ${COCOA_LIBRARY})

		SET(SOURCES  ${SOURCES}
			src/client/mac/handler/breakpad_nlist_64.cc
			src/client/mac/handler/dynamic_images.cc
			src/client/mac/handler/exception_handler.cc
			src/client/mac/handler/minidump_generator.cc
			src/client/mac/handler/protected_memory_allocator.cc
			src/common/mac/dump_syms.mm
			src/common/mac/dwarf/bytereader.cc
			src/common/mac/dwarf/dwarf2reader.cc
			src/common/mac/dwarf/functioninfo.cc
			src/common/mac/file_id.cc
			src/common/mac/macho_id.cc
			src/common/mac/macho_utilities.cc
			src/common/mac/macho_walker.cc
			src/common/mac/string_utilities.cc
		)
	else (APPLE)
		SET(SOURCES  ${SOURCES}
			src/client/linux/handler/exception_handler.cc
			src/client/linux/handler/linux_thread.cc
			src/client/linux/handler/minidump_generator.cc
			src/common/linux/dump_symbols.cc
			src/common/linux/file_id.cc
			src/common/linux/guid_creator.cc
			src/common/linux/http_upload.cc
			src/common/md5.c
		   )
	endif (APPLE)
endif (UNIX)

add_library(breakpad ${SOURCES})

IF(UNIX AND NOT APPLE)
	SET_TARGET_PROPERTIES( breakpad PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF(UNIX AND NOT APPLE)

target_link_libraries(breakpad ${EXTRA_LIBS}) 
install(TARGETS breakpad DESTINATION lib${LIB_SUFFIX})
install(DIRECTORY
    src/client
    src/common
    src/google_breakpad
    src/processor
DESTINATION include/breakpad)
