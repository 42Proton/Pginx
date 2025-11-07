#!/bin/bash

# POST Request Tests for webserv
# Make sure your server is running before executing this script

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

BASE_URL="http://localhost:3000"
UPLOAD_PATH="/upload"

echo -e "${YELLOW}=== POST Request Tests ===${NC}\n"

# Test 1: Simple POST with text data
echo -e "${YELLOW}Test 1: Simple POST with text data${NC}"
curl -X POST "$BASE_URL$UPLOAD_PATH" \
  -H "Content-Type: text/plain" \
  -d "Hello, World!" \
  -w "\nHTTP Status: %{http_code}\n" \
  -s
echo -e "\n---\n"

# Test 2: POST with JSON data
echo -e "${YELLOW}Test 2: POST with JSON data${NC}"
curl -X POST "$BASE_URL$UPLOAD_PATH" \
  -H "Content-Type: application/json" \
  -d '{"message":"test","timestamp":"2025-11-06"}' \
  -w "\nHTTP Status: %{http_code}\n" \
  -s
echo -e "\n---\n"

# Test 3: POST with form data
echo -e "${YELLOW}Test 3: POST with form data${NC}"
curl -X POST "$BASE_URL$UPLOAD_PATH" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "username=testuser&password=testpass&email=test@example.com" \
  -w "\nHTTP Status: %{http_code}\n" \
  -s
echo -e "\n---\n"

# Test 4: POST with empty body (should fail validation)
echo -e "${YELLOW}Test 4: POST with empty body (should return 400)${NC}"
curl -X POST "$BASE_URL$UPLOAD_PATH" \
  -H "Content-Length: 0" \
  -w "\nHTTP Status: %{http_code}\n" \
  -s
echo -e "\n---\n"

# Test 5: POST with large body (test client_max_body_size)
echo -e "${YELLOW}Test 5: POST with large body${NC}"
dd if=/dev/zero bs=1024 count=512 2>/dev/null | curl -X POST "$BASE_URL$UPLOAD_PATH" \
  -H "Content-Type: application/octet-stream" \
  --data-binary @- \
  -w "\nHTTP Status: %{http_code}\n" \
  -s
echo -e "\n---\n"

# Test 6: POST to create a file
echo -e "${YELLOW}Test 6: POST to create a file with specific name${NC}"
echo "Test file content $(date)" > /tmp/test_upload.txt
curl -X POST "$BASE_URL/upload/testfile.txt" \
  -H "Content-Type: text/plain" \
  --data-binary @/tmp/test_upload.txt \
  -w "\nHTTP Status: %{http_code}\n" \
  -s
echo -e "\n---\n"

# Test 7: Verbose POST to see all headers
echo -e "${YELLOW}Test 7: Verbose POST (showing request/response headers)${NC}"
curl -v -X POST "$BASE_URL$UPLOAD_PATH" \
  -H "Content-Type: text/plain" \
  -d "Verbose test data" \
  2>&1
echo -e "\n---\n"

# Test 8: POST with chunked transfer encoding
echo -e "${YELLOW}Test 8: POST with chunked transfer encoding${NC}"
echo "Chunked data" | curl -X POST "$BASE_URL$UPLOAD_PATH" \
  -H "Transfer-Encoding: chunked" \
  --data-binary @- \
  -w "\nHTTP Status: %{http_code}\n" \
  -s
echo -e "\n---\n"

echo -e "${GREEN}=== All POST tests completed ===${NC}"
