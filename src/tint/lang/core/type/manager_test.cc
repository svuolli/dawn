// Copyright 2022 The Tint Authors.
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

#include "src/tint/lang/core/type/manager.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/u32.h"

namespace tint::type {
namespace {

template <typename T>
size_t count(const T& range_loopable) {
    size_t n = 0;
    for (auto it : range_loopable) {
        (void)it;
        n++;
    }
    return n;
}

using ManagerTest = testing::Test;

TEST_F(ManagerTest, GetUnregistered) {
    Manager tm;
    auto* t = tm.Get<I32>();
    ASSERT_NE(t, nullptr);
    EXPECT_TRUE(t->Is<I32>());
}

TEST_F(ManagerTest, GetSameTypeReturnsSamePtr) {
    Manager tm;
    auto* t = tm.Get<I32>();
    ASSERT_NE(t, nullptr);
    EXPECT_TRUE(t->Is<I32>());

    auto* t2 = tm.Get<I32>();
    EXPECT_EQ(t, t2);
}

TEST_F(ManagerTest, GetDifferentTypeReturnsDifferentPtr) {
    Manager tm;
    Type* t = tm.Get<I32>();
    ASSERT_NE(t, nullptr);
    EXPECT_TRUE(t->Is<I32>());

    Type* t2 = tm.Get<U32>();
    ASSERT_NE(t2, nullptr);
    EXPECT_NE(t, t2);
    EXPECT_TRUE(t2->Is<U32>());
}

TEST_F(ManagerTest, CppToType) {
    Manager tm;
    const Type* b1 = tm.Get<bool>();
    const Type* b2 = tm.Get<Bool>();
    ASSERT_EQ(b1, b2);

    const Type* i1 = tm.Get<i32>();
    const Type* i2 = tm.Get<I32>();
    ASSERT_EQ(i1, i2);

    const Type* u1 = tm.Get<u32>();
    const Type* u2 = tm.Get<U32>();
    ASSERT_EQ(u1, u2);

    const Type* f1 = tm.Get<f32>();
    const Type* f2 = tm.Get<F32>();
    ASSERT_EQ(f1, f2);

    const Type* h1 = tm.Get<f16>();
    const Type* h2 = tm.Get<F16>();
    ASSERT_EQ(h1, h2);
}

TEST_F(ManagerTest, Find) {
    Manager tm;
    auto* created = tm.Get<I32>();

    EXPECT_EQ(tm.Find<U32>(), nullptr);
    EXPECT_EQ(tm.Find<I32>(), created);
}

TEST_F(ManagerTest, WrapDoesntAffectInner) {
    Manager inner;
    Manager outer = Manager::Wrap(inner);

    inner.Get<I32>();

    EXPECT_EQ(count(inner), 1u);
    EXPECT_EQ(count(outer), 0u);

    outer.Get<U32>();

    EXPECT_EQ(count(inner), 1u);
    EXPECT_EQ(count(outer), 1u);
}

}  // namespace
}  // namespace tint::type
