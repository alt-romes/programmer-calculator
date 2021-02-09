## Testing

The folder `tests` has two files for each test. `file.test` and `file.correct`

The `.test` file is the input passed into `pcalc -c`, and the `.correct` file is the expected output

To test one of these files run:
```
$ diff -b tests/number-bases.correct <(cat tests/number-bases.test | ./pcalc -c)
```

If something is printed out to the console then the actual output and the expected output differ, and changes should be made until all tests pass.

To test all files at the same time run:
```
$ ./run-tests.sh
```

It would be better to run `./run-tests.sh` multiple times because i.e. I currently have a bug that crashes the code with segfault only half of the times

### Test file

A test file contains sequences of expressions to be run in the calculator and must end with "quit" or "exit"

When creating a new test file, add the name to the array of tests in `run-tests.sh`

### Writing a test

I've found that the best way to write a test is by testing multiple operations in the calculator while checking it's result, and save the operations done. If everything you observe is correct, you run the test and save the output as the correction.

First: Test input while saving it
```
$ tee -a tests/name-of-test.test | ./pcalc -c
```

If all results from the input inserted are correct, save the output as the correction
```
$ cat tests/name-of-test.test | ./pcalc -c > name-of-test.correct
```
