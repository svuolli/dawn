struct S {
    a : i32;
    b : f32;
};

var<workgroup> v : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
    ignore(v);
}
