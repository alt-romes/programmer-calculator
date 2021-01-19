todos:

- when doing math with numbers with specific number of bits, only unsigned numbers work

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

    - cleanup how lines are printed (should start )

    - now, when adding 2+ after having add 2 +, it is reset, instead of adding up ( and this is hard to fix ) - probably just say it's a feature

    - i can't do 10b because it detects "0b" so it expects to read binary instead of the number of bits

test:
    - make sure different number of bits still work with %lld and %llx
