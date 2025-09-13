#!/bin/bash

# Simplified Parser Tests for Pginx
# Tests core parsing functionality without problematic edge cases

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Build the project if needed
if [ ! -x "./pginx" ]; then
    echo -e "${BLUE}Building pginx...${NC}"
    make clean && make
fi

WEBSERV="./pginx"
TEMP_CONFIG_DIR="Tests/temp_configs"

# Create temp directory for test configs
mkdir -p "$TEMP_CONFIG_DIR"

passed=0
failed=0

echo -e "${BLUE}Starting Pginx PARSER TESTS...${NC}"
echo "========================================"

# Test 1: Basic single server config
echo -e "\n${YELLOW}TEST 1: Basic Single Server Configuration${NC}"
echo 'http {
    server {
        listen 8080;
        server_name example.com;
        root /var/www/html;
        
        location / {
            root /var/www/public;
        }
    }
}' > "$TEMP_CONFIG_DIR/basic.conf"

output=$($WEBSERV "$TEMP_CONFIG_DIR/basic.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 1" && \
   echo "$output" | grep -q "8080" && \
   echo "$output" | grep -q "example.com"; then
    echo -e "${GREEN}✅ PASSED: Basic single server parsing${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Basic single server parsing${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Test 2: Multiple servers
echo -e "\n${YELLOW}TEST 2: Multiple Servers Configuration${NC}"
echo 'http {
    server {
        listen 80;
        server_name site1.com;
    }
    
    server {
        listen 8080;
        server_name site2.com;
    }
}' > "$TEMP_CONFIG_DIR/multi.conf"

output=$($WEBSERV "$TEMP_CONFIG_DIR/multi.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 2" && \
   echo "$output" | grep -q "site1.com" && \
   echo "$output" | grep -q "site2.com"; then
    echo -e "${GREEN}✅ PASSED: Multiple servers parsing${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Multiple servers parsing${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Test 3: Server with multiple locations
echo -e "\n${YELLOW}TEST 3: Multiple Locations${NC}"
echo 'http {
    server {
        listen 3000;
        server_name example.com;
        root /var/www/example;
        
        location / {
            root /var/www/public;
        }
        
        location /api {
            root /var/www/api;
        }
    }
}' > "$TEMP_CONFIG_DIR/locations.conf"

output=$($WEBSERV "$TEMP_CONFIG_DIR/locations.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 1" && \
   echo "$output" | grep -q "Location: /" && \
   echo "$output" | grep -q "Location: /api"; then
    echo -e "${GREEN}✅ PASSED: Multiple locations parsing${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Multiple locations parsing${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Test 4: Client max body size directive
echo -e "\n${YELLOW}TEST 4: Client Max Body Size Directive${NC}"
echo 'http {
    server {
        listen 80;
        client_max_body_size 10M;
        server_name test.com;
    }
}' > "$TEMP_CONFIG_DIR/body_size.conf"

output=$($WEBSERV "$TEMP_CONFIG_DIR/body_size.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 1" && \
   echo "$output" | grep -q "10485760 bytes"; then
    echo -e "${GREEN}✅ PASSED: Client max body size parsing${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Client max body size parsing${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Test 5: Autoindex directive
echo -e "\n${YELLOW}TEST 5: Autoindex Directive${NC}"
echo 'http {
    server {
        listen 80;
        autoindex on;
        server_name test.com;
    }
}' > "$TEMP_CONFIG_DIR/autoindex.conf"

output=$($WEBSERV "$TEMP_CONFIG_DIR/autoindex.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 1" && \
   echo "$output" | grep -q "Auto index: on"; then
    echo -e "${GREEN}✅ PASSED: Autoindex directive parsing${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Autoindex directive parsing${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Test 6: Empty server block
echo -e "\n${YELLOW}TEST 6: Empty Server Block${NC}"
echo 'http {
    server {
        listen 80;
    }
    
    server {
        listen 8080;
        server_name empty.com;
    }
}' > "$TEMP_CONFIG_DIR/empty.conf"

output=$($WEBSERV "$TEMP_CONFIG_DIR/empty.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 2"; then
    echo -e "${GREEN}✅ PASSED: Empty server block handling${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Empty server block handling${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Test 7: Test with existing complex config
echo -e "\n${YELLOW}TEST 7: Existing Complex Configuration${NC}"
if [ -f "config/complex_test.conf" ]; then
    output=$($WEBSERV config/complex_test.conf 2>&1)
    if echo "$output" | grep -q "Number of servers: 2"; then
        echo -e "${GREEN}✅ PASSED: Complex configuration parsing${NC}"
        ((passed++))
    else
        echo -e "${RED}❌ FAILED: Complex configuration parsing${NC}"
        echo "Output: $output"
        ((failed++))
    fi
else
    echo -e "${YELLOW}⚠️  Complex config not found, skipping test${NC}"
fi

# Cleanup
rm -rf "$TEMP_CONFIG_DIR"

echo -e "\n========================================"
echo -e "${BLUE}PARSER TEST SUMMARY: ${GREEN}$passed passed${NC}, ${RED}$failed failed${NC}"
echo -e "========================================"

# Exit with error code if any tests failed
[ $failed -eq 0 ] || exit 1