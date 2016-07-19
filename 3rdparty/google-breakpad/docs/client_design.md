# Breakpad Client Libraries

## Objective

The Breakpad client libraries are responsible for monitoring an application for
crashes (exceptions), handling them when they occur by generating a dump, and
providing a means to upload dumps to a crash reporting server. These tasks are
divided between the “handler” (short for “exception handler”) library linked in
to an application being monitored for crashes, and the “sender” library,
intended to be linked in to a separate external program.

## Background

As one of the chief tasks of the client handler is to generate a dump, an
understanding of [dump files](processor_design.md) will aid in understanding the
handler.

## Overview

Breakpad provides client libraries for each of its target platforms. Currently,
these exist for Windows on x86 and Mac OS X on both x86 and PowerPC. A Linux
implementation has been written and is currently under review.

Because the mechanisms for catching exceptions and the methods for obtaining the
information that a dump contains vary between operating systems, each target
operating system requires a completely different handler implementation. Where
multiple CPUs are supported for a single operating system, the handler
implementation will likely also require separate code for each processor type to
extract CPU-specific information. One of the goals of the Breakpad handler is to
provide a prepackaged cross-platform system that masks many of these
system-level differences and quirks from the application developer. Although the
underlying implementations differ, the handler library for each system follows
the same set of principles and exposes a similar interface.

Code that wishes to take advantage of Breakpad should be linked against the
handler library, and should, at an appropriate time, install a Breakpad handler.
For applications, it is generally desirable to install the handler as early in
the start-up process as possible. Developers of library code using Breakpad to
monitor itself may wish to install a Breakpad handler when the library is
loaded, or may only want to install a handler when calls are made in to the
library.

The handler can be triggered to generate a dump either by catching an exception
or at the request of the application itself. The latter case may be useful in
debugging assertions or other conditions where developers want to know how a
program got in to a specific non-crash state. After generating a dump, the
handler calls a user-specified callback function. The callback function may
collect additional data about the program’s state, quit the program, launch a
crash reporter application, or perform other tasks. Allowing for this
functionality to be dictated by a callback function preserves flexibility.

The sender library is also has a separate implementation for each supported
platform, because of the varying interfaces for accessing network resources on
different operating systems. The sender transmits a dump along with other
application-defined information to a crash report server via HTTP. Because dumps
may contain sensitive data, the sender allows for the use of HTTPS.

The canonical example of the entire client system would be for a monitored
application to link against the handler library, install a Breakpad handler from
its main function, and provide a callback to launch a small crash reporter
program. The crash reporter program would be linked against the sender library,
and would send the crash dump when launched. A separate process is recommended
for this function because of the unreliability inherent in doing any significant
amount of work from a crashed process.

## Detailed Design

### Exception Handler Installation

The mechanisms for installing an exception handler vary between operating
systems. On Windows, it’s a relatively simple matter of making one call to
register a [top-level exception filter]
(http://msdn.microsoft.com/library/en-us/debug/base/setunhandledexceptionfilter.asp)
callback function. On most Unix-like systems such as Linux, processes are
informed of exceptions by the delivery of a signal, so an exception handler
takes the form of a signal handler. The native mechanism to catch exceptions on
Mac OS X requires a large amount of code to set up a Mach port, identify it as
the exception port, and assign a thread to listen for an exception on that port.
Just as the preparation of exception handlers differ, the manner in which they
are called differs as well. On Windows and most Unix-like systems, the handler
is called on the thread that caused the exception. On Mac OS X, the thread
listening to the exception port is notified that an exception has occurred. The
different implementations of the Breakpad handler libraries perform these tasks
in the appropriate ways on each platform, while exposing a similar interface on
each.

A Breakpad handler is embodied in an `ExceptionHandler` object. Because it’s a
C++ object, `ExceptionHandler`s may be created as local variables, allowing them
to be installed and removed as functions are called and return. This provides
one possible way for a developer to monitor only a portion of an application for
crashes.

### Exception Basics

Once an application encounters an exception, it is in an indeterminate and
possibly hazardous state. Consequently, any code that runs after an exception
occurs must take extreme care to avoid performing operations that might fail,
hang, or cause additional exceptions. This task is not at all straightforward,
and the Breakpad handler library seeks to do it properly, accounting for all of
the minute details while allowing other application developers, even those with
little systems programming experience, to reap the benefits. All of the Breakpad
handler code that executes after an exception occurs has been written according
to the following guidelines for safety at exception time:

*   Use of the application heap is forbidden. The heap may be corrupt or
    otherwise unusable, and allocators may not function.
*   Resource allocation must be severely limited. The handler may create a new
    file to contain the dump, and it may attempt to launch a process to continue
    handling the crash.
*   Execution on the thread that caused the exception is significantly limited.
    The only code permitted to execute on this thread is the code necessary to
    transition handling to a dedicated preallocated handler thread, and the code
    to return from the exception handler.
*   Handlers shouldn’t handle crashes by attempting to walk stacks themselves,
    as stacks may be in inconsistent states. Dump generation should be performed
    by interfacing with the operating system’s memory manager and code module
    manager.
*   Library code, including runtime library code, must be avoided unless it
    provably meets the above guidelines. For example, this means that the STL
    string class may not be used, because it performs operations that attempt to
    allocate and use heap memory. It also means that many C runtime functions
    must be avoided, particularly on Windows, because of heap operations that
    they may perform.

A dedicated handler thread is used to preserve the state of the exception thread
when an exception occurs: during dump generation, it is difficult if not
impossible for a thread to accurately capture its own state. Performing all
exception-handling functions on a separate thread is also critical when handling
stack-limit-exceeded exceptions. It would be hazardous to run out of stack space
while attempting to handle an exception. Because of the rule against allocating
resources at exception time, the Breakpad handler library creates its handler
thread when it installs its exception handler. On Mac OS X, this handler thread
is created during the normal setup of the exception handler, and the handler
thread will be signaled directly in the event of an exception. On Windows and
Linux, the handler thread is signaled by a small amount of code that executes on
the exception thread. Because the code that executes on the exception thread in
this case is small and safe, this does not pose a problem. Even when an
exception is caused by exceeding stack size limits, this code is sufficiently
compact to execute entirely within the stack’s guard page without causing an
exception.

The handler thread may also be triggered directly by a user call, even when no
exception occurs, to allow dumps to be generated at any point deemed
interesting.

### Filter Callback

When the handler thread begins handling an exception, it calls an optional
user-defined filter callback function, which is responsible for judging whether
Breakpad’s handler should continue handling the exception or not. This mechanism
is provided for the benefit of library or plug-in code, whose developers may not
be interested in reports of crashes that occur outside of their modules but
within processes hosting their code. If the filter callback indicates that it is
not interested in the exception, the Breakpad handler arranges for it to be
delivered to any previously-installed handler.

### Dump Generation

Assuming that the filter callback approves (or does not exist), the handler
writes a dump in a directory specified by the application developer when the
handler was installed, using a previously generated unique identifier to avoid
name collisions. The mechanics of dump generation also vary between platforms,
but in general, the process involves enumerating each thread of execution, and
capturing its state, including processor context and the active portion of its
stack area. The dump also includes a list of the code modules loaded in to the
application, and an indicator of which thread generated the exception or
requested the dump. In order to avoid allocating memory during this process, the
dump is written in place on disk.

### Post-Dump Behavior

Upon completion of writing the dump, a second callback function is called. This
callback may be used to launch a separate crash reporting program or to collect
additional data from the application. The callback may also be used to influence
whether Breakpad will treat the exception as handled or unhandled. Even after a
dump is successfully generated, Breakpad can be made to behave as though it
didn’t actually handle an exception. This function may be useful for developers
who want to test their applications with Breakpad enabled but still retain the
ability to use traditional debugging techniques. It also allows a
Breakpad-enabled application to coexist with a platform’s native crash reporting
system, such as Mac OS X’ [CrashReporter]
(http://developer.apple.com/technotes/tn2004/tn2123.html) and [Windows Error
Reporting](http://msdn.microsoft.com/isv/resources/wer/).

Typically, when Breakpad handles an exception fully and no debuggers are
involved, the crashed process will terminate.

Authors of both callback functions that execute within a Breakpad handler are
cautioned that their code will be run at exception time, and that as a result,
they should observe the same programming practices that the Breakpad handler
itself adheres to. Notably, if a callback is to be used to collect additional
data from an application, it should take care to read only “safe” data. This
might involve accessing only static memory locations that are updated
periodically during the course of normal program execution.

### Sender Library

The Breakpad sender library provides a single function to send a crash report to
a crash server. It accepts a crash server’s URL, a map of key-value parameters
that will accompany the dump, and the path to a dump file itself. Each of the
key-value parameters and the dump file are sent as distinct parts of a multipart
HTTP POST request to the specified URL using the platform’s native HTTP
facilities. On Linux, [libcurl](http://curl.haxx.se/) is used for this function,
as it is the closest thing to a standard HTTP library available on that
platform.

## Future Plans

Although we’ve had great success with in-process dump generation by following
our guidelines for safe code at exception time, we are exploring options for
allowing dumps to be generated in a separate process, to further enhance the
handler library’s robustness.

On Windows, we intend to offer tools to make it easier for Breakpad’s settings
to be managed by the native group policy management system.

We also plan to offer tools that many developers would find desirable in the
context of handling crashes, such as a mechanism to determine at launch if the
program last terminated in a crash, and a way to calculate “crashiness” in terms
of crashes over time or the number of application launches between crashes.

We are also investigating methods to capture crashes that occur early in an
application’s launch sequence, including crashes that occur before a program’s
main function begins executing.
