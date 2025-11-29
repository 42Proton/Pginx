#!/usr/bin/env python3
# Test CGI script for chunked POST data
import os
import sys

# Read all data from stdin until EOF
# Server should have un-chunked the request
body_data = sys.stdin.read()

# Output without Content-Length to test EOF handling
print("Content-Type: text/plain")
print("")  # End of headers

# Output the received data
print("=== Chunked POST Test Results ===")
print(f"Received {len(body_data)} bytes")
print(f"Body data: {body_data}")
print("")
print("Environment variables:")
print(f"REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', 'N/A')}")
print(f"CONTENT_LENGTH: {os.environ.get('CONTENT_LENGTH', 'N/A')}")
print(f"CONTENT_TYPE: {os.environ.get('CONTENT_TYPE', 'N/A')}")
print(f"HTTP_TRANSFER_ENCODING: {os.environ.get('HTTP_TRANSFER_ENCODING', 'N/A')}")
print("")
print("If Transfer-Encoding was chunked, server should have:")
print("1. Un-chunked the body before passing to CGI")
print("2. Set correct Content-Length")
print("3. CGI reads until EOF (stdin pipe close)")
