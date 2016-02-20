# Breakpad Crash Reporting for Mozilla

*   January 24, 2007
    *   Links updated February 14, 2007
*   Mozilla HQ
*   Mark Mentovai
*   Brian Ryner

## What is a crash reporter?

*   Enables developers to analyze crashes that occur in the wild
*   Produces stack backtraces that help identify how a program failed
*   Offers higher-level data aggregation (topcrashes, MTBF statistics)

## Motivation

*   Talkback is proprietary and unmaintained
*   Smaller open-source projects have few options
*   Larger projects need flexibility and scalability

## Design Options

*   Stackwalking done on client
    *   Apple CrashReporter
    *   GNOME BugBuddy
*   Client sends memory dump
    *   Talkback
    *   Windows Error Reporting
    *   Breakpad

## Goals

*   Provide libraries around which systems can be based
*   Open-source
*   Cross-platform
    *   Mac OS X x86, PowerPC
    *   Linux x86
    *   Windows x86
*   No requirement to distribute symbols

## Client Libraries

*   Exception handler installed at application startup
    *   Spawns a separate thread
*   Minidump file written at crash time
    *   Format used by Windows debuggers
*   Separate application invoked to send
    *   HTTP[S](S.md) POST, can include additional parameters

## Symbols

*   Cross-platform symbol file format
*   Contents
    *   Function names
    *   Source file names and line numbers
    *   Windows: Frame pointer omission data
    *   Future: parameters and local variables
*   Symbol conversion methods

## Processor

*   Examines minidump file and invokes stackwalker
*   Symbol files requested from a SymbolSupplier
*   Produces stack trace
*   Output may be placed where convenient

## Intergation

*   Breakpad client present in Gran Paradiso Alpha 1 for Windows
    *   Disabled by default
    *   Enable with `MOZ_AIRBAG`
*   Proof-of-concept collector
    *   http://mavra.perilith.com/~luser/airbag-collector/list.pl
*   Other platforms coming soon

## More Information

*   Project home: http://code.google.com/p/google-breakpad/
*   Mailing lists
    *   [google-breakpad-dev@googlegroups.com]
        (http://groups.google.com/group/google-breakpad-dev/)
    *   [google-breakpad-discuss@googlegroups.com]
        (http://groups.google.com/group/google-breakpad-discuss/)
*   Ask me (irc.mozilla.org: mento)
