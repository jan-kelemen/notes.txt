# The Rust Programming Language

https://doc.rust-lang.org/stable/book/

2020-12-22
----------

Version of rustc currently used: 1.48.0

References are annotated with '&' on the call site as well.

2020-12-23
----------

Return types on the function definitions aren't optional.

Assignment involving a String does a move not copy.
If copying is desired the type should implement a Copy trait.

Struct update syntax with structs containing noncopyable types still
performes a move on the struct members. This is not possible:
#[derive(Debug)]
struct Rectangle {
    width: u32,
    height: u32,
    description: String
}

fn main() {
    let rect1 = Rectangle {
        width: 30,
        height: 50,
        description: String::from("foobar"),
    };
    
    let rect2 = Rectangle {
        width: 40,
        ..rect1
    };

    println!("The area of rectangle is {:?} square pixels.", rect1);
}

