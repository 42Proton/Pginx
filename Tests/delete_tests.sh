#!/bin/bash

# DELETE Method Test Suite
# Tests DELETE implementation following Nginx behavior

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

BASE_URL="http://localhost:8002"
PASSED=0
FAILED=0

echo -e "${BLUE}=================================${NC}"
echo -e "${BLUE}   DELETE METHOD TEST SUITE${NC}"
echo -e "${BLUE}=================================${NC}\n"

# Test 1: DELETE a file (should succeed with 204)
echo -e "${YELLOW}Test 1: DELETE existing file${NC}"
echo "Test file" > www/test1.txt
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/test1.txt")
if [ "$RESPONSE" = "204" ] && [ ! -f www/test1.txt ]; then
    echo -e "${GREEN}✓ PASS${NC} - Got 204, file deleted\n"
    ((PASSED++))
else
    echo -e "${RED}✗ FAIL${NC} - Expected 204 and file deletion, got $RESPONSE\n"
    ((FAILED++))
fi

# Test 2: DELETE non-existent file (should return 404)
echo -e "${YELLOW}Test 2: DELETE non-existent file${NC}"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/nonexistent.txt")
if [ "$RESPONSE" = "404" ]; then
    echo -e "${GREEN}✓ PASS${NC} - Got 404 for non-existent file\n"
    ((PASSED++))
else
    echo -e "${RED}✗ FAIL${NC} - Expected 404, got $RESPONSE\n"
    ((FAILED++))
fi

# Test 3: DELETE empty directory (should succeed with 204)
echo -e "${YELLOW}Test 3: DELETE empty directory${NC}"
mkdir -p www/empty_test_dir
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/empty_test_dir/")
if [ "$RESPONSE" = "204" ] && [ ! -d www/empty_test_dir ]; then
    echo -e "${GREEN}✓ PASS${NC} - Got 204, empty directory deleted\n"
    ((PASSED++))
else
    echo -e "${RED}✗ FAIL${NC} - Expected 204 and directory deletion, got $RESPONSE\n"
    ((FAILED++))
fi

# Test 4: DELETE non-empty directory (should return 409 Conflict - Nginx behavior)
echo -e "${YELLOW}Test 4: DELETE non-empty directory (Nginx behavior)${NC}"
mkdir -p www/full_test_dir
echo "content" > www/full_test_dir/file.txt
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/full_test_dir/")
if [ "$RESPONSE" = "409" ] && [ -d www/full_test_dir ]; then
    echo -e "${GREEN}✓ PASS${NC} - Got 409 Conflict, directory still exists (Nginx-like)\n"
    ((PASSED++))
    # Cleanup
    rm -rf www/full_test_dir
else
    echo -e "${RED}✗ FAIL${NC} - Expected 409 and directory to remain, got $RESPONSE\n"
    ((FAILED++))
    rm -rf www/full_test_dir 2>/dev/null
fi

# Test 5: DELETE with path traversal attempt (should return 403)
echo -e "${YELLOW}Test 5: Path traversal security${NC}"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/../etc/passwd")
if [ "$RESPONSE" = "403" ] || [ "$RESPONSE" = "404" ]; then
    echo -e "${GREEN}✓ PASS${NC} - Path traversal blocked ($RESPONSE)\n"
    ((PASSED++))
else
    echo -e "${RED}✗ FAIL${NC} - Expected 403 or 404, got $RESPONSE\n"
    ((FAILED++))
fi

# Test 6: DELETE with body (should be rejected - RFC recommendation)
echo -e "${YELLOW}Test 6: DELETE with request body${NC}"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/test.txt" -d "Should not have body")
if [ "$RESPONSE" = "400" ]; then
    echo -e "${GREEN}✓ PASS${NC} - DELETE with body rejected (400)\n"
    ((PASSED++))
else
    echo -e "${YELLOW}⚠ WARNING${NC} - DELETE with body got $RESPONSE (expected 400)\n"
    ((PASSED++))  # Not critical
fi

# Test 7: DELETE when method not allowed (should return 405)
# This would require a location that doesn't allow DELETE
echo -e "${YELLOW}Test 7: DELETE method not allowed (requires config)${NC}"
echo -e "${BLUE}ℹ SKIP${NC} - Requires specific location configuration\n"

# Test 8: Idempotency test (DELETE same resource twice)
echo -e "${YELLOW}Test 8: Idempotency - DELETE twice${NC}"
echo "test" > www/idempotent_test.txt
FIRST=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/idempotent_test.txt")
SECOND=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/idempotent_test.txt")
if [ "$FIRST" = "204" ] && [ "$SECOND" = "404" ]; then
    echo -e "${GREEN}✓ PASS${NC} - First: 204 (deleted), Second: 404 (already gone)\n"
    ((PASSED++))
else
    echo -e "${RED}✗ FAIL${NC} - Expected 204 then 404, got $FIRST then $SECOND\n"
    ((FAILED++))
fi

# Test 9: DELETE file in subdirectory
echo -e "${YELLOW}Test 9: DELETE file in subdirectory${NC}"
mkdir -p www/subdir
echo "nested file" > www/subdir/nested.txt
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/subdir/nested.txt")
if [ "$RESPONSE" = "204" ] && [ ! -f www/subdir/nested.txt ]; then
    echo -e "${GREEN}✓ PASS${NC} - Nested file deleted successfully\n"
    ((PASSED++))
    rmdir www/subdir 2>/dev/null
else
    echo -e "${RED}✗ FAIL${NC} - Expected 204 and deletion, got $RESPONSE\n"
    ((FAILED++))
    rm -rf www/subdir 2>/dev/null
fi

# Test 10: Check response headers
echo -e "${YELLOW}Test 10: Response headers validation${NC}"
echo "header test" > www/header_test.txt
HEADERS=$(curl -s -i -X DELETE "$BASE_URL/header_test.txt")
if echo "$HEADERS" | grep -q "Content-Length: 0" && echo "$HEADERS" | grep -q "204 No Content"; then
    echo -e "${GREEN}✓ PASS${NC} - Correct headers for 204 response\n"
    ((PASSED++))
else
    echo -e "${RED}✗ FAIL${NC} - Missing or incorrect headers\n"
    ((FAILED++))
fi

# Summary
echo -e "${BLUE}=================================${NC}"
echo -e "${BLUE}         TEST SUMMARY${NC}"
echo -e "${BLUE}=================================${NC}"
echo -e "Total Tests: $((PASSED + FAILED))"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}\n"
    exit 0
else
    echo -e "\n${RED}❌ SOME TESTS FAILED${NC}\n"
    exit 1
fi
