The goal of this document is to give an overview of the exception handling
options in breakpad.

# Basics

Exception handling is a mechanism designed to handle the occurrence of
exceptions, special conditions that change the normal flow of program execution.

`SetUnhandledExceptionFilter` replaces all unhandled exceptions when Breakpad is
enabled. TODO: More on first and second change and vectored v. try/catch.

There are two main types of exceptions across all platforms: in-process and
out-of-process.

# In-Process

In process exception handling is relatively simple since the crashing process
handles crash reporting. It is generally considered unsafe to write a minidump
from a crashed process. For example, key data structures could be corrupted or
the stack on which the exception handler runs could have been overwritten. For
this reason all platforms also support some level of out-of-process exception
handling.

## Windows

In-process exception handling Breakpad creates a 'handler head' that waits
infinitely on a semaphore at start up. When this thread is woken it writes the
minidump and signals to the excepting thread that it may continue. A filter will
tell the OS to kill the process if the minidump is written successfully.
Otherwise it continues.

# Out-of-Process

Out-of-process exception handling is more complicated than in-process exception
handling because of the need to set up a separate process that can read the
state of the crashing process.

## Windows

Breakpad uses two abstractions around the exception handler to make things work:
`CrashGenerationServer` and `CrashGenerationClient`. The constructor for these
takes a named pipe name.

During server start up a named pipe and registers callbacks for client
connections are created. The named pipe is used for registration and all IO on
the pipe is done asynchronously. `OnPipeConnected` is called when a client
attempts to connect (call `CreateFile` on the pipe). `OnPipeConnected` does the
state machine transition from `Initial` to `Connecting` and on through
`Reading`, `Reading_Done`, `Writing`, `Writing_Done`, `Reading_ACK`, and
`Disconnecting`.

When registering callbacks, the client passes in two pointers to pointers: 1. A
pointer to the `EXCEPTION_INFO` pointer 1. A pointer to the `MDRawAssertionInfo`
which handles various non-exception failures like assertions

The essence of registration is adding a "`ClientInfo`" object that contains
handles used for synchronization with the crashing process to an array
maintained by the server. This is how we can keep track of all the clients on
the system that have registered for minidumps. These handles are: *
`server_died(mutex)` * `dump_requested(Event)` * `dump_generated(Event)`

The server registers asynchronous waits on these events with the `ClientInfo`
object as the callback context. When the `dump_requested` event is set by the
client, the `OnDumpRequested()` callback is called. The server uses the handles
inside `ClientInfo` to communicate with the child process. Once the child sets
the event, it waits for two objects: 1. the `dump_generated` event 1. the
`server_died` mutex

In the end handles are "duped" into the client process, and the clients use
`SetEvent` to request events, wait on the other event, or the `server_died`
mutex.

## Linux

### Current Status

As of July 2011, Linux had a minidump generator that is not entirely
out-of-process. The minidump was generated from a separate process, but one that
shared an address space, file descriptors, signal handles and much else with the
crashing process. It worked by using the `clone()` system call to duplicate the
crashing process, and then uses `ptrace()` and the `/proc` file system to
retrieve the information required to write the minidump. Since then Breakpad has
updated Linux exception handling to provide more benefits of out-of-process
report generation.

### Proposed Design

#### Overview

Breakpad would use a per-user daemon to write out a minidump that does not have,
interact with or depend on the crashing process. We don't want to start a new
separate process every time a user launches a Breakpad-enabled process. Doing
one daemon per machine is unacceptable for security concerns around one user
being able to initiate a minidump generation for another user's process.

#### Client/Server Communication

On Breakpad initialization in a process, the initializer would check if the
daemon is running and, if not, start it. The race condition between the check
and the initialization is not a problem because multiple daemons can check if
the IPC endpoint already exists and if a server is listening. Even if multiple
copies of the daemon try to `bind()` the filesystem to name the socket, all but
one will fail and can terminate.

This point is relevant for error handling conditions. Linux does not clean the
file system representation of a UNIX domain socket even if both endpoints
terminate, so checking for existence is not strong enough. However checking the
process list or sending a ping on the socket can handle this.

Breakpad uses UNIX domain sockets since they support full duplex communication
(unlike Windows, named pipes on Linux are half) and the kernal automatically
creates a private channel between the client and server once the client calls
`connect()`.

#### Minidump Generation

Breakpad could use the current system with `ptrace()` and `/proc` within the
daemon executable.

Overall the operations look like: 1. Signal from OS indicating crash 1. Signal
Handler suspends all threads except itself 1. Signal Handler sends
`CRASH_DUMP_REQUEST` message to server and waits for response 1. Server inspects
1. Minidump is asynchronously written to disk by the server 1. Server responds
indicating inspection is done

## Mac OSX

Out-of-process exception handling is fully supported on Mac.
