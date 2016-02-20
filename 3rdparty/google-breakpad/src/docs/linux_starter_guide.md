# How To Add Breakpad To Your Linux Application

This document is an overview of using the Breakpad client libraries on Linux.

## Building the Breakpad libraries

Breakpad provides an Autotools build system that will build both the Linux
client libraries and the processor libraries. Running `./configure && make` in
the Breakpad source directory will produce
**src/client/linux/libbreakpad\_client.a**, which contains all the code
necessary to produce minidumps from an application.

## Integrating Breakpad into your Application

First, configure your build process to link **libbreakpad\_client.a** into your
binary, and set your include paths to include the **src** directory in the
**google-breakpad** source tree. Next, include the exception handler header: ```

# include "client/linux/handler/exception_handler.h"

```

Now you can instantiate an `ExceptionHandler` object. Exception handling is active for the lifetime of the `ExceptionHandler` object, so you should instantiate it as early as possible in your application's startup process, and keep it alive for as close to shutdown as possible. To do anything useful, the `ExceptionHandler` constructor requires a path where it can write minidumps, as well as a callback function to receive information about minidumps that were written:
```

static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
void* context, bool succeeded) { printf("Dump path: %s\n", descriptor.path());
return succeeded; }

void crash() { volatile int* a = (int*)(NULL); *a = 1; }

int main(int argc, char* argv[]) { google_breakpad::MinidumpDescriptor
descriptor("/tmp"); google_breakpad::ExceptionHandler eh(descriptor, NULL,
dumpCallback, NULL, true, -1); crash(); return 0; } ```

Compiling and running this example should produce a minidump file in /tmp, and
it should print the minidump filename before exiting. You can read more about
the other parameters to the `ExceptionHandler` constructor <a
href='http://code.google.com/p/google-breakpad/source/browse/trunk/src/client/linux/handler/exception_handler.h'>in
the exception_handler.h source file</a>.

**Note**: You should do as little work as possible in the callback function.
Your application is in an unsafe state. It may not be safe to allocate memory or
call functions from other shared libraries. The safest thing to do is `fork` and
`exec` a new process to do any work you need to do. If you must do some work in
the callback, the Breakpad source contains <a
href='http://code.google.com/p/google-breakpad/source/browse/trunk/src/common/linux/linux_libc_support.h'>some
simple reimplementations of libc functions</a>, to avoid calling directly into
libc, as well as <a href='http://code.google.com/p/linux-syscall-support/'>a
header file for making Linux system calls</a> (in **src/third\_party/lss**) to
avoid calling into other shared libraries.

## Sending the minidump file

In a real application, you would want to handle the minidump in some way, likely
by sending it to a server for analysis. The Breakpad source tree contains <a
href='http://code.google.com/p/google-breakpad/source/browse/#svn/trunk/src/common/linux'>some
HTTP upload source</a> that you might find useful, as well as <a
href='http://code.google.com/p/google-breakpad/source/browse/#svn/trunk/src/tools/linux/symupload'>a
minidump upload tool</a>.

## Producing symbols for your application

To produce useful stack traces, Breakpad requires you to convert the debugging
symbols in your binaries to <a
href='http://code.google.com/p/google-breakpad/wiki/SymbolFiles'>text-format
symbol files</a>. First, ensure that you've compiled your binaries with `-g` to
include debugging symbols. Next, compile the `dump_syms` tool by running
`configure && make` in the Breakpad source directory. Next, run `dump_syms` on
your binaries to produce the text-format symbols. For example, if your main
binary was named `test`: `$ google-breakpad/src/tools/linux/dump_syms/dump_syms
./test > test.sym
`

In order to use these symbols with the `minidump_stackwalk` tool, you will need
to place them in a specific directory structure. The first line of the symbol
file contains the information you need to produce this directory structure, for
example (your output will vary): `$ head -n1 test.sym MODULE Linux x86_64
6EDC6ACDB282125843FD59DA9C81BD830 test $ mkdir -p
./symbols/test/6EDC6ACDB282125843FD59DA9C81BD830 $ mv test.sym
./symbols/test/6EDC6ACDB282125843FD59DA9C81BD830
`

You may also find the <a
href='http://mxr.mozilla.org/mozilla-central/source/toolkit/crashreporter/tools/symbolstore.py'>symbolstore.py</a>
script in the Mozilla repository useful, as it encapsulates these steps.

## Processing the minidump to produce a stack trace

Breakpad includes a tool called `minidump_stackwalk` which can take a minidump
plus its corresponding text-format symbols and produce a symbolized stacktrace.
It should be in the **google-breakpad/src/processor** directory if you compiled
the Breakpad source using the directions above. Simply pass it the minidump and
the symbol path as commandline parameters:
`google-breakpad/src/processor/minidump_stackwalk minidump.dmp ./symbols
` It produces verbose output on stderr, and the stacktrace on stdout, so you may
want to redirect stderr.
