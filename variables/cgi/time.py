from datetime import datetime
now = datetime.now()
print(f"""
<!DOCTYPE html>
<html>
<head><title>Current Time</title></head>
<body>
    <h1>Current Server Time</h1>
    <p>{now.strftime('%Y-%m-%d %H:%M:%S')}</p>
</body>
</html>
""")