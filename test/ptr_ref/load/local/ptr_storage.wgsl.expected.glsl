#version 310 es
precision mediump float;

struct S {
  int a;
};

layout(binding = 0, std430) buffer S_1 {
  int a;
} v;
void tint_symbol() {
  int use = (v.a + 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
