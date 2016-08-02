// Copyright (c) 2012 Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef CLIENT_LINUX_HANDLER_MINIDUMP_DESCRIPTOR_H_
#define CLIENT_LINUX_HANDLER_MINIDUMP_DESCRIPTOR_H_

#include <assert.h>
#include <sys/types.h>

#include <string>

#include "client/linux/handler/microdump_extra_info.h"
#include "common/using_std_string.h"

// This class describes how a crash dump should be generated, either:
// - Writing a full minidump to a file in a given directory (the actual path,
//   inside the directory, is determined by this class).
// - Writing a full minidump to a given fd.
// - Writing a reduced microdump to the console (logcat on Android).
namespace google_breakpad {

class MinidumpDescriptor {
 public:
  struct MicrodumpOnConsole {};
  static const MicrodumpOnConsole kMicrodumpOnConsole;

  MinidumpDescriptor()
      : mode_(kUninitialized),
        fd_(-1),
        size_limit_(-1) {}

  explicit MinidumpDescriptor(const string& directory)
      : mode_(kWriteMinidumpToFile),
        fd_(-1),
        directory_(directory),
        c_path_(NULL),
        size_limit_(-1) {
    assert(!directory.empty());
  }

  explicit MinidumpDescriptor(int fd)
      : mode_(kWriteMinidumpToFd),
        fd_(fd),
        c_path_(NULL),
        size_limit_(-1) {
    assert(fd != -1);
  }

  explicit MinidumpDescriptor(const MicrodumpOnConsole&)
      : mode_(kWriteMicrodumpToConsole),
        fd_(-1),
        size_limit_(-1) {}

  explicit MinidumpDescriptor(const MinidumpDescriptor& descriptor);
  MinidumpDescriptor& operator=(const MinidumpDescriptor& descriptor);

  static MinidumpDescriptor getMicrodumpDescriptor();

  bool IsFD() const { return mode_ == kWriteMinidumpToFd; }

  int fd() const { return fd_; }

  string directory() const { return directory_; }

  const char* path() const { return c_path_; }

  bool IsMicrodumpOnConsole() const {
    return mode_ == kWriteMicrodumpToConsole;
  }

  // Updates the path so it is unique.
  // Should be called from a normal context: this methods uses the heap.
  void UpdatePath();

  off_t size_limit() const { return size_limit_; }
  void set_size_limit(off_t limit) { size_limit_ = limit; }

  MicrodumpExtraInfo* microdump_extra_info() {
    assert(IsMicrodumpOnConsole());
    return &microdump_extra_info_;
  };

 private:
  enum DumpMode {
    kUninitialized = 0,
    kWriteMinidumpToFile,
    kWriteMinidumpToFd,
    kWriteMicrodumpToConsole
  };

  // Specifies the dump mode (see DumpMode).
  DumpMode mode_;

  // The file descriptor where the minidump is generated.
  int fd_;

  // The directory where the minidump should be generated.
  string directory_;

  // The full path to the generated minidump.
  string path_;

  // The C string of |path_|. Precomputed so it can be access from a compromised
  // context.
  const char* c_path_;

  off_t size_limit_;

  // The extra microdump data (e.g. product name/version, build
  // fingerprint, gpu fingerprint) that should be appended to the dump
  // (microdump only). Microdumps don't have the ability of appending
  // extra metadata after the dump is generated (as opposite to
  // minidumps MIME fields), therefore the extra data must be provided
  // upfront. Any memory pointed to by members of the
  // MicrodumpExtraInfo struct must be valid for the lifetime of the
  // process (read: the caller has to guarantee that it is stored in
  // global static storage.)
  MicrodumpExtraInfo microdump_extra_info_;
};

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_HANDLER_MINIDUMP_DESCRIPTOR_H_
