// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>

#include "include/proxy-wasm/compat.h"
#include "include/proxy-wasm/null.h"
#include "include/proxy-wasm/wasm.h"

namespace proxy_wasm {
namespace test {

inline void CallOnThisThread(std::function<void()> f) {
  f();
}

class MockWasm : public WasmBase {
public:
  MockWasm(std::unique_ptr<WasmVm> wasm_vm, string_view vm_id,
           string_view vm_configuration, string_view vm_key)
      : WasmBase(std::move(wasm_vm), vm_id, vm_configuration, vm_key) {}

  virtual void error(string_view message) { 
    std::cerr << "Mock WASM failure: " << message << "\n";
  }

  virtual CallOnThreadFunction callOnThreadFunction() {
    return CallOnThisThread;
  }
};

// WasmHandleFactory implementation
std::shared_ptr<WasmHandleBase> createMockHandle(string_view vm_key) {
  auto wasm = std::make_shared<MockWasm>(createNullVm(), vm_key, "", vm_key);
  return std::make_shared<WasmHandleBase>(wasm);
}

// WasmHandleCloneFactory implementation
std::shared_ptr<WasmHandleBase> cloneMockHandle(std::shared_ptr<WasmHandleBase> wasm) {
  // For the purposes of testing (only) it's OK to not clone
  return wasm;
}

}  // namespace test
}  // namespace proxy_wasm