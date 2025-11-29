#!/bin/bash

# Test script for chunked transfer encoding

echo "=== Testing Chunked Transfer Encoding ==="
echo ""

# Test 1: Simple chunked request
echo "Test 1: Simple chunked POST request"
echo "-----------------------------------"

# Create a chunked request manually
# Format: <hex-size>\r\n<data>\r\n ... 0\r\n\r\n

printf "POST /upload HTTP/1.1\r\n\
Host: localhost:8080\r\n\
Transfer-Encoding: chunked\r\n\
Content-Type: text/plain\r\n\
\r\n\
4\r\n\
test\r\n\
5\r\n\
data!\r\n\
0\r\n\
\r\n" > /tmp/chunked_test1.txt

echo "Chunked request created:"
cat /tmp/chunked_test1.txt | od -c
echo ""

# Test 2: Multiple chunks
echo "Test 2: Multiple chunks with various sizes"
echo "-------------------------------------------"

printf "POST /upload HTTP/1.1\r\n\
Host: localhost:8080\r\n\
Transfer-Encoding: chunked\r\n\
\r\n\
1a\r\n\
field1=value1&field2=value2\r\n\
d\r\n\
&field3=val3\r\n\
0\r\n\
\r\n" > /tmp/chunked_test2.txt

echo "Multiple chunk request created"
echo "Expected un-chunked body: 'field1=value1&field2=value2&field3=val3'"
echo ""

# Test 3: Chunk with extensions (should be ignored)
echo "Test 3: Chunk with extensions"
echo "------------------------------"

printf "POST /test HTTP/1.1\r\n\
Host: localhost:8080\r\n\
Transfer-Encoding: chunked\r\n\
\r\n\
5;name=value\r\n\
hello\r\n\
6;ext=test\r\n\
 world\r\n\
0\r\n\
\r\n" > /tmp/chunked_test3.txt

echo "Chunk with extensions created"
echo "Expected un-chunked body: 'hello world'"
echo ""

echo "=== Test files created in /tmp/ ==="
echo "To test with the server:"
echo "1. Start server: ./webserv config/default.conf"
echo "2. Send test: cat /tmp/chunked_test1.txt | nc localhost 8080"
echo ""
echo "Or use curl with chunked encoding:"
echo "echo 'test data' | curl -X POST -H 'Transfer-Encoding: chunked' --data-binary @- http://localhost:8080/upload"
