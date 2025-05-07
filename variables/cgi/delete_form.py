import os
import urllib.parse

#this has to change depending on the root
# UPLOAD_DIR = os.environ.get("UPLOAD_DIR", "")
UPLOAD_DIR = "variables/uploads"
os.makedirs(UPLOAD_DIR, exist_ok=True)
query_string = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query_string)
file_param = params.get("file", [None])[0]
if file_param:
	safe_filename = os.path.basename(file_param)
	file_path = os.path.join(UPLOAD_DIR, safe_filename)
	try:
		os.remove(file_path)
		print("Content-Type: text/plain\n")
		print(f"Deleted file: {safe_filename}")
	except FileNotFoundError:
		print("Content-Type: text/plain\n")
		print("File not found.")
	except Exception as e:
		print("Content-Type: text/plain\n")
	print(f"Error deleting file: {e}")
else:
	print("Content-Type: text/plain\n")
	print("No file specified.")