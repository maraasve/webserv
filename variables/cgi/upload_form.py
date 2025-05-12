#!/usr/bin/env python3
import cgi, os

#this has to be the root
UPLOAD_DIR = "variables/uploads"
#UPLOAD_DIR = os.environ.get("UPLOAD_DIR", "")
os.makedirs(UPLOAD_DIR, exist_ok=True)
form = cgi.FieldStorage()
fileitem = form['file'] if 'file' in form else None

if fileitem is not None and fileitem.filename:
    fn = os.path.basename(fileitem.filename)
    with open(os.path.join(UPLOAD_DIR, fn), 'wb') as out:
        out.write(fileitem.file.read())
    print(f"<h1>Upload successful: {fn}</h1>")
    print(f"<p><a href=\"/uploads/{fn}\">Download</a></p>")
else:
    print("<h1>No file was uploaded</h1>")
