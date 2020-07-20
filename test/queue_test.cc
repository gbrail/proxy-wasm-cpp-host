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

#include "include/proxy-wasm/context.h"
#include "include/proxy-wasm/null.h"
#include "include/proxy-wasm/wasm.h"

#include "gtest/gtest.h"

using proxy_wasm::ContextBase;
using proxy_wasm::SharedQueueDequeueToken;
using proxy_wasm::SharedQueueEnqueueToken;
using proxy_wasm::WasmBase;
using proxy_wasm::WasmResult;
using proxy_wasm::WasmVm;

namespace {

class TestContext : public ContextBase {
 public:
  TestContext(WasmBase* base) : ContextBase(base) {}
  void onQueueReady(SharedQueueDequeueToken token) override {
  }
};

class QueueTest : public ::testing::Test {
 protected:
  void SetUp() {
    base_.reset(new WasmBase(proxy_wasm::createNullVm(), "TestVm", "", "TestVmKey"));
    ctx_.reset(new TestContext(base_.get()));
  }

  std::unique_ptr<WasmBase> base_;
  std::unique_ptr<TestContext> ctx_;
};

TEST_F(QueueTest, NotFound) {
  SharedQueueDequeueToken token;
  auto result = ctx_->lookupSharedQueue("", "NotFound", &token);
  EXPECT_EQ(result, WasmResult::NotFound);
}

TEST_F(QueueTest, Register) {
  SharedQueueDequeueToken token;
  auto status = ctx_->registerSharedQueue("TestQueue", &token);
  ASSERT_EQ(status, WasmResult::Ok);
  status = ctx_->lookupSharedQueue("TestVm", "TestQueue", &token);
  ASSERT_EQ(status, WasmResult::Ok);
}

TEST_F(QueueTest, EnqDeq) {
  SharedQueueDequeueToken deq_token;
  auto status = ctx_->registerSharedQueue("TestQueue", &deq_token);
  ASSERT_EQ(status, WasmResult::Ok);

  SharedQueueEnqueueToken enq_token;
  status = ctx_->lookupSharedQueue("TestVm", "TestQueue", &enq_token);
  ASSERT_EQ(status, WasmResult::Ok);

  // Queue should be empty to start
  std::string data;
  status = ctx_->dequeueSharedQueue(deq_token, &data);
  EXPECT_EQ(status, WasmResult::Empty);

  status = ctx_->enqueueSharedQueue(enq_token, "This is a test");
  ASSERT_EQ(status, WasmResult::Ok);
  status = ctx_->dequeueSharedQueue(deq_token, &data);
  ASSERT_EQ(status, WasmResult::Ok);
  EXPECT_EQ(data, "This is a test");
}

}

