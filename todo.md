todos:

important:
    - write features in webpage and display product
    - keep track of all features

features:

    - compare two numbers ? (perhaps change to 16 bits when comparing) (show both numbers side to side)

    - up arrow scrolls history (like terminals) - romes

    - change number of bits used

fix:

    - history is broken when 2*2 is inserted

    - if there's an operation in course, two enters should reset the calculator

    - possibly change "n, t" operations to something else (it breaks words with them)

    - atualizar lista de operações

    - history only displays what was accepted, prettify. (i.e. askdfj = 10, help = help, + = +, 20*20 = 20 * 20, 20 *20 = 20 * 20, *20 = * 20 )

    no priority: 2+ when stack isn't empty should restart history & stack

    - cleanup how lines are printed (should start )

    - spamming number + number in 1 line multiple times eventually leads to seg fault.
test:
    - make sure different number of bits still work with %lld and %llx
