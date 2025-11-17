#!/bin/bash

# Parser Tests for Pginx
# Tests the parsing functionality and output validation

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
TEST_DIR="Tests"
TEMP_CONFIG_DIR="$TEST_DIR/temp_configs"

# Create temp directory for test configs
mkdir -p "$TEMP_CONFIG_DIR"

passed=0
failed=0

echo -e "${BLUE}Starting Pginx PARSER TESTS...${NC}"
echo "========================================"

# Test 1: Basic single server config
echo -e "\n${YELLOW}TEST 1: Basic Single Server Configuration${NC}"
cat > "$TEMP_CONFIG_DIR/basic.conf" << 'EOF'
http {
    server {
        listen 8080;
        server_name example.com;
        root /var/www/html;
        
        location / {
            root /var/www/public;
        }
    }
}
EOF

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
cat > "$TEMP_CONFIG_DIR/multi.conf" << 'EOF'
http {
    server {
        listen 80;
        server_name site1.com;
    }
    
    server {
        listen 8080;
        server_name site2.com;
    }
}
EOF

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

# Test 3: Complex configuration with all directives
echo -e "\n${YELLOW}TEST 3: Complex Configuration with All Directives${NC}"
cat > "$TEMP_CONFIG_DIR/complex.conf" << 'EOF'
http {
    server {
        listen 3000;
        server_name example.com www.example.com;
        root /var/www/example;
        index index.html index.htm;
        client_max_body_size 10M;
        autoindex on;
        
        error_page 404 /custom_404.html;
        error_page 500 502 503 504 /50x.html;
        
        location / {
            root /var/www/example/public;
            index index.html;
        }
        
        location /api {
            root /var/www/example/api;
            autoindex off;
        }
    }
    
    server {
        listen 8080;
        server_name api.example.com;
        root /var/www/api;
        
        location /v1 {
            root /var/www/api/v1;
        }
    }
}
EOF

output=$($WEBSERV "$TEMP_CONFIG_DIR/complex.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 2" && \
   echo "$output" | grep -q "3000" && \
   echo "$output" | grep -q "8080" && \
   echo "$output" | grep -q "example.com" && \
   echo "$output" | grep -q "api.example.com" && \
   echo "$output" | grep -q "10485760 bytes" && \
   echo "$output" | grep -q "Auto index: on"; then
    echo -e "${GREEN}✅ PASSED: Complex configuration parsing${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Complex configuration parsing${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Test 4: Nested locations
echo -e "\n${YELLOW}TEST 4: Nested Locations${NC}"
cat > "$TEMP_CONFIG_DIR/nested.conf" << 'EOF'
http {
    server {
        listen 80;
        
        location /api {
            root /var/www/api;
            
            location /api/auth {
                root /var/www/auth;
            }
        }
    }
}
EOF

output=$($WEBSERV "$TEMP_CONFIG_DIR/nested.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 1" && \
   echo "$output" | grep -q "Location: /api"; then
    echo -e "${GREEN}✅ PASSED: Nested locations parsing${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Nested locations parsing${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Test 5: Comments handling
echo -e "\n${YELLOW}TEST 5: Comments Handling${NC}"
cat > "$TEMP_CONFIG_DIR/comments.conf" << 'EOF'
# This is a comment
http {
    # Another comment
    server {
        listen 80; # Inline comment
        server_name test.com;
        # Comment between directives
        root /var/www;
    }
}
# Final comment
EOF

output=$($WEBSERV "$TEMP_CONFIG_DIR/comments.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 1" && \
   echo "$output" | grep -q "test.com"; then
    echo -e "${GREEN}✅ PASSED: Comments handling${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Comments handling${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Test 6: Empty server block
echo -e "\n${YELLOW}TEST 6: Empty Server Block${NC}"
cat > "$TEMP_CONFIG_DIR/empty.conf" << 'EOF'
http {
    server {
        listen 80;
    }
    
    server {
        listen 8080;
        server_name empty.com;
    }
}
EOF

output=$($WEBSERV "$TEMP_CONFIG_DIR/empty.conf" 2>&1)
if echo "$output" | grep -q "Number of servers: 2"; then
    echo -e "${GREEN}✅ PASSED: Empty server block handling${NC}"
    ((passed++))
else
    echo -e "${RED}❌ FAILED: Empty server block handling${NC}"
    echo "Output: $output"
    ((failed++))
fi

# Cleanup
rm -rf "$TEMP_CONFIG_DIR"

echo -e "\n========================================"
echo -e "${BLUE}PARSER TEST SUMMARY: ${GREEN}$passed passed${NC}, ${RED}$failed failed${NC}"
echo -e "========================================"

# Exit with error code if any tests failed
[ $failed -eq 0 ] || exit 1