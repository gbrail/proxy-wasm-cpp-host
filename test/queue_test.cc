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

#include <functional>
#include <memory>

#include "include/proxy-wasm/compat.h"
#include "include/proxy-wasm/context.h"
#include "include/proxy-wasm/null.h"
#include "include/proxy-wasm/wasm.h"
#include "test/mocks.h"

#include "gtest/gtest.h"

using proxy_wasm::CallOnThreadFunction;
using proxy_wasm::ContextBase;
using proxy_wasm::SharedQueueDequeueToken;
using proxy_wasm::SharedQueueEnqueueToken;
using proxy_wasm::WasmBase;
using proxy_wasm::WasmResult;
using proxy_wasm::WasmVm;

namespace {

class TestContext : public ContextBase {
public:
  TestContext(WasmBase *base) : ContextBase(base) {}
  void onQueueReady(SharedQueueDequeueToken token) override {
    ready_token_ = token;
  }

  void resetReadyToken() {
    ready_token_ = UINT_MAX;
  }

  SharedQueueDequeueToken readyToken() const {
    return ready_token_;
  }

 private:
  SharedQueueDequeueToken ready_token_ = UINT_MAX;
};

class QueueTest : public ::testing::Test {
protected:
  void SetUp() {
    auto base = proxy_wasm::createWasm("KeyKeyKey", "", nullptr, 
      proxy_wasm::test::createMockHandle,
      proxy_wasm::test::cloneMockHandle, true);
    ASSERT_TRUE(base);
    ctx_.reset(new TestContext(base->wasm().get()));
  }

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
  EXPECT_EQ(ctx_->readyToken(), UINT_MAX);

  status = ctx_->enqueueSharedQueue(enq_token, "This is a test");
  ASSERT_EQ(status, WasmResult::Ok);
  status = ctx_->dequeueSharedQueue(deq_token, &data);
  ASSERT_EQ(status, WasmResult::Ok);
  EXPECT_EQ(data, "This is a test");
  EXPECT_EQ(ctx_->readyToken(), deq_token);

  // It should be empty again
  ctx_->resetReadyToken();
  status = ctx_->dequeueSharedQueue(deq_token, &data);
  EXPECT_EQ(status, WasmResult::Empty);
  EXPECT_EQ(ctx_->readyToken(), UINT_MAX);
}

} // namespace
