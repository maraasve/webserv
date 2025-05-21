import os
import urllib.parse

upload_dir = os.environ.get("UPLOAD_DIR", "")
os.makedirs(upload_dir, exist_ok=True)
query_string = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query_string)
file_param = params.get("file", [None])[0]
if file_param:
	safe_filename = os.path.basename(file_param)
	file_path = os.path.join(upload_dir, safe_filename)
	try:
		os.remove(file_path)
		print(f"Deleted file: {safe_filename}")
	except FileNotFoundError:
		print("File not found.")
	except Exception as e:
		print(f"Error deleting file: {e}")
else:
	print("No file specified.")