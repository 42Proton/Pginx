#!/usr/bin/env python3
import os
import cgi
import uuid

def parse_cookies(cookie_str):
    """Parse HTTP_COOKIE into dictionary"""
    cookies = {}
    if cookie_str:
        for cookie in cookie_str.split(';'):
            cookie = cookie.strip()
            if '=' in cookie:
                name, value = cookie.split('=', 1)
                cookies[name] = value
    return cookies

# Parse query parameters
form = cgi.FieldStorage()
action = form.getvalue('action', 'index')

# Parse cookies from HTTP_COOKIE environment variable
http_cookie = os.environ.get('HTTP_COOKIE', '')
cookies = parse_cookies(http_cookie)

print("Content-Type: text/html")

if action == 'login':
    # Generate new session ID
    session_id = str(uuid.uuid4())
    print("Set-Cookie: session_id={}; Path=/; HttpOnly; Max-Age=3600".format(session_id))
    print("")
    print("<html><body>")
    print("<h1>Login Successful</h1>")
    print("<p>Session ID: {}</p>".format(session_id))
    print("<p><a href='/cgi-bin/session.py?action=profile'>View Profile</a></p>")
    print("</body></html>")

elif action == 'logout':
    # Delete session cookie by setting Max-Age=0
    print("Set-Cookie: session_id=; Path=/; Max-Age=0")
    print("")
    print("<html><body>")
    print("<h1>Logged Out</h1>")
    print("<p><a href='/cgi-bin/session.py'>Home</a></p>")
    print("</body></html>")

elif action == 'profile':
    print("")
    print("<html><body>")
    if 'session_id' in cookies:
        print("<h1>Profile Page</h1>")
        print("<p>Logged in with session: {}</p>".format(cookies['session_id']))
        print("<p><a href='/cgi-bin/session.py?action=logout'>Logout</a></p>")
    else:
        print("<h1>Not Logged In</h1>")
        print("<p><a href='/cgi-bin/session.py?action=login'>Login</a></p>")
    print("</body></html>")

else:
    # Index page
    print("")
    print("<html><body>")
    print("<h1>Session Management Demo</h1>")
    if 'session_id' in cookies:
        print("<p>Current session: {}</p>".format(cookies['session_id']))
        print("<p><a href='/cgi-bin/session.py?action=profile'>Profile</a> | ")
        print("<a href='/cgi-bin/session.py?action=logout'>Logout</a></p>")
    else:
        print("<p>Not logged in</p>")
        print("<p><a href='/cgi-bin/session.py?action=login'>Login</a></p>")
    print("</body></html>")
