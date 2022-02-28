Notes on packaging Krita with G’MIC
==================================

Krita 3 and later are compatible with G’MIC, an open-source digital
image processing framework. This support is provided by G’MIC-Qt, a
Qt-based frontend for G’MIC. Since its inception, G’MIC-Qt was shipped
as a standalone, externally built executable that is an optional,
runtime dependency of Krita.

Krita 5 changes the way G’MIC-Qt is consumed. In order to support CentOS
and macOS, G’MIC-Qt has been converted into a dynamically loadable
library that is a dependent of Krita.

This file reviews these changes, and how to package Krita accordingly.

Rationale
---------

We have chosen to ship G’MIC-Qt as a library because of two longstanding
bugs.

The Krita host for G’MIC-Qt relies on `QSharedMemory`, i.e. a shared
memory segment, on wich a pipe is instantiated to pass messages to and
from the host app. Firstly, this approach made opening two simultaneous
G’MIC-Qt instances (each paired to its own Krita instance) impossible
[^1]. Secondly, it also forbade using G’MIC-Qt with Krita on CentOS, as
well as macOS, because the former doesn’t support `QSharedMemory` [^2],
and the latter has a meager 4KB as the maximum shared segment size.
While there’s no workaround (to our knowledge) in CentOS, the only
workaround for macOS is to manipulate the maximum segment size via
`sysctl` [^3], which was already difficult pre-Mojave [^4] and now, due
to the significant security measures of recent macOS versions, is
nothing short of a sysadmin task [^5].

There were two approaches. One was to move to a `mmap`-d file, which is
unpredictable to sync due to each canvas’s differing space requirements.
The easiest, and the one we chose, was to move to a tighter coupled
memory model– a dynamically loadable plugin, as shown in my proposal PR
[^6]. This was rejected by the G’MIC developers because of the
possibility of crashing the host app due to a G’MIC internal bug
[^7][^8]. This decision was later enacted as part of G’MIC contributing
policies [^9].

How did you fix it?
-------------------

Due to the above, the only path forward was to fork G’MIC, which we did
in Krita MR !581 [^10].

From a source code point of view, our fork is based on top of the latest
version’s tarball. Each tarball’s contents are committed to the `main`
branch of the `amyspark/gmic` GitHub repository [^11]. For every covered
release, there is a branch that in turn overlays our own plugin
implementation, along with additional fixes that ensure that G’MIC-Qt
doesn’t attempt to overwrite the internal state of the host application;
namely, `QCoreApplication` settings, widget styles, and the installed
translators.

From a technical point of view, this library interfaces with Krita
through a new, purpose specific library, `kisqmicinterface`. This
library contains nothing more than the previous iteration of the
communications system, but now exported through namesake APIs [^12].

In short, we have reversed the dependency flow; while in Krita v4 and
earlier G’MIC-Qt was a runtime dependency, in v5, it’s G’MIC-Qt that
depends on Krita as a build *and* runtime dependency.

Getting the source code
-----------------------

The patched version’s tarballs are GPG signed and available at the
Releases section of the GitHub repository [^13]. Alternatively, the
tarballs (though not the signatures) are also mirrored at our
dependencies stash at files.kde.org [^14]. The tarballs are signed with
the GPG key which is available at my GitHub profile. Its fingerprint is
`4894424D2412FEE5176732A3FC00108CFD9DBF1E`.

Building Krita’s G’MIC-Qt library
---------------------------------

After building Krita with your standard process, the CMake install
process should have put `kisqmicinterface.so` in your `lib` folder:

    [2022-01-09T16:21:32.589Z] -- Installing: /home/appimage/appimage-workspace/krita.appdir/usr/lib/x86_64-linux-gnu/libkritaqmicinterface.so.18.0.0
    [2022-01-09T16:21:32.589Z] -- Installing: /home/appimage/appimage-workspace/krita.appdir/usr/lib/x86_64-linux-gnu/libkritaqmicinterface.so.18
    [2022-01-09T16:21:32.589Z] -- Set runtime path of "/home/appimage/appimage-workspace/krita.appdir/usr/lib/x86_64-linux-gnu/libkritaqmicinterface.so.18.0.0" to "/home/appimage/appimage-workspace/krita.appdir/usr/lib/x86_64-linux-gnu:/home/appimage/appimage-workspace/deps/usr/lib:/home/appimage/appimage-workspace/deps/usr/lib/x86_64-linux-gnu"
    [2022-01-09T16:21:32.589Z] -- Installing: /home/appimage/appimage-workspace/krita.appdir/usr/lib/x86_64-linux-gnu/libkritaqmicinterface.so

It should also install these headers, as illustrated below:

-   `kis_qmic_plugin_interface.h` exports a G’MIC-alike `launch` entry
    point that the plugin will implement
-   `kis_qmic_interface.h` implements the G’MIC request-response APIs
-   `kritaqmicinterface_export.h` is the CMake auto-generated export
    decoration header

<!-- -->

    [2022-01-09T16:21:32.589Z] -- Installing: /home/appimage/appimage-workspace/krita.appdir/usr/include/kis_qmic_interface.h
    [2022-01-09T16:21:32.589Z] -- Installing: /home/appimage/appimage-workspace/krita.appdir/usr/include/kis_qmic_plugin_interface.h
    [2022-01-09T16:21:32.589Z] -- Installing: /home/appimage/appimage-workspace/krita.appdir/usr/include/kritaqmicinterface_export.h

The three headers, along with the `libkritaqmicinterface.a` archive
library (if building for Windows under MinGW), comprise a `krita-gmic-dev`
package that’ll be a build dependency of the new G’MIC-Qt plugin.
Please note that `libkritaqmicinterface.so` is consumed
by Krita and MUST NOT be placed inside this dev package.

Now, download the G’MIC-Qt tarball from one of the sources listed
previously, and unpack it to an isolated directory. Then, you can build
it with these lines (adjust them as described):

    mkdir build
    cmake -S ./gmic-$<the tarball's G'MIC version>-patched/gmic-qt \
          -B ./build \
          -DCMAKE_PREFIX_PATH=$<installation prefix of krita-gmic-dev> \
          -DCMAKE_INSTALL_PREFIX=$<installation prefix of krita itself> \ 
          -DENABLE_SYSTEM_GMIC=$<false if you don't want to use your system's G'MIC> \
          -DGMIC_QT_HOST=krita-plugin
    cmake --build . --config $<your desired build type> --target install

The changes from a standard G’MIC build are:

-   the new `GMIC_QT_HOST` value, `krita-plugin`
-   the requirement for the `krita-gmic-dev` package to be available in
    `CMAKE_PREFIX_PATH`

This process is illustrated in any of our official build scripts for
Windows [^15] and for macOS/Linux [^16]. You can also check the
`3rdparty_plugins` section of our source tree [^17] to see what other
hardening we apply to the build.

------------------------------------------------------------------------

[^1]: Bug \#44 on c-koi/gmic-qt: “CentOS7: Krita 4.0.4 + gmic\_krita\_qt
    2.3.0/2.2.3 - QSharedMemory::attach”.
    <https://github.com/c-koi/gmic-qt/issues/44>

[^2]: Bug 424514 on krita: “Guaranteed crash when opening 2 G’MIC-qt”.
    <https://bugs.kde.org/show_bug.cgi?id=424514>

[^3]: <https://www.ssec.wisc.edu/mcidas/doc/users_guide/2017.1/SharedMemory.html>

[^4]: e.g. <https://stackoverflow.com/questions/2017004/setting-shmmax-etc-values-on-mac-os-x-10-6-for-postgresql>

[^5]: See <https://developer.apple.com/forums/thread/669625> for an
    approach applicable after `com.apple.rootless` was fully enforced.

[^6]: <https://github.com/c-koi/gmic-qt/pull/102>

[^7]: <https://github.com/c-koi/gmic-qt/pull/102>

[^8]: Note that this was already possible, simply by crashing G’MIC the
    host app would deadlock, waiting forever for a response.

[^9]: <https://github.com/c-koi/gmic-qt/blob/master/NEW_HOST_HOWTO.md>

[^10]: <https://invent.kde.org/graphics/krita/-/merge_requests/581>

[^11]: <https://github.com/amyspark/gmic>

[^12]: An older version of what’s
    <https://github.com/c-koi/gmic-qt/blob/master/src/Host/GmicQtHost.h>
    nowadays.

[^13]: <https://github.com/amyspark/gmic/releases>

[^14]: <https://files.kde.org/krita/build/dependencies/>

[^15]: <https://invent.kde.org/graphics/krita/-/tree/master/build-tools/windows>

[^16]: <https://invent.kde.org/graphics/krita/-/blob/master/packaging>

[^17]: <https://invent.kde.org/graphics/krita/-/tree/master/3rdparty_plugins>
