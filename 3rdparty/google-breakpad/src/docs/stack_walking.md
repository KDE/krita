# Introduction

This page aims to provide a detailed description of how Breakpad produces stack
traces from the information contained within a minidump file.

# Details

## Starting the Process

Typically the stack walking process is initiated by instantiating the
[MinidumpProcessor]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/processor/minidump_processor.cc)
class and calling the [MinidumpProcessor::Process]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/processor/minidump_processor.cc#61)
method, providing it a minidump file to process. To produce a useful stack
trace, the MinidumpProcessor requires two other objects which are passed in its
constructor: a [SymbolSupplier]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/symbol_supplier.h)
and a [SourceLineResolverInterface]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/source_line_resolver_interface.h).
The SymbolSupplier object is responsible for locating and providing SymbolFiles
that match modules from the minidump. The SourceLineResolverInterface is
responsible for loading the symbol files and using the information contained
within to provide function and source information for stack frames, as well as
information on how to unwind from a stack frame to its caller. More detail will
be provided on these interactions later.

A number of data streams are extracted from the minidump to begin stack walking:
the list of threads from the process ([MinidumpThreadList]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/minidump.h#335)),
the list of modules loaded in the process ([MinidumpModuleList]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/minidump.h#501)),
and information about the exception that caused the process to crash
([MinidumpException]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/minidump.h#615)).

## Enumerating Threads

For each thread in the thread list ([MinidumpThread]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/minidump.h#299)),
the thread memory containing the stack for the thread ([MinidumpMemoryRegion]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/minidump.h#236))
and the CPU context representing the CPU state of the thread at the time the
dump was written ([MinidumpContext]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/minidump.h#171))
are extracted from the minidump. If the thread being processed is the thread
that produced the exception then a CPU context is obtained from the
MinidumpException object instead, which represents the CPU state of the thread
at the point of the exception. A stack walker is then instantiated by calling
the [Stackwalker::StackwalkerForCPU]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/stackwalker.h#77)
method and passing it the CPU context, the thread memory, the module list, as
well as the SymbolSupplier and SourceLineResolverInterface. This method selects
the specific !Stackwalker subclass based on the CPU architecture of the provided
CPU context and returns an instance of that subclass.

## Walking a thread's stack

Once a !Stackwalker instance has been obtained, the processor calls the
[Stackwalker::Walk]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/source_line_resolver_interface.h)
method to obtain a list of frames representing the stack of this thread. The
!Stackwalker starts by calling the GetContextFrame method which returns a
StackFrame representing the top of the stack, with CPU state provided by the
initial CPU context. From there, the stack walker repeats the following steps
for each frame in turn:

### Finding the Module

The address of the instruction pointer of the current frame is used to determine
which module contains the current frame by calling the module list's
[GetModuleForAddress]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/code_modules.h#56)
method.

### Locating Symbols

If a module is located, the SymbolSupplier is asked to locate symbols
corresponding to the module by calling its [GetCStringSymbolData]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/symbol_supplier.h#87)
method. Typically this is implemented by using the module's debug filename (the
PDB filename for Windows dumps) and debug identifier (a GUID plus one extra
digit) as a lookup key. The [SimpleSymbolSupplier]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/processor/simple_symbol_supplier.cc)
class simply uses these as parts of a file path to locate a flat file on disk.

### Loading Symbols

If a symbol file is located, the SourceLineResolverInterface is then asked to
load the symbol file by calling its [LoadModuleUsingMemoryBuffer]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/source_line_resolver_interface.h#71)
method. The [BasicSourceLineResolver]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/processor/basic_source_line_resolver.cc)
implementation parses the text-format [symbol file](symbol_files.md) into
in-memory data structures to make lookups by address of function names, source
line information, and unwind information easy.

### Getting source line information

If a symbol file has been successfully loaded, the SourceLineResolverInterface's
[FillSourceLineInfo]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/source_line_resolver_interface.h#89)
method is called to provide a function name and source line information for the
current frame. This is done by subtracting the base address of the module
containing the current frame from the instruction pointer of the current frame
to obtain a relative virtual address (RVA), which is a code offset relative to
the start of the module. This RVA is then used as a lookup into a table of
functions ([FUNC lines](SymbolFiles#FUNC_records.md) from the symbol file), each
of which has an associated address range (function start address, function
size). If a function is found whose address range contains the RVA, then its
name is used. The RVA is then used as a lookup into a table of source lines
([line records](SymbolFiles#Line_records.md) from the symbol file), each of
which also has an associated address range. If a match is found it will provide
the file name and source line associated with the current frame. If no match was
found in the function table, another table of publicly exported symbols may be
consulted ([PUBLIC lines](SymbolFiles#PUBLIC_records.md) from the symbol file).
Public symbols contain only a start address, so the lookup simply looks for the
nearest symbol that is less than the provided RVA.

### Finding the caller frame

To find the next frame in the stack, the !Stackwalker calls its [GetCallerFrame]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/stackwalker.h#186)
method, passing in the current frame. Each !Stackwalker subclass implements
GetCallerFrame differently, but there are common patterns.

Typically the first step is to query the SourceLineResolverInterface for the
presence of detailed unwind information. This is done using its
[FindWindowsFrameInfo]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/source_line_resolver_interface.h#96)
and [FindCFIFrameInfo]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/source_line_resolver_interface.h#102)
methods. These methods look for Windows unwind info extracted from a PDB file
([STACK WIN](SymbolFiles#STACK_WIN_records.md) lines from the symbol file), or
DWARF CFI extracted from a binary ([STACK CFI](SymbolFiles#STACK_CFI_records.md)
lines from the symbol file) respectively. The information covers address ranges,
so the RVA of the current frame is used for lookup as with function and source
line information.

If unwind info is found it provides a set of rules to recover the register state
of the caller frame given the current register state as well as the thread's
stack memory. The rules are evaluated to produce the caller frame.

If unwind info is not found then the !Stackwalker may resort to other methods.
Typically on architectures which specify a frame pointer unwinding by
dereferencing the frame pointer is tried next. If that is successful it is used
to produce the caller frame.

If no caller frame was found by any other method most !Stackwalker
implementations resort to stack scanning by looking at each word on the stack
down to a fixed depth (implemented in the [Stackwalker::ScanForReturnAddress]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/stackwalker.h#131)
method) and using a heuristic to attempt to find a reasonable return address
(implemented in the [Stackwalker::InstructionAddressSeemsValid]
(http://code.google.com/p/google-breakpad/source/browse/trunk/src/google_breakpad/processor/stackwalker.h#111)
method).

If no caller frame is found or the caller frame seems invalid, stack walking
stops. If a caller frame was found then these steps repeat using the new frame
as the current frame.
