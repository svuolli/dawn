SKIP: FAILED

static uint var_1 = 0u;

void main_1() {
  var_1 = 0u;
  [loop] while (true) {
    var_1 = 1u;
    var_1 = 2u;
    if (false) {
      break;
    }
    var_1 = 3u;
    {
      var_1 = 4u;
    }
  }
  var_1 = 5u;
  return;
}

void main() {
  main_1();
  return;
}
warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
/tmp/tint_T39Xh2:20: error: Loop must have break.
Validation failed.


