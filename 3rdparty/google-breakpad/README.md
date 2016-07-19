# Breakpad

Breakpad is a set of client and server components which implement a
crash-reporting system.

* [Homepage](https://chromium.googlesource.com/breakpad/breakpad/)
* [Documentation](https://chromium.googlesource.com/breakpad/breakpad/+/master/docs/)
* [Bugs](https://bugs.chromium.org/p/google-breakpad/)
* Discussion/Questions: [google-breakpad-discuss@googlegroups.com](https://groups.google.com/d/forum/google-breakpad-discuss)
* Developer/Reviews: [google-breakpad-dev@googlegroups.com](https://groups.google.com/d/forum/google-breakpad-dev)

## Getting started in 32-bit mode (from trunk)

```sh
# Configure
CXXFLAGS=-m32 CFLAGS=-m32 CPPFLAGS=-m32 ./configure
# Build
make
# Test
make check
# Install
make install
```

If you need to reconfigure your build be sure to run `make distclean` first.

## To request change review:

1.  Get a copy of depot_tools repo.
    http://dev.chromium.org/developers/how-tos/install-depot-tools

2.  Create a new directory for checking out the source code.
    mkdir breakpad && cd breakpad

3.  Run the `fetch` tool from depot_tools to download all the source repos.
    `fetch breakpad`

4.  Make changes. Build and test your changes.
    For core code like processor use methods above.
    For linux/mac/windows, there are test targets in each project file.

5.  Commit your changes to your local repo and upload them to the server.
    http://dev.chromium.org/developers/contributing-code
    e.g. `git commit ... && git cl upload ...`
    You will be prompted for credential and a description.

6.  At https://codereview.chromium.org/ you'll find your issue listed; click on
    it, and select Publish+Mail, and enter in the code reviewer and CC
    google-breakpad-dev@googlegroups.com
