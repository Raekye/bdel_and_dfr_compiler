# Hmmmm
Hmmm

# Dependencies
- g++
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
- return statement
- input
- console stdlib
- optimize don't allocate stack if not needed
- tail call optimization?
- bison line numbers
- don't increment stack if all in registers
- strings "-" for unused, check over (3 places)
- keep globals in registers
- need to track for when generating initialization
- inline
- helper for modifying variables in registers/registers with variables

## More hmmmm
- [Sufficient conditions for Turing-completeness][1]

[1]: http://cs.stackexchange.com/questions/991/are-there-minimum-criteria-for-a-programming-language-being-turing-complete
