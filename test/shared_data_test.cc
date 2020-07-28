// Copyright 2016-2019 Envoy Project Authors
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

#include "gtest/gtest.h"

#include "include/proxy-wasm/shared_data.h"

using proxy_wasm::shared_data::SharedData;
using proxy_wasm::WasmResult;

namespace {

class SharedDataTest : public ::testing::Test {
 protected:
  SharedData shared_;
};

TEST_F(SharedDataTest, GetNotFound) {
  std::pair<std::string, uint32_t> result;
  auto status = shared_.get("vm_id", "NotFound", &result);
  ASSERT_EQ(status, WasmResult::NotFound);
}

TEST_F(SharedDataTest, VmIdMismatch) {
  auto status = shared_.set("vm_id", "one", "first", 0);
  ASSERT_EQ(status, WasmResult::Ok);

  std::pair<std::string, uint32_t> result;
  status = shared_.get("wrong_vm_id", "one", &result);
  ASSERT_EQ(status, WasmResult::NotFound);
}

TEST_F(SharedDataTest, KeyMismatch) {
  auto status = shared_.set("vm_id", "one", "first", 0);
  ASSERT_EQ(status, WasmResult::Ok);

  std::pair<std::string, uint32_t> result;
  status = shared_.get("vm_id", "NotFound", &result);
  ASSERT_EQ(status, WasmResult::NotFound);
}

TEST_F(SharedDataTest, SetGet) {
  auto status = shared_.set("vm_id", "one", "first", 0);
  ASSERT_EQ(status, WasmResult::Ok);

  std::pair<std::string, uint32_t> result;
  status = shared_.get("vm_id", "one", &result);
  ASSERT_EQ(status, WasmResult::Ok);
  EXPECT_EQ(result.first, "first");
  
  status = shared_.set("vm_id", "one", "second", result.second);
  ASSERT_EQ(status, WasmResult::Ok);

  status = shared_.get("vm_id", "one", &result);
  ASSERT_EQ(status, WasmResult::Ok);
  EXPECT_EQ(result.first, "second");
}

TEST_F(SharedDataTest, CasMismatch) {
  auto status = shared_.set("vm_id", "one", "first", 0);
  ASSERT_EQ(status, WasmResult::Ok);

  std::pair<std::string, uint32_t> result;
  status = shared_.get("vm_id", "one", &result);
  ASSERT_EQ(status, WasmResult::Ok);
  EXPECT_EQ(result.first, "first");
  
  status = shared_.set("vm_id", "one", "second", result.second + 1);
  ASSERT_EQ(status, WasmResult::CasMismatch);
}

}