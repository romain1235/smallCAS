# smallCAS

smallCAS is a computer algebra system designed to work with a very little amount of RAM

## Features

- simple operations: `+`, `-`, `*`, `/`, `^`
- integers and rationals with values up to 256^127-1
- symbolic calculation (e.g. 5x+5x->10x)
- parse and serialize implicit multiplication
- precise syntax error and error position in the input when parsing

## How to use

### add the sources in your makefile
include smallCAS's makefile, add `$(SCAS_SRCS)` to your sources and `$(SCAS_CXXFLAGS)` to your flags
### create a TreeStack
smallCAS needs a stack to store trees, to create a TreeStack, include [tree_stack.hpp](tree_stack.hpp) then do 
```cpp
TreeStack stack(buffer, bufferSize);
```
where buffer is an external uint8_t buffer and bufferSize is its size.
### set a checkpoint
before using any smallCAS functions, you need to set a checkpoint to handle errors.
to do that, you need to include [checkpoint.hpp](checkpoint.hpp) then create your checkpoint with your previously created TreeStack like that:
```cpp
Checkpoint checkpoint(&stack);
if (setjmp(checkpoint.env)==0){
    // if you just set the checkpoint
}else{
    // if there was an error
}
```
then, to access the error type, do
```cpp
Error error = Checkpoint::error;
```
### Trees
a Tree is a math expression that you can work with.
to create a Tree, there are two ways:
- parsing: include [parser.hpp](parser.hpp) and use `parse(TreeStack * stack, const char * text)` that returns a struct with:
  - Tree * result: the parsed Tree (nullptr if syntax error)
  - SyntaxError error: the type of syntax error (e.g. missing parenthesis, incomplete expression, ...)
  - const char * errorPos: the position of the error in your input buffer
- k_trees: functions to make constexpr trees defined in [k_tree.hpp](k_tree.hpp)

then there are multiple functions you can use on your Tree:
- `tree->project(TreeStack * stack)`: return a new version of the tree usable for simplification.
- `tree->simpleReduction(TreeStack * stack)`: return a simplified copy of the tree. It flattens sums and products, drops 0 and 1, sorts and combines like terms, and evaluates powers of numbers.
- `tree->beautify(TreeStack * stack)`: return a serializable tree.
- `tree->serialize(char *out, uint16_t size)`: write the serialized tree in out, and size is it's size, if the out buffer is too small, the tree will be cut
### marks
marks are useful to delete Trees in the stack, to create one, do:
```cpp
uint32_t mark = stack.size;
```
then you can use this mark in two ways:
- `stack.trimTo(uint32_t mark)`: delete everything after the mark
- `stack.keepOnly(uint32_t mark, Tree * result)`: return the same tree as result, but at the position of mark, and delete everything after the mark except result

