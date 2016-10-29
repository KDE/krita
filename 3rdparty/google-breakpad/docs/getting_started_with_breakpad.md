# Introduction

Breakpad is a library and tool suite that allows you to distribute an
application to users with compiler-provided debugging information removed,
record crashes in compact "minidump" files, send them back to your server, and
produce C and C++ stack traces from these minidumps. Breakpad can also write
minidumps on request for programs that have not crashed.

Breakpad is currently used by Google Chrome, Firefox, Google Picasa, Camino,
Google Earth, and other projects.

![http://google-breakpad.googlecode.com/svn/wiki/breakpad.png]
(http://google-breakpad.googlecode.com/svn/wiki/breakpad.png)

Breakpad has three main components:

*   The **client** is a library that you include in your application. It can
    write minidump files capturing the current threads' state and the identities
    of the currently loaded executable and shared libraries. You can configure
    the client to write a minidump when a crash occurs, or when explicitly
    requested.

*   The **symbol dumper** is a program that reads the debugging information
    produced by the compiler and produces a **symbol file**, in [Breakpad's own
    format](symbol_files.md).

*   The **processor** is a program that reads a minidump file, finds the
    appropriate symbol files for the versions of the executables and shared
    libraries the minidump mentions, and produces a human-readable C/C++ stack
    trace.

# The minidump file format

The minidump file format is similar to core files but was developed by Microsoft
for its crash-uploading facility. A minidump file contains:

*   A list of the executable and shared libraries that were loaded in the
    process at the time the dump was created. This list includes both file names
    and identifiers for the particular versions of those files that were loaded.

*   A list of threads present in the process. For each thread, the minidump
    includes the state of the processor registers, and the contents of the
    threads' stack memory. These data are uninterpreted byte streams, as the
    Breakpad client generally has no debugging information available to produce
    function names or line numbers, or even identify stack frame boundaries.

*   Other information about the system on which the dump was collected:
    processor and operating system versions, the reason for the dump, and so on.

Breakpad uses Windows minidump files on all platforms, instead of the
traditional core files, for several reasons:

*   Core files can be very large, making them impractical to send across a
    network to the collector for processing. Minidumps are smaller, as they were
    designed to be used this way.

*   The core file format is poorly documented. For example, the Linux Standards
    Base does not describe how registers are stored in `PT_NOTE` segments.

*   It is harder to persuade a Windows machine to produce a core dump file than
    it is to persuade other machines to write a minidump file.

*   It simplifies the Breakpad processor to support only one file format.

# Overview/Life of a minidump

A minidump is generated via calls into the Breakpad library. By default,
initializing Breakpad installs an exception/signal handler that writes a
minidump to disk at exception time. On Windows, this is done via
`SetUnhandledExceptionFilter()`; on OS X, this is done by creating a thread that
waits on the Mach exception port; and on Linux, this is done by installing a
signal handler for various exceptions like `SIGILL, SIGSEGV` etc.

Once the minidump is generated, each platform has a slightly different way of
uploading the crash dump. On Windows & Linux, a separate library of functions is
provided that can be called into to do the upload. On OS X, a separate process
is spawned that prompts the user for permission, if configured to do so, and
sends the file.

# Terminology

**In-process vs. out-of-process exception handling** - it's generally considered
that writing the minidump from within the crashed process is unsafe - key
process data structures could be corrupted, or the stack on which the exception
handler runs could have been overwritten, etc. All 3 platforms support what's
known as "out-of-process" exception handling.

# Integration overview

## Breakpad Code Overview

All the client-side code is found by visiting the Google Project at
http://code.google.com/p/google-breakpad. The following directory structure is
present in the `src` directory:

*   `processor` Contains minidump-processing code that is used on the server
    side and isn't of use on the client side
*   `client` Contains client minidump-generation libraries for all platforms
*   `tools` Contains source code & projects for building various tools on each
    platform.

(Among other directories)

*   <a
    href='http://code.google.com/p/google-breakpad/wiki/WindowsClientIntegration'>Windows
    Integration Guide</a>
*   <a
    href='http://code.google.com/p/google-breakpad/wiki/MacBreakpadStarterGuide'>Mac
    Integration Guide</a>
*   <a href='http://code.google.com/p/google-breakpad/wiki/LinuxStarterGuide'>
    Linux Integration Guide</a>

## Build process specifics(symbol generation)

This applies to all platforms. Inside `src/tools/{platform}/dump_syms` is a tool
that can read debugging information for each platform (e.g. for OS X/Linux,
DWARF and STABS, and for Windows, PDB files) and generate a Breakpad symbol
file. This tool should be run on your binary before it's stripped(in the case of
OS X/Linux) and the symbol files need to be stored somewhere that the minidump
processor can find. There is another tool, `symupload`, that can be used to
upload symbol files if you have written a server that can accept them.
