#/usr/bin/python3

from wsgiref.handlers import format_date_time
import os
from datetime import datetime, timezone
import time

def cgi_response(msg = "Welcome to Session Managment CGI, This is your first login"):
    datetime_now = datetime.now()
    timestamp = time.mktime(datetime_now.timetuple())
    last_login_date = format_date_time(timestamp)
    print("Status: 200 OK")
    print("Content-Type: text/html")
    print(f"Set-Cookie: last_login={last_login_date}; Max-Age=86400")
    print("")
    print(
f"""<html>
<head>
<title>Session Managment</title>
</head>
<body>
<h1>{msg}</h1>
</body>
</html>"""
)

if "HTTP_COOKIE" not in os.environ:
    cgi_response()
else:
    http_cookie = os.environ["HTTP_COOKIE"]
    cookies = http_cookie.split("; ")
    for cookie in cookies:
        cookie = cookie.lstrip()
        cookie_name, sep, cookie_value = cookie.partition('=')
        if cookie_name == "last_login":
            try:
                parsed_date = datetime.strptime(cookie_value, "%a, %d %b %Y %H:%M:%S GMT")
                parsed_date = parsed_date.replace(tzinfo=timezone.utc)
                datetime_now = datetime.now(timezone.utc)
                diff_date = datetime_now - parsed_date
                if diff_date.total_seconds() < 0:
                    cgi_response("Nice you are a time traveller!")
                else:
                    msg = f"{int(diff_date.total_seconds())} seconds passed since last login"
                    cgi_response(msg)
                break
            except Exception as e:
                cgi_response()
                break