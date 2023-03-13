tests=( "number-bases" "random" "expressions" "input-formats" "corner-cases" )
for t in "${tests[@]}"
do
    diff -b tests/$t.correct <(cat tests/$t.test | bin/pcalc -n) ||
        if echo "Test failed:"; then
            echo tests/$t
            exit 1
        fi
done
echo "All tests passed"
