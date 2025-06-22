#!/bin/bash

if [ ! -x "../WebServ" ]; then
    make -C ..
fi

WEBSERV="../Webserv"


TEST_CASES=(
    "Default Config:config/default.conf:0"
    "No arguements::0"
    "Invalid Extension:config/invlaid:1"
    "Missing file:config/none.conf:1"
    "Multiple Arguements:config/default.conf LOLL:1"
)

passed=0;
failed=0;

echo "Starting WebServ Initialization TESTS..."

for test in "${TEST_CASES[@]}"; do
    desc=$(echo "$test" | cut -d':' -f1)
    args=$(echo "$test" | cut -d':' -f2)
    code=$(echo "$test" | cut -d':' -f3)

    echo "-----------------------------"
    echo "TEST: $desc"
    echo "Args: '$args'"
    echo "Expected exit code: $code"


    $WEBSERV $args > /dev/null 2>&1
    actual_code=$?
    
    echo "Actual exit code: $actual_code"

    if [ "$actual_code" -eq "$code" ]; then
        if [ "$code" -eq 0 ]; then
            echo "✅ Test passed (program succeeded as expected)"
        else
            echo "✅ Test passed (program failed as expected)"
        fi
        ((passed++))
    else
        if [ "$code" -eq 0 ]; then
            echo "❌ Test failed (program should have succeeded)"
        else
            echo "❌ Test failed (program should have failed with exit code $code)"
        fi
        ((failed++))
    fi
done

echo "----------------------------"
echo "TEST SUMMARY: $passed passed, $failed failed"
make -C .. fclean

[ $failed -eq 0 ] || exit 1