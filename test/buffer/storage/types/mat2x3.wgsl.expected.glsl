#version 310 es
precision mediump float;

struct tint_symbol_block {
  mat2x3 inner;
};

layout(binding = 0, std430) buffer tint_symbol_block_1 {
  mat2x3 inner;
} tint_symbol;
layout(binding = 1, std430) buffer tint_symbol_block_2 {
  mat2x3 inner;
} tint_symbol_1;
void tint_symbol_2() {
  tint_symbol_1.inner = tint_symbol.inner;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_2();
  return;
}
