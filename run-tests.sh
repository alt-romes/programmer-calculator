tests=( "number-bases" "random" "expressions" "input-formats" )
for t in "${tests[@]}"
do
    diff -b tests/$t.correct <(cat tests/$t.test | ./pcalc -n) ||
        if echo "Test failed:"; then
            echo tests/$t
            exit
        fi
done
echo "All tests passed"
