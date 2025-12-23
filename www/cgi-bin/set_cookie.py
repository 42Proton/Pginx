#!/usr/bin/env python3

print("Content-Type: text/html")
print("Set-Cookie: session_id=xyz789; Path=/; HttpOnly; Max-Age=3600")
print("Set-Cookie: user_pref=dark_mode; Path=/; Max-Age=31536000")
print("")  # Blank line separates headers from body

print("<html><head><title>Cookie Set</title></head><body>")
print("<h1>Cookies Set Successfully</h1>")
print("<p>Two cookies have been set:</p>")
print("<ul>")
print("<li><b>session_id</b> = xyz789 (HttpOnly, 1 hour)</li>")
print("<li><b>user_pref</b> = dark_mode (1 year)</li>")
print("</ul>")
print("<p><a href='/cgi-bin/read_cookie.py'>Read cookies</a></p>")
print("</body></html>")
