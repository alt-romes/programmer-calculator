todos:

compare two numbers ? (perhaps change to 16 bits when comparing)

up arrow scrolls history (like terminals)

change number of bits used

fix: long long + long long is being done with integers ( c integer arithmetic ), so 0xffffffff + 1 with 64 bits should be a higher number, and shouldn't become 0 as it is doing now.
