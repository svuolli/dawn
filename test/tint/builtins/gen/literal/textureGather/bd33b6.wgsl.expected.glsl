#version 310 es

uniform highp isampler2DArray arg_1_arg_2;

void textureGather_bd33b6() {
  ivec4 res = textureGatherOffset(arg_1_arg_2, vec3(0.0f, 0.0f, float(1u)), ivec2(0), int(1u));
}

vec4 vertex_main() {
  textureGather_bd33b6();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

uniform highp isampler2DArray arg_1_arg_2;

void textureGather_bd33b6() {
  ivec4 res = textureGatherOffset(arg_1_arg_2, vec3(0.0f, 0.0f, float(1u)), ivec2(0), int(1u));
}

void fragment_main() {
  textureGather_bd33b6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp isampler2DArray arg_1_arg_2;

void textureGather_bd33b6() {
  ivec4 res = textureGatherOffset(arg_1_arg_2, vec3(0.0f, 0.0f, float(1u)), ivec2(0), int(1u));
}

void compute_main() {
  textureGather_bd33b6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}