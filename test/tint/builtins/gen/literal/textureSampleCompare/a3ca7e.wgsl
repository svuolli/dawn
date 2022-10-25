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

@group(1) @binding(0) var arg_0: texture_depth_cube_array;
@group(1) @binding(1) var arg_1: sampler_comparison;

// fn textureSampleCompare(texture: texture_depth_cube_array, sampler: sampler_comparison, coords: vec3<f32>, array_index: i32, depth_ref: f32) -> f32
fn textureSampleCompare_a3ca7e() {
  var res: f32 = textureSampleCompare(arg_0, arg_1, vec3<f32>(), 1i, 1.f);
}

@fragment
fn fragment_main() {
  textureSampleCompare_a3ca7e();
}
