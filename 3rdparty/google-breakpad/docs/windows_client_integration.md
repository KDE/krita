# Windows Integration overview

## Windows Client Code

The Windows client code is in the `src/client/windows` directory of the tree.
Since the header files are fairly well commented some specifics are purposely
omitted from this document.

## Integration of minidump-generation

Once you build the solution inside `src/client/windows`, an output file of
`exception_handler.lib` will be generated. You can either check this into your
project's directory or build directly from the source, as the project itself
does.

Enabling Breakpad in your application requires you to `#include
"exception_handler.h"` and instantiate the `ExceptionHandler` object like so:

```
  handler = new ExceptionHandler(const wstring& dump_path,
                                                              FilterCallback filter,
                                                              MinidumpCallback callback,
                                                              void* callback_context,
                                                              int handler_types,
                                                              MINIDUMP_TYPE dump_type,
                                                              const wchar_t* pipe_name,
                                                              const CustomClientInfo* custom_info);
```

The parameters, in order, are:

*   pathname for minidumps to be written to - this is ignored if OOP dump
    generation is used
*   A callback that is called when the exception is first handled - you can
    return true/false here to continue/stop exception processing
*   A callback that is called after minidumps have been written
*   Context for the callbacks
*   Which exceptions to handle - see `HandlerType` enumeration in
    exception\_handler.h
*   The type of minidump to generate, using the `MINIDUMP_TYPE` definitions in
    `DbgHelp.h`
*   A pipe name that can be used to communicate with a crash generation server
*   A pointer to a CustomClientInfo class that can be used to send custom data
    along with the minidump when using OOP generation

You can also see `src/client/windows/tests/crash_generation_app/*` for a sample
app that uses OOP generation.

## OOP Minidump Generation

For out of process minidump generation, more work is needed. If you look inside
`src/client/windows/crash_generation`, you will see a file called
`crash_generation_server.h`. This file is the interface for a crash generation
server, which must be instantiated with the same pipe name that is passed to the
client above. The logistics of running a separate process that instantiates the
crash generation server is left up to you, however.

## Build process specifics(symbol generation, upload)

The symbol creation step is talked about in the general overview doc, since it
doesn't vary much by platform. You'll need to make sure that the symbols are
available wherever minidumps are uploaded to for processing.

## Out in the field - uploading the minidump

Inside `src/client/windows/sender` is a class implementation called
`CrashReportSender`. This class can be compiled into a separate standalone CLI
or in the crash generation server and used to upload the report; it can know
when to do so via one of the callbacks provided by the `CrashGenerationServer`
or the `ExceptionHandler` object for in-process generation.
