#!/bin/bash
# Simple shell script CGI test

echo "Content-Type: text/html"
echo ""
echo "<html><body>"
echo "<h1>Shell Script CGI Test</h1>"
echo "<p><strong>Query String:</strong> $QUERY_STRING</p>"
echo "<p><strong>Request Method:</strong> $REQUEST_METHOD</p>"
echo "<p><strong>Server Name:</strong> $SERVER_NAME</p>"
echo "<p><strong>Server Port:</strong> $SERVER_PORT</p>"
echo "</body></html>"
