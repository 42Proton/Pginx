#!/bin/bash

# Comprehensive CGI Test Script
# Tests chunked encoding, POST data, and EOF handling

echo "╔════════════════════════════════════════════════════════╗"
echo "║     CGI Implementation Test Suite                     ║"
echo "║     Testing Chunked Encoding & EOF Handling           ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

SERVER="localhost:8080"
PASS_COUNT=0
FAIL_COUNT=0

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "Prerequisites:"
echo "  1. Server must be running: ./webserv config/complex_test.conf"
echo "  2. Python3 must be installed"
echo "  3. CGI scripts must be executable"
echo ""

read -p "Press Enter to start tests..."
echo ""

# Test 1: Simple GET request
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 1: Simple GET Request"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
curl -s "http://${SERVER}/cgi-bin/test_get.py?name=John&age=30" > /tmp/cgi_test1.txt
if grep -q "Query String: name=John&age=30" /tmp/cgi_test1.txt; then
    echo -e "${GREEN}✓ PASS${NC} - GET request with query string"
    ((PASS_COUNT++))
else
    echo -e "${RED}✗ FAIL${NC} - GET request failed"
    ((FAIL_COUNT++))
fi
echo ""

# Test 2: Regular POST request (non-chunked)
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 2: Regular POST Request (Non-Chunked)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
curl -s -X POST -d "field1=value1&field2=value2" \
     http://${SERVER}/cgi-bin/test_post.py > /tmp/cgi_test2.txt
if grep -q "field1=value1&field2=value2" /tmp/cgi_test2.txt; then
    echo -e "${GREEN}✓ PASS${NC} - Regular POST request"
    ((PASS_COUNT++))
else
    echo -e "${RED}✗ FAIL${NC} - Regular POST failed"
    ((FAIL_COUNT++))
fi
echo ""

# Test 3: Chunked POST request (CRITICAL TEST)
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 3: Chunked POST Request (SUBJECT REQUIREMENT)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Testing: Server un-chunks request before passing to CGI"

# Create chunked request
cat > /tmp/chunked_request.txt << 'EOF'
POST /cgi-bin/test_chunked_post.py HTTP/1.1
Host: localhost:8080
Transfer-Encoding: chunked

1a
field1=value1&field2=value2
d
&field3=val3
0

EOF

# Convert to proper CRLF format
cat /tmp/chunked_request.txt | tr '\n' '\r' | sed 's/\r/\r\n/g' > /tmp/chunked_request_crlf.txt

echo "Sending chunked request via netcat..."
nc localhost 8080 < /tmp/chunked_request_crlf.txt > /tmp/cgi_test3.txt 2>&1 &
sleep 2

if grep -q "field1=value1&field2=value2&field3=val3" /tmp/cgi_test3.txt; then
    echo -e "${GREEN}✓ PASS${NC} - Chunked POST un-chunked correctly"
    ((PASS_COUNT++))
else
    echo -e "${RED}✗ FAIL${NC} - Chunked POST failed (check if server un-chunks)"
    echo "Expected: field1=value1&field2=value2&field3=val3"
    echo "Response saved to: /tmp/cgi_test3.txt"
    ((FAIL_COUNT++))
fi
echo ""

# Test 4: CGI without Content-Length (EOF handling)
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 4: CGI Output Without Content-Length (SUBJECT REQUIREMENT)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Testing: Server reads CGI output until EOF"

curl -s http://${SERVER}/cgi-bin/test_no_content_length.py > /tmp/cgi_test4.txt
if grep -q "Line 10:" /tmp/cgi_test4.txt && grep -q "EOF will mark the end" /tmp/cgi_test4.txt; then
    echo -e "${GREEN}✓ PASS${NC} - CGI output read until EOF"
    ((PASS_COUNT++))
else
    echo -e "${RED}✗ FAIL${NC} - CGI output incomplete (server may not be reading until EOF)"
    ((FAIL_COUNT++))
fi
echo ""

# Test 5: Shell script CGI
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 5: Shell Script CGI"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
curl -s "http://${SERVER}/cgi-bin/test_shell.sh?test=hello" > /tmp/cgi_test5.txt
if grep -q "Shell Script CGI Test" /tmp/cgi_test5.txt; then
    echo -e "${GREEN}✓ PASS${NC} - Shell script CGI execution"
    ((PASS_COUNT++))
else
    echo -e "${RED}✗ FAIL${NC} - Shell script CGI failed"
    ((FAIL_COUNT++))
fi
echo ""

# Summary
echo "╔════════════════════════════════════════════════════════╗"
echo "║                    Test Summary                        ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo -e "Total Tests:  $((PASS_COUNT + FAIL_COUNT))"
echo -e "${GREEN}Passed:       ${PASS_COUNT}${NC}"
echo -e "${RED}Failed:       ${FAIL_COUNT}${NC}"
echo ""

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║     ALL TESTS PASSED! ✓                                ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════╝${NC}"
    exit 0
else
    echo -e "${RED}╔════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║     SOME TESTS FAILED ✗                                ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Check test output files in /tmp/cgi_test*.txt"
    exit 1
fi
