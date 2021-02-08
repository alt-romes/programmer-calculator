tests=( "number-bases" "random" "expressions" )
for t in "${tests[@]}"
do
    echo tests/$t
    diff -b tests/$t.correct <(cat tests/$t.test | ./pcalc -c)
done
