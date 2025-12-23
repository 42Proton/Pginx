#!/usr/bin/env python3
import os

print("Content-Type: text/html")
print("")  # Blank line

print("<html><head><title>Cookie Read</title></head><body>")
print("<h1>Cookies Received from HTTP_COOKIE</h1>")

http_cookie = os.environ.get('HTTP_COOKIE', '')

if http_cookie:
    print("<p>Raw Cookie header: <code>{}</code></p>".format(http_cookie))
    print("<h2>Parsed Cookies:</h2>")
    print("<table border='1' cellpadding='5'>")
    print("<tr><th>Name</th><th>Value</th></tr>")
    
    for cookie in http_cookie.split(';'):
        cookie = cookie.strip()
        if '=' in cookie:
            name, value = cookie.split('=', 1)
            print("<tr><td><b>{}</b></td><td>{}</td></tr>".format(name, value))
    
    print("</table>")
else:
    print("<p><i>No cookies received (HTTP_COOKIE not set)</i></p>")

print("<p><a href='/cgi-bin/set_cookie.py'>Set cookies</a></p>")
print("</body></html>")
