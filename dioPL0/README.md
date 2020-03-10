# dioPL0, A Compliler of Pl/0

Implementation of 「コンパイラ: 作りながら学ぶ」("Compiler: learn by making")

This language is a PL/0-like language, dioPL.

- These codes are not from scratch
- I used the references below

## Dependency

lang: C
compiler: Apple LLVM version 10.0.1 (clang-1001.0.46.4)

## Setup

### How to install

You can install this repository with the following code.

```bash
git clone https://github.com/diohabara/dioPL.git
```

### Build

Type this code.

```bash
make
```

You get the file `diopl0`.

This is the compiler of dioPL

```bash
./diopl0
```

After typing this commad, you get the following code, `enter source file name`.

You will type down the object file such "hogehoge.pl0" or "foo.pl0".

```bash
foo.pl0
```

This will show output of the file on your terminal.

Moreover, you get an .html file after the compiling, which tells you the errors of your code.

### Remove

If you would like to delete the .html, .o and diopl0 files, you just type this code below.

```bash
make clean
```

## Language feature

### Arithmetic Operations

You can add, subtract, multiply, divide numbers.

The spaces between the characters are unnecessary.

You can choose whether to write them or not. It does not affect the result of arithmetics.

The syntax is an influx one.

```pl0
write(10^0); # 1
write(1+1); # 2
write(5-2); # 3
write(1*4); # 4
write(10/2); # 5
write(20%7); # 6
write(7^2); # 49
```

### Conditional

Logical formula can be computed.

- `=` sees if the two operands are equal
- `<` sees if the former operand is smaller than the later one
- `>` sees if the former operand is bigger than the later one
- `<>` sees if the two operands are **NOT** equal
- `<=` sees if the former operand is **NOT** bigger than the later one
- `>=` sees if the two operands is **NOT** smaller than the later one

```pl0
a = b # a equal b
a < b # a less than b
a > b # a greater than b
a <> b # a not equal to b
a <= b # a less than or equal to b
a >= b # a graeter than or equal to b
```

You can write `if statement` and `while statement` by using this conditionals

This compiler does not have `else statement`, but later I may implement it.

Below is example of them.

```pl0
if 0 < 1 then write(1000); # 1000
```

```pl0
# the explanation of variables is in the next section
var cnt;
begin
    cnt := 0;
    while 0 < 5 do
        begin
            cnt := cnt+1;
            write(cnt); # 1, 2, 3, 4, 5
            writeln;
        end
end.
```

### Variables

Put `var` before the variables.

You can declare multiple variables, puctuating with `,`.

```pl0
var a, b, c; # the declaration of variables, a, b and c
```

### Assignment

`:=` is for assigning value into variables.

```pl0
var a;
begin
    a := 1 ;
end.
```

### Statement

All statements are enclosed by `begin` and `end`.

The final `end` must be attached with `.`, and other must be with `;` except when you write `while statement`.

### Output

Using the `write()` procedure, you can output the number in the parantheses.

Using the `writeln` procedure, you make a new line.

### Functions

`function` defines a function.

In functions, the arguments cannot be used as variables. You have to define other variables to use the value of arguments.

Here is an example of how to write a function.

```pl0
function add(x, y)
    var a, b;
begin a := x, b := y;
    return a + b
end;
```

### Let's use

You can test examples in the test directory.

## Licence

This software is released under the MIT License, see LICENSE.

## Author

diohabara

## References

https://www.ohmsha.co.jp/book/9784274221163/

https://github.com/rui314/8cc/blob/master/Makefile
