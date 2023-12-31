// Copyright 2023 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_DAWN_UTILS_BINARYSEMAPHORE_H_
#define SRC_DAWN_UTILS_BINARYSEMAPHORE_H_

#include <condition_variable>
#include <mutex>

namespace dawn::utils {

// Binary semaphore implementation. Could eventually be replaced with std::binary_semaphore when
// C++20 is available everywhere.
class BinarySemaphore {
  public:
    void Release();
    void Acquire();

  private:
    std::mutex mMutex;
    std::condition_variable mCv;
    bool mSignaled = false;
};

}  // namespace dawn::utils

#endif  // SRC_DAWN_UTILS_BINARYSEMAPHORE_H_
