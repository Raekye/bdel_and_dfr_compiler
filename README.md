# Bdel and dfr compiler
Compiler for a basic language to run on a custom soft processor on an FPGA.
This is a fun project.
A compiler makes it easier to write programs for our processor.
The language is Turing-complete, and with a "standard library", supports dynamic memory, data structures, and a console-like interface for reading/writing to a VGA monitor.
My favourite example is `mergesort` in `stdlib.txt`.

## Processor
Processor source is hosted at [github.com/Lrac/bdel_and_dfr_processor][2]

## Dependencies
- g++
- flex
- bison

## Interesting
- when out of registers, compiler chooses register to kick out by least recently used
- if branches, while loop, have own scope (including `if (var x = something()) { another(x); ... } ...`)

## Usage
- compile compiler: `make`, output is `bin/pleb`
- compile stuff: `./bin/pleb [file]` or `cat a.txt b.txt c.txt | ./bin/pleb`
- assemble stuff (input from stdin, or give files to perl script): `./assembler.pl`
- example: `./bin/pleb [in-file] | ./assembler.pl > [out-file]`

## Assembly
- 16 registers (0 to 15), referred to by "r" plus the index
- the stack pointer points to the end of the stack
- labels with `something:`, refer to labels with `:something`

### Opcodes
- `load <literal, stack offset> <register>`: read from stack (e.g. `load 1 r0` loads the value at the top of the stack into register 0)
- `store <register> <literal, stack offset>`: write to stack (e.g. `store 2 r0` writes the value in register 1 to the second-from-top location on the stack)
- `literal <literal> <register>`: put a literal value into a register
- `input <register>`: read input from FPGA switches ("enter" with FPGA key/button)
- `output <register>`: output to FPGA LED display
- `add|sub|div|mul|mod|and|or|eq|lt|gt <register a> <register b> <register c>`: binary operation with registers a and b, put result in c
- `branch <register>`: execute the next instruction if register is non-zero, execute the 2nd instruction if the register is zero
- `jump :<label>`: jump to label
- `jump <register>`: jump to address in register
- `stack <literal>`: increase or decrease the stack pointer
- `supermandive`: save all registers on stack (helpful for hand writing functions and function calls)
- `getup`: restore all registers from stack
- `heap <register a> <register b>`: put value in register a into heap address in register b
- `unheap <register a> <register b>`: read value from heap address in register a into register b
- `inc|dec <register>`: increment or decrement register in place
- `mov <register a> <register b>`: move value in register a into register b
- `not <register a> <register b>`: logical not value in register a into register b
- `rand <register a>`: put random value into register a
- `interrupt <literal, interrupt code> <register>`: register interrupt handler, address of interrupt handler in register
- `uninterrupt`: used at the end of an interrupt handler
- `draw|printdec|printhex <register a> <register b> <register c>`: draw value in register a to VGA, x in register b, y in register c
- `keyboard|keydec|keyhex <register a> <register b> <register c>`: read from keyboard into register c, display what user typed at x in register a, y in register b (blocking)
- `unkey <register>`: put last pressed keycode into register (for keyboard interrupt handler)`
- `heapreset`: zero the heap
- `die`: so it goes

## Language
- only data type is a 16-bit signed integer
- 0 is false, true otherwise
- functions and function declarations only at the top level
- everything else is an expression (including if-statements, while-loops, and assembly blocks)
- the last expression in a block is the value of the block (last expression in a function is the return value)
- embedding assembly (`asm`) writes all variables to the stack. The value of an assembly block is whatever is in register 13
- strings can only contain spaces, lowercase letters, and numbers (digits)

```
/* Variables */
var x = 3;
x = x * 4;

/* Function declaration */
def foo(var x);

/* Function declaration and definition */
def bar(var x, var y, var z) {
	x + foo(y) + 2 * z;
}

/* If statements */
if (x == 0) {
	bar(3);
} else if (x > 10 && x < 20) {
	echo "hmmm";
} else {
	echo "something else";
}

/* While loops */
while (x > 0) {
	x = x - 1;
}

/* Print literal string */
echo "abc 123";

/* Dynamically allocated string */
var str = "hmmm";
io_printstring(str);

/* Embedded assembly */
var time = 0;
def timer_interrupt_handler() {
	time = time + 1;
	asm {
		#uninterrupt;
	};
};
var loc = :timer_interrupt_handler;
asm {
	#load 1 r0;
	#interrupt 0 r0;
};
```

### Code examples
- `fib.txt`
- `mergesort.txt`
- `snake.txt`
- `stdlib.txt`
	- functions for drawing/printing to the screen as if it were a console (basic) (e.g. it keeps track of where to put the next character)
	- vector (dynamically resizing array) data structure
	- linear `malloc(n)`, `free(ptr)`, and `heap_get(ptr)` and `heap_set(ptr, x)`

### OSMaker
Combines multiple programs into a wrapper program with a menu to run them (and return to main screen when finished)

## Todo
- return statement
- optimize don't allocate stack if not needed, don't increment stack if all in registers
- tail call optimization?
- bison line numbers
- strings "-" for unused, check over (3 places)
- need to track for when generating initialization
- inline
- helper for modifying variables in registers/registers with variables
- break stack
- don't need register tmp?
- unneeded commit globals?
- scope should know which variables in registers
- string library functions

## More hmmmm
- [Sufficient conditions for Turing-completeness][1]

[1]: http://cs.stackexchange.com/questions/991/are-there-minimum-criteria-for-a-programming-language-being-turing-complete
[2]: https://github.com/Lrac/bdel_and_dfr_processor
