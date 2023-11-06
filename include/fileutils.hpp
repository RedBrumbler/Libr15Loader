// MIT License

// Copyright (c) 2019 Anairkoen Schno

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <jni.h>

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

#include "_config.h"

namespace fileutils {

struct MAIN_LOCAL path_container {
  std::filesystem::path modloaderSearchPath;
  std::filesystem::path filesDir;
  std::filesystem::path externalDir;
};

MAIN_LOCAL std::optional<path_container> getDirs(JNIEnv* env, std::string_view application_id);
MAIN_LOCAL std::optional<std::string> getApplicationId();
MAIN_LOCAL bool ensurePerms(JNIEnv* env, std::string_view application_id);

namespace literals {

// To facilitate easier combination
MAIN_LOCAL inline std::filesystem::path operator""_fp(char const* p, [[maybe_unused]] std::size_t sz) {
  return std::filesystem::path(p);
}

}  // namespace literals

}  // namespace fileutils
