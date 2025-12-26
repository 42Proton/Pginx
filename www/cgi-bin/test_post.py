#!/usr/bin/env python3
# Test CGI script for POST requests (including chunked)
import os
import sys

# Get POST data from stdin
content_length = int(os.environ.get('CONTENT_LENGTH', 0))

# Read POST data - server should have un-chunked it
# CGI expects EOF as end of body
post_data = ""
if content_length > 0:
    post_data = sys.stdin.read(content_length)
else:
    # Read until EOF if no Content-Length
    post_data = sys.stdin.read()

# Output CGI response
print("Content-Type: text/html\r")
print("\r")
print("<html><body>")
print("<h1>POST Data Received</h1>")
print(f"<p><strong>Content-Length:</strong> {content_length}</p>")
print(f"<p><strong>POST Data:</strong></p>")
print(f"<pre>{post_data}</pre>")
print("<p><strong>Transfer-Encoding:</strong> " + os.environ.get('HTTP_TRANSFER_ENCODING', 'Not set') + "</p>")
print("<p>If this was a chunked request, the server should have un-chunked it.</p>")
print("<p>CGI should only see the clean body data.</p>")
print("</body></html>")
