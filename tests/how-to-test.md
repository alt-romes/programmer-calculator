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

### Test file

A test file contains sequences of expressions to be run in the calculator and must end with "quit" or "exit"

When creating a new test file, add the name to the array of tests in `run-tests.sh`
