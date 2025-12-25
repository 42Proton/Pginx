#!/usr/bin/env python3
# Simple GET request test
import os

query_string = os.environ.get('QUERY_STRING', '')

print("Content-Type: text/html\r")
print("\r")
print("<html><body>")
print("<h1>GET Request Test</h1>")
print(f"<p><strong>Query String:</strong> {query_string}</p>")
print(f"<p><strong>Request Method:</strong> {os.environ.get('REQUEST_METHOD', 'N/A')}</p>")
print(f"<p><strong>Script Name:</strong> {os.environ.get('SCRIPT_NAME', 'N/A')}</p>")
print(f"<p><strong>Path Info:</strong> {os.environ.get('PATH_INFO', 'N/A')}</p>")
print("</body></html>")
