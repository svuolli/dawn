SKIP: FAILED

#version 310 es

layout(rg32ui) uniform highp writeonly uimage2DArray arg_0;
void textureDimensions_2674d8() {
  uvec2 res = uvec2(imageSize(arg_0).xy);
}

vec4 vertex_main() {
  textureDimensions_2674d8();
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
Error parsing GLSL shader:
ERROR: 0:3: 'image load-store format' : not supported with this profile: es
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

layout(rg32ui) uniform highp writeonly uimage2DArray arg_0;
void textureDimensions_2674d8() {
  uvec2 res = uvec2(imageSize(arg_0).xy);
}

void fragment_main() {
  textureDimensions_2674d8();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'image load-store format' : not supported with this profile: es
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(rg32ui) uniform highp writeonly uimage2DArray arg_0;
void textureDimensions_2674d8() {
  uvec2 res = uvec2(imageSize(arg_0).xy);
}

void compute_main() {
  textureDimensions_2674d8();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:3: 'image load-store format' : not supported with this profile: es
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.


