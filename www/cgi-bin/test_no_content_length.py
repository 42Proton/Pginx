#!/usr/bin/env python3
# CGI script that outputs without Content-Length header
# Server should read until EOF to get complete output
import sys

print("Content-Type: text/plain")
print("")  # End of headers (no Content-Length!)

# Output data - server should read until EOF
for i in range(10):
    print(f"Line {i+1}: Testing EOF-based output reading")
    sys.stdout.flush()

print("")
print("=== End of CGI Output ===")
print("Server should have read all lines until EOF")
print("Subject requirement: 'EOF will mark the end of the returned data'")
