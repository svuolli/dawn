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

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/src/cmd/gen
// using the template:
//   test/tint/builtins/gen/gen.wgsl.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////


// fn insertBits(vec<2, i32>, vec<2, i32>, u32, u32) -> vec<2, i32>
fn insertBits_fe6ba6() {
  var arg_0 = vec2<i32>(1i);
  var arg_1 = vec2<i32>(1i);
  var arg_2 = 1u;
  var arg_3 = 1u;
  var res: vec2<i32> = insertBits(arg_0, arg_1, arg_2, arg_3);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_fe6ba6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  insertBits_fe6ba6();
}

@compute @workgroup_size(1)
fn compute_main() {
  insertBits_fe6ba6();
}
