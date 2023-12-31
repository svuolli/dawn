// Copyright 2023 The Tint Authors.
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

#include "gmock/gmock.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/ir/program_test_helper.h"
#include "src/tint/lang/wgsl/ast/case_selector.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_FromProgramMaterializeTest = ProgramTestHelper;

TEST_F(IR_FromProgramMaterializeTest, EmitExpression_MaterializedCall) {
    auto* expr = Return(Call("trunc", 2.5_f));

    Func("test_function", {}, ty.f32(), expr, utils::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test_function = func():f32 -> %b1 {
  %b1 = block {
    ret 2.0f
  }
}
)");
}

}  // namespace
}  // namespace tint::ir
