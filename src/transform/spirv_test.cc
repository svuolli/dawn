// Copyright 2021 The Tint Authors.
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

#include "src/transform/spirv.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using SpirvTest = TransformTest;

TEST_F(SpirvTest, HandleEntryPointIOTypes_Parameters) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[builtin(frag_coord)]] coord : vec4<f32>,
             [[location(1)]] loc1 : f32) {
  var col : f32 = (coord.x * loc1);
}

[[stage(compute)]]
fn compute_main([[builtin(local_invocation_id)]] local_id : vec3<u32>,
                [[builtin(local_invocation_index)]] local_index : u32) {
  var id_x : u32 = local_id.x;
}
)";

  auto* expect = R"(
[[builtin(frag_coord)]] var<in> tint_symbol : vec4<f32>;

[[location(1)]] var<in> tint_symbol_1 : f32;

[[stage(fragment)]]
fn frag_main() {
  var col : f32 = (tint_symbol.x * tint_symbol_1);
}

[[builtin(local_invocation_id)]] var<in> tint_symbol_2 : vec3<u32>;

[[builtin(local_invocation_index)]] var<in> tint_symbol_3 : u32;

[[stage(compute)]]
fn compute_main() {
  var id_x : u32 = tint_symbol_2.x;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_Parameter_TypeAlias) {
  auto* src = R"(
type myf32 = f32;

[[stage(fragment)]]
fn frag_main([[location(1)]] loc1 : myf32) {
}
)";

  auto* expect = R"(
type myf32 = f32;

[[location(1)]] var<in> tint_symbol : myf32;

[[stage(fragment)]]
fn frag_main() {
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_ReturnBuiltin) {
  auto* src = R"(
[[stage(vertex)]]
fn vert_main() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(1.0, 2.0, 3.0, 0.0);
}
)";

  auto* expect = R"(
[[builtin(position)]] var<out> tint_symbol_1 : vec4<f32>;

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
}

[[stage(vertex)]]
fn vert_main() {
  tint_symbol_2(vec4<f32>(1.0, 2.0, 3.0, 0.0));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_ReturnLocation) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[location(0)]] loc_in : u32) -> [[location(0)]] f32 {
  if (loc_in > 10u) {
    return 0.5;
  }
  return 1.0;
}
)";

  auto* expect = R"(
[[location(0)]] var<in> tint_symbol : u32;

[[location(0)]] var<out> tint_symbol_2 : f32;

fn tint_symbol_3(tint_symbol_1 : f32) {
  tint_symbol_2 = tint_symbol_1;
}

[[stage(fragment)]]
fn frag_main() {
  if ((tint_symbol > 10u)) {
    tint_symbol_3(0.5);
    return;
  }
  tint_symbol_3(1.0);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_ReturnLocation_TypeAlias) {
  auto* src = R"(
type myf32 = f32;

[[stage(fragment)]]
fn frag_main([[location(0)]] loc_in : u32) -> [[location(0)]] myf32 {
  if (loc_in > 10u) {
    return 0.5;
  }
  return 1.0;
}
)";

  auto* expect = R"(
type myf32 = f32;

[[location(0)]] var<in> tint_symbol : u32;

[[location(0)]] var<out> tint_symbol_2 : myf32;

fn tint_symbol_3(tint_symbol_1 : myf32) {
  tint_symbol_2 = tint_symbol_1;
}

[[stage(fragment)]]
fn frag_main() {
  if ((tint_symbol > 10u)) {
    tint_symbol_3(0.5);
    return;
  }
  tint_symbol_3(1.0);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_StructParameters) {
  auto* src = R"(
struct FragmentInput {
  [[builtin(frag_coord)]] coord : vec4<f32>;
  [[location(1)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) {
  var col : f32 = inputs.coord.x * inputs.value;
}
)";

  auto* expect = R"(
struct FragmentInput {
  coord : vec4<f32>;
  value : f32;
};

[[builtin(frag_coord)]] var<in> tint_symbol : vec4<f32>;

[[location(1)]] var<in> tint_symbol_1 : f32;

[[stage(fragment)]]
fn frag_main() {
  let tint_symbol_2 : FragmentInput = FragmentInput(tint_symbol, tint_symbol_1);
  var col : f32 = (tint_symbol_2.coord.x * tint_symbol_2.value);
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_StructParameters_EmptyBody) {
  auto* src = R"(
struct FragmentInput {
  [[location(1)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) {
}
)";

  auto* expect = R"(
struct FragmentInput {
  value : f32;
};

[[location(1)]] var<in> tint_symbol : f32;

[[stage(fragment)]]
fn frag_main() {
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_ReturnStruct) {
  auto* src = R"(
struct VertexOutput {
  [[builtin(position)]] pos : vec4<f32>;
  [[location(1)]] value : f32;
};

[[stage(vertex)]]
fn vert_main() -> VertexOutput {
  if (false) {
    return VertexOutput();
  }
  var pos : vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 0.0);
  return VertexOutput(pos, 2.0);
}
)";

  auto* expect = R"(
struct VertexOutput {
  pos : vec4<f32>;
  value : f32;
};

[[builtin(position)]] var<out> tint_symbol_1 : vec4<f32>;

[[location(1)]] var<out> tint_symbol_2 : f32;

fn tint_symbol_3(tint_symbol : VertexOutput) {
  tint_symbol_1 = tint_symbol.pos;
  tint_symbol_2 = tint_symbol.value;
}

[[stage(vertex)]]
fn vert_main() {
  if (false) {
    tint_symbol_3(VertexOutput());
    return;
  }
  var pos : vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 0.0);
  tint_symbol_3(VertexOutput(pos, 2.0));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_SharedStruct_SameShader) {
  auto* src = R"(
struct Interface {
  [[location(1)]] value : f32;
};

[[stage(vertex)]]
fn vert_main(inputs : Interface) -> Interface {
  return inputs;
}
)";

  auto* expect = R"(
struct Interface {
  value : f32;
};

[[location(1)]] var<in> tint_symbol : f32;

[[location(1)]] var<out> tint_symbol_3 : f32;

fn tint_symbol_4(tint_symbol_2 : Interface) {
  tint_symbol_3 = tint_symbol_2.value;
}

[[stage(vertex)]]
fn vert_main() {
  let tint_symbol_1 : Interface = Interface(tint_symbol);
  tint_symbol_4(tint_symbol_1);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_SharedStruct_DifferentShaders) {
  auto* src = R"(
struct Interface {
  [[location(1)]] value : f32;
};

[[stage(vertex)]]
fn vert_main() -> Interface {
  return Interface(42.0);
}

[[stage(fragment)]]
fn frag_main(inputs : Interface) {
  var x : f32 = inputs.value;
}
)";

  auto* expect = R"(
struct Interface {
  value : f32;
};

[[location(1)]] var<out> tint_symbol_1 : f32;

fn tint_symbol_2(tint_symbol : Interface) {
  tint_symbol_1 = tint_symbol.value;
}

[[stage(vertex)]]
fn vert_main() {
  tint_symbol_2(Interface(42.0));
  return;
}

[[location(1)]] var<in> tint_symbol_3 : f32;

[[stage(fragment)]]
fn frag_main() {
  let tint_symbol_4 : Interface = Interface(tint_symbol_3);
  var x : f32 = tint_symbol_4.value;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_StructLayoutDecorations) {
  auto* src = R"(
[[block]]
struct FragmentInput {
  [[size(16), location(1)]] value : f32;
  [[builtin(frag_coord)]] [[align(32)]] coord : vec4<f32>;
};

struct FragmentOutput {
  [[size(16), location(1)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(inputs.coord.x * inputs.value);
}
)";

  auto* expect = R"(
[[block]]
struct FragmentInput {
  [[size(16)]]
  value : f32;
  [[align(32)]]
  coord : vec4<f32>;
};

struct FragmentOutput {
  [[size(16)]]
  value : f32;
};

[[location(1)]] var<in> tint_symbol : f32;

[[builtin(frag_coord)]] var<in> tint_symbol_1 : vec4<f32>;

[[location(1)]] var<out> tint_symbol_4 : f32;

fn tint_symbol_5(tint_symbol_3 : FragmentOutput) {
  tint_symbol_4 = tint_symbol_3.value;
}

[[stage(fragment)]]
fn frag_main() {
  let tint_symbol_2 : FragmentInput = FragmentInput(tint_symbol, tint_symbol_1);
  tint_symbol_5(FragmentOutput((tint_symbol_2.coord.x * tint_symbol_2.value)));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_WithPrivateGlobalVariable) {
  // Test with a global variable to ensure that symbols are cloned correctly.
  // crbug.com/tint/701
  auto* src = R"(
var<private> x : f32;

struct VertexOutput {
  [[builtin(position)]] Position : vec4<f32>;
};

[[stage(vertex)]]
fn main() -> VertexOutput {
    return VertexOutput(vec4<f32>());
}
)";

  auto* expect = R"(
var<private> x : f32;

struct VertexOutput {
  Position : vec4<f32>;
};

[[builtin(position)]] var<out> tint_symbol_1 : vec4<f32>;

fn tint_symbol_2(tint_symbol : VertexOutput) {
  tint_symbol_1 = tint_symbol.Position;
}

[[stage(vertex)]]
fn main() {
  tint_symbol_2(VertexOutput(vec4<f32>()));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleSampleMaskBuiltins_Basic) {
  auto* src = R"(
[[builtin(sample_index)]] var<in> sample_index : u32;

[[builtin(sample_mask_in)]] var<in> mask_in : u32;

[[builtin(sample_mask_out)]] var<out> mask_out : u32;

[[stage(fragment)]]
fn main() {
  mask_out = mask_in;
}
)";

  auto* expect = R"(
[[builtin(sample_index)]] var<in> sample_index : u32;

[[builtin(sample_mask_in)]] var<in> mask_in : array<u32, 1>;

[[builtin(sample_mask_out)]] var<out> mask_out : array<u32, 1>;

[[stage(fragment)]]
fn main() {
  mask_out[0] = mask_in[0];
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleSampleMaskBuiltins_FunctionArg) {
  auto* src = R"(
[[builtin(sample_mask_in)]] var<in> mask_in : u32;

[[builtin(sample_mask_out)]] var<out> mask_out : u32;

fn filter(mask: u32) -> u32 {
  return (mask & 3u);
}

fn set_mask(input : u32) {
  mask_out = input;
}

[[stage(fragment)]]
fn main() {
  set_mask(filter(mask_in));
}
)";

  auto* expect = R"(
[[builtin(sample_mask_in)]] var<in> mask_in : array<u32, 1>;

[[builtin(sample_mask_out)]] var<out> mask_out : array<u32, 1>;

fn filter(mask : u32) -> u32 {
  return (mask & 3u);
}

fn set_mask(input : u32) {
  mask_out[0] = input;
}

[[stage(fragment)]]
fn main() {
  set_mask(filter(mask_in[0]));
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, AddEmptyEntryPoint) {
  auto* src = R"()";

  auto* expect = R"(
[[stage(compute)]]
fn _tint_unused_entry_point() {
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

// Test that different transforms within the sanitizer interact correctly.
TEST_F(SpirvTest, MultipleTransforms) {
  auto* src = R"(
[[stage(fragment)]]
fn main([[builtin(sample_index)]] sample_index : u32,
        [[builtin(sample_mask_in)]] mask_in : u32)
        -> [[builtin(sample_mask_out)]] u32 {
  return mask_in;
}
)";

  auto* expect = R"(
[[builtin(sample_index)]] var<in> tint_symbol : u32;

[[builtin(sample_mask_in)]] var<in> tint_symbol_1 : array<u32, 1>;

[[builtin(sample_mask_out)]] var<out> tint_symbol_3 : array<u32, 1>;

fn tint_symbol_4(tint_symbol_2 : u32) {
  tint_symbol_3[0] = tint_symbol_2;
}

[[stage(fragment)]]
fn main() {
  tint_symbol_4(tint_symbol_1[0]);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
