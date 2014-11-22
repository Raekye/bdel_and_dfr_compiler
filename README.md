# Hmmmm
Hmmm

# Dependencies
- flex
- bison

# Usage
- compile compiler: `make`, output is `bin/pleb`
- `bin/pleb` and `assembler.pl` expect input from stdin
- compile stuff: `cat [file] | ./bin/pleb`
- compile and assemble stuff: `cat [in-file] | ./bin/pleb | ./assembler.pl > [out-file]`

## Todo
- local scope
- return statement
- embed assembly
- let compiler choose global heap address for data
- print strings
- input
- console stdlib
