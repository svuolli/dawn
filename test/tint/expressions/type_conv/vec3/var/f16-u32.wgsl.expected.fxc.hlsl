SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static vector<float16_t, 3> u = (float16_t(1.0h)).xxx;

void f() {
  const uint3 v = uint3(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000001D88B216170(6,15-23): error X3000: syntax error: unexpected token 'float16_t'
