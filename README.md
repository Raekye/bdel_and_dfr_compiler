# Hmmmm
Hmmm

# Dependencies
- flex
- bison

# Interesting
- when out of registers, compiler chooses register to kick out by least recently used

# Usage
- compile compiler: `make`, output is `bin/pleb`
- `bin/pleb` and `assembler.pl`
- compile stuff: `./bin/pleb [file]`
- compile and assemble stuff: `./bin/pleb [in-file] | ./assembler.pl > [out-file]`

## Todo
- local scope
- return statement
- embed assembly
- let compiler choose global heap address for data
- print strings
- input
- console stdlib
- optimize don't allocate stack if not needed
- while loops
- tail call optimization?
- bison line numbers
- don't increment stack if all in registers
- strings "-" for unused, check over (3 places)

## More hmmmm
- [Sufficient conditions for Turing-completeness][1]

[1]: http://cs.stackexchange.com/questions/991/are-there-minimum-criteria-for-a-programming-language-being-turing-complete
