// Copyright (c) 2011 Google Inc.
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

// symbol_upload.cc: implemented google_breakpad::sym_upload::Start, a helper
// function for linux symbol upload tool.

#include "common/linux/http_upload.h"
#include "common/linux/symbol_upload.h"

#include <assert.h>
#include <stdio.h>

#include <functional>
#include <vector>

namespace google_breakpad {
namespace sym_upload {

void TokenizeByChar(const string &source_string, int c,
                    std::vector<string> *results) {
  assert(results);
  string::size_type cur_pos = 0, next_pos = 0;
  while ((next_pos = source_string.find(c, cur_pos)) != string::npos) {
    if (next_pos != cur_pos)
      results->push_back(source_string.substr(cur_pos, next_pos - cur_pos));
    cur_pos = next_pos + 1;
  }
  if (cur_pos < source_string.size() && next_pos != cur_pos)
    results->push_back(source_string.substr(cur_pos));
}

//=============================================================================
// Parse out the module line which have 5 parts.
// MODULE <os> <cpu> <uuid> <module-name>
bool ModuleDataForSymbolFile(const string &file,
                             std::vector<string> *module_parts) {
  assert(module_parts);
  const size_t kModulePartNumber = 5;
  FILE* fp = fopen(file.c_str(), "r");
  if (fp) {
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), fp)) {
      string line(buffer);
      string::size_type line_break_pos = line.find_first_of('\n');
      if (line_break_pos == string::npos) {
        assert(0 && "The file is invalid!");
        fclose(fp);
        return false;
      }
      line.resize(line_break_pos);
      const char kDelimiter = ' ';
      TokenizeByChar(line, kDelimiter, module_parts);
      if (module_parts->size() != kModulePartNumber)
        module_parts->clear();
    }
    fclose(fp);
  }

  return module_parts->size() == kModulePartNumber;
}

//=============================================================================
string CompactIdentifier(const string &uuid) {
  std::vector<string> components;
  TokenizeByChar(uuid, '-', &components);
  string result;
  for (size_t i = 0; i < components.size(); ++i)
    result += components[i];
  return result;
}

//=============================================================================
void Start(Options *options) {
  std::map<string, string> parameters;
  options->success = false;
  std::vector<string> module_parts;
  if (!ModuleDataForSymbolFile(options->symbolsPath, &module_parts)) {
    fprintf(stderr, "Failed to parse symbol file!\n");
    return;
  }

  string compacted_id = CompactIdentifier(module_parts[3]);

  // Add parameters
  if (!options->version.empty())
    parameters["version"] = options->version;

  // MODULE <os> <cpu> <uuid> <module-name>
  // 0      1    2     3      4
  parameters["os"] = module_parts[1];
  parameters["cpu"] = module_parts[2];
  parameters["debug_file"] = module_parts[4];
  parameters["code_file"] = module_parts[4];
  parameters["debug_identifier"] = compacted_id;

  std::map<string, string> files;
  files["symbol_file"] = options->symbolsPath;

  string response, error;
  long response_code;
  bool success = HTTPUpload::SendRequest(options->uploadURLStr,
                                         parameters,
                                         files,
                                         options->proxy,
                                         options->proxy_user_pwd,
                                         "",
                                         &response,
                                         &response_code,
                                         &error);

  if (!success) {
    printf("Failed to send symbol file: %s\n", error.c_str());
    printf("Response code: %ld\n", response_code);
    printf("Response:\n");
    printf("%s\n", response.c_str());
  } else if (response_code == 0) {
    printf("Failed to send symbol file: No response code\n");
  } else if (response_code != 200) {
    printf("Failed to send symbol file: Response code %ld\n", response_code);
    printf("Response:\n");
    printf("%s\n", response.c_str());
  } else {
    printf("Successfully sent the symbol file.\n");
  }
  options->success = success;
}

}  // namespace sym_upload
}  // namespace google_breakpad
