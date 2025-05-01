#!/usr/bin/env python3
import cgi, os

UPLOAD_DIR = "variables/uploads"
os.makedirs(UPLOAD_DIR, exist_ok=True)

form = cgi.FieldStorage()
fileitem = form['file'] if 'file' in form else None

if fileitem and fileitem.filename:
    fn = os.path.basename(fileitem.filename)
    with open(os.path.join(UPLOAD_DIR, fn), 'wb') as out:
        out.write(fileitem.file.read())
    print("Content-Type: text/html\r\n\r\n")
    print(f"<h1>Upload successful: {fn}</h1>")
    print(f"<p><a href=\"/uploads/{fn}\">Download</a></p>")
else:
    print("Content-Type: text/html\r\n\r\n")
    print("<h1>No file was uploaded</h1>")
