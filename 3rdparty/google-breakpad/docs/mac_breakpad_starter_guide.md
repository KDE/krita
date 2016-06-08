# How To Add Breakpad To Your Mac Client Application

This document is a step-by-step recipe to get your Mac client app to build with
Breakpad.

## Preparing a binary build of Breakpad for use in your tree

You can either check in a binary build of the Breakpad framework & tools or
build it as a dependency of your project. The former is recommended, and
detailed here, since building dependencies through other projects is
problematic(matching up configuration names), and the Breakpad code doesn't
change nearly often enough as your application's will.

## Building the requisite targets

All directories are relative to the `src` directory of the Breakpad checkout.

*   Build the 'All' target of `client/mac/Breakpad.xcodeproj` in Release mode.
*   Execute `cp -R client/mac/build/Release/Breakpad.framework <location in your
    source tree>`
*   Inside `tools/mac/dump_syms` directory, build dump\_syms.xcodeproj, and copy
    tools/mac/dump\_syms/build/Release/dump\_syms to a safe location where it
    can be run during the build process.

## Adding Breakpad.framework

Inside your application's framework, add the Breakpad.Framework to your
project's framework settings. When you select it from the file chooser, it will
let you pick a target to add it to; go ahead and check the one that's relevant
to your application.

## Copy Breakpad into your Application Package

Copy Breakpad into your Application Package, so it will be around at run time.

Go to the Targets section of your Xcode Project window. Hit the disclosure
triangle to reveal the build phases of your application. Add a new Copy Files
phase using the Contextual menu (Control Click). On the General panel of the new
'Get Info' of this new phase, set the destination to 'Frameworks' Close the
'Info' panel. Use the Contextual Menu to Rename your new phase 'Copy Frameworks'
Now drag Breakpad again into this Copy Frameworks phase. Drag it from whereever
it appears in the project file tree.

## Add a New Run Script build phase

Near the end of the build phases, add a new Run Script build phase. This will be
run before Xcode calls /usr/bin/strip on your project. This is where you'll be
calling dump\_sym to output the symbols for each architecture of your build. In
my case, the relevant lines read:

```
#!/bin/sh
$TOOL_DIR=<location of dump_syms from step 3 above>

"$TOOL_DIR/dump_syms" -a ppc "$PROD" > "$TARGET_NAME ppc.breakpad"

"$TOOL_DIR/dump_syms" -a i386 "$PROD" > "$TARGET_NAME i386.breakpad"
```

## Adjust the Project Settings

*   Turn on Separate Strip,
*   Set the Strip Style to Non-Global Symbols.

## Write Code!

You'll need to have an object that acts as the delegate for NSApplication.
Inside this object's header, you'll need to add

1.  add an ivar for Breakpad and
2.  a declaration for the applicationShouldTerminate:(NSApplication`*` sender)
    message.

```
#import <Breakpad/Breakpad.h>

@interface BreakpadTest : NSObject {
   .
   .
   .
   BreakpadRef breakpad;
   .
   .
   .
}
.
.
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
.
.
@end
```

Inside your object's implementation file,

1.  add the following method InitBreakpad
2.  modify your awakeFromNib method to look like the one below,
3.  modify/add your application's delegate method to look like the one below

```
static BreakpadRef InitBreakpad(void) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  BreakpadRef breakpad = 0;
  NSDictionary *plist = [[NSBundle mainBundle] infoDictionary];
  if (plist) {
    // Note: version 1.0.0.4 of the framework changed the type of the argument 
    // from CFDictionaryRef to NSDictionary * on the next line:
    breakpad = BreakpadCreate(plist);
  }
  [pool release];
  return breakpad;
}

- (void)awakeFromNib {
  breakpad = InitBreakpad();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
  BreakpadRelease(breakpad);
  return NSTerminateNow;
}
```

## Configure Breakpad

Configure Breakpad for your application.

1.  Take a look inside the Breakpad.framework at the Breakpad.h file for the
    keys, default values, and descriptions to be passed to BreakpadCreate().
2.  Add/Edit the Breakpad specific entries in the dictionary passed to
    BreakpadCreate() -- typically your application's info plist.

Example from the Notifier Info.plist:
`<key>BreakpadProduct</key><string>Google_Notifier_Mac</string>
<key>BreakpadProductDisplay</key><string>${PRODUCT_NAME}</string>
`

## Build Your Application

Almost done!

## Verify

Double-check:

Your app should have in its package contents:
myApp.app/Contents/Frameworks/Breakpad.framework.

The symbol files have reasonable contents (you can look at them with a text
editor.)

Look again at the Copy Frameworks phase of your project. Are you leaking .h
files? Select them and delete them. (If you drag a bunch of files into your
project, Xcode often wants to copy your .h files into the build, revealing
Google secrets. Be vigilant!)

## Upload the symbol file

You'll need to configure your build process to store symbols in a location that
is accessible by the minidump processor. There is a tool in tools/mac/symupload
that can be used to send the symbol file via HTTP post.

1.  Test

Configure breakpad to send reports to a URL by adding to your app's Info.plist:

```
<key>BreakpadURL</key>
<string>upload URL</string>
<key>BreakpadReportInterval</key>
<string>30</string>
```

## Final Notes

Breakpad checks whether it is being run under a debugger, and if so, normally
does nothing. But, you can force Breakpad to function under a debugger by
setting the Unix shell variable BREAKPAD\_IGNORE\_DEBUGGER to a non-zero value.
You can bracket the source code in the above Write The Code step with #if DEBUG
to completely eliminate it from Debug builds. See
//depot/googlemac/GoogleNotifier/main.m for an example. FYI, when your process
forks(), exception handlers are reset to the default for child processes. So
they must reinitialize Breakpad, otherwise exceptions will be handled by Apple's
Crash Reporter.
