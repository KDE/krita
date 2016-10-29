// Copyright (c) 2006, Google Inc.
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

// symupload.cc: Upload a symbol file to a HTTP server.  The upload is sent as
// a multipart/form-data POST request with the following parameters:
//  code_file: the basename of the module, e.g. "app"
//  debug_file: the basename of the debugging file, e.g. "app"
//  debug_identifier: the debug file's identifier, usually consisting of
//                    the guid and age embedded in the pdb, e.g.
//                    "11111111BBBB3333DDDD555555555555F"
//  version: the file version of the module, e.g. "1.2.3.4"
//  os: the operating system that the module was built for
//  cpu: the CPU that the module was built for
//  symbol_file: the contents of the breakpad-format symbol file

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/linux/symbol_upload.h"

using google_breakpad::sym_upload::Options;

//=============================================================================
static void
Usage(int argc, const char *argv[]) {
  fprintf(stderr, "Submit symbol information.\n");
  fprintf(stderr, "Usage: %s [options...] <symbols> <upload-URL>\n", argv[0]);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "<symbols> should be created by using the dump_syms tool.\n");
  fprintf(stderr, "<upload-URL> is the destination for the upload\n");
  fprintf(stderr, "-v:\t Version information (e.g., 1.2.3.4)\n");
  fprintf(stderr, "-x:\t <host[:port]> Use HTTP proxy on given port\n");
  fprintf(stderr, "-u:\t <user[:password]> Set proxy user and password\n");
  fprintf(stderr, "-h:\t Usage\n");
  fprintf(stderr, "-?:\t Usage\n");
}

//=============================================================================
static void
SetupOptions(int argc, const char *argv[], Options *options) {
  extern int optind;
  int ch;

  while ((ch = getopt(argc, (char * const *)argv, "u:v:x:h?")) != -1) {
    switch (ch) {
      case 'h':
      case '?':
        Usage(argc, argv);
        exit(0);
        break;
      case 'u':
        options->proxy_user_pwd = optarg;
        break;
      case 'v':
        options->version = optarg;
        break;
      case 'x':
        options->proxy = optarg;
        break;

      default:
        fprintf(stderr, "Invalid option '%c'\n", ch);
        Usage(argc, argv);
        exit(1);
        break;
    }
  }

  if ((argc - optind) != 2) {
    fprintf(stderr, "%s: Missing symbols file and/or upload-URL\n", argv[0]);
    Usage(argc, argv);
    exit(1);
  }

  options->symbolsPath = argv[optind];
  options->uploadURLStr = argv[optind + 1];
}

//=============================================================================
int main(int argc, const char* argv[]) {
  Options options;
  SetupOptions(argc, argv, &options);
  google_breakpad::sym_upload::Start(&options);
  return options.success ? 0 : 1;
}
