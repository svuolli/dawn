SKIP: FAILED

vk-gl-cts/graphicsfuzz/cov-apfloat-module-small-number/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<i32, 2>;
            ^^^^^^

vk-gl-cts/graphicsfuzz/cov-apfloat-module-small-number/0-opt.wgsl:7:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr_1 = @stride(16) array<f32, 1>;
              ^^^^^^

vk-gl-cts/graphicsfuzz/cov-apfloat-module-small-number/0-opt.wgsl:39:22 warning: use of deprecated builtin
  let x_48 : bool = (isInf(x_44) || (x_46 == 1.0));
                     ^^^^^

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  int el;
};

struct buf1 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
};

struct tint_padded_array_element_1 {
  float el;
};

struct buf0 {
  tint_padded_array_element_1 x_GLF_uniform_float_values[1];
};

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
} x_8;

void main_1() {
  float f0 = 0.0f;
  float s1 = 0.0f;
  float f1 = 0.0f;
  bool x_72 = false;
  bool x_73_phi = false;
  f0 = (10.0f % 0.000001f);
  s1 = 9.99999935e-39f;
  if ((s1 == 0.0f)) {
    s1 = 1.0f;
  }
  bool x_62 = false;
  bool x_71 = false;
  bool x_63_phi = false;
  bool x_72_phi = false;
  f1 = (10.0f % s1);
  bool tint_tmp = isinf(f1);
  if (!tint_tmp) {
    tint_tmp = (s1 == 1.0f);
  }
  bool x_48 = (tint_tmp);
  x_73_phi = x_48;
  if (!(x_48)) {
    bool x_54 = (f0 == f1);
    x_63_phi = x_54;
    if (!(x_54)) {
      bool tint_tmp_1 = (f0 > 0.99000001f);
      if (tint_tmp_1) {
        tint_tmp_1 = (f0 < 0.01f);
      }
      x_62 = (tint_tmp_1);
      x_63_phi = x_62;
    }
    bool x_63 = x_63_phi;
    x_72_phi = x_63;
    if (!(x_63)) {
      bool tint_tmp_2 = (f1 > 0.99000001f);
      if (tint_tmp_2) {
        tint_tmp_2 = (f1 < 0.01f);
      }
      x_71 = (tint_tmp_2);
      x_72_phi = x_71;
    }
    x_72 = x_72_phi;
    x_73_phi = x_72;
  }
  bool tint_tmp_3 = x_73_phi;
  if (!tint_tmp_3) {
    tint_tmp_3 = (f1 == 10.0f);
  }
  if ((tint_tmp_3)) {
    int x_81 = x_8.x_GLF_uniform_int_values[1].el;
    int x_84 = x_8.x_GLF_uniform_int_values[0].el;
    int x_87 = x_8.x_GLF_uniform_int_values[0].el;
    int x_90 = x_8.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(float(x_81), float(x_84), float(x_87), float(x_90));
  } else {
    int x_94 = x_8.x_GLF_uniform_int_values[0].el;
    float x_95 = float(x_94);
    x_GLF_color = vec4(x_95, x_95, x_95, x_95);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};

main_out tint_symbol() {
  main_1();
  main_out tint_symbol_1 = main_out(x_GLF_color);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol();
  x_GLF_color_1_1 = inner_result.x_GLF_color_1;
  return;
}
Error parsing GLSL shader:
ERROR: 0:32: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' const float' and a right operand of type ' const float' (or there is no acceptable conversion)
ERROR: 0:32: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



