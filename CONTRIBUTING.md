# How to contribute

Hi! I'm happy you're interested in contributing. This project is still growing so we're trying to both better the current code, and adding interesting features.

If you have any question or want to clarify something, write it out in the Discussions tab. We don't have a live chat channel but if proven needed we might make one.


## Submitting changes

All changes must be submitted through pull requests.

Before creating a pull request don't forget that:

  * The entire project should be recompiled with `make clean && make`
  * Changes must be document so that reviewers and future contributors can understand the code
  * All existing tests should be run
  * The PR should have a simple and descriptive title

## Coding style

  * Indent using four spaces (soft tabs)
  * Please use standard snake_case variable names and functions in newly introduced code.
  * Functions, loops and if have a space between name and bracket (i.e. `while (i < 10) {`, `int func (int i) {`)
  * The curly bracket start on the same line as the declaration (i.e. `if (a == b) {*`)
  * When writing `else if`s, and `else`s, please write them one line after the closing `}`:
  ```
  if (...) {
    // ...
  }
  else {
  ...
  }
  ```
  * A space always comes after a comma (`int func (int x, int y, int z)`, not `int func (int x,int y,int z)`)
  * Between a variable and an operator is a space (i.e. `int i = 1;`, `i += 1;`)
  * When defining a pointer the `*` should stay close to the type (i.e. `int* i`, `char* str`), and when dereferencing the pointer the `*` should stay close to the variable name (i.e. `*i = 20`, `*str = 'a'`)
  * Consider the people who will read your code, and make it look nice for them :)


## Testing

For information on testing please see [Testing](https://github.com/alt-romes/programmer-calculator/blob/master/tests/how-to-test.md)


Thank you, ~romes
