from flask import Flask, request
import time

app = Flask(__name__)

@app.route('/log', methods=['POST', 'PUT'])
def receive_log():
    print("=== Received ===")
    
    # Handle multipart/form-data
    if request.mimetype == 'multipart/form-data':
        if 'user_id' in request.form:
            print(f"User ID: {request.form['user_id']}")
            
        if 'pixet_logs_file' in request.files:
            file = request.files['pixet_logs_file']
            print(f"Received file: {file.filename}")
            file.save(f'customer_{request.form["user_id"]}_{file.filename}')
        else:
            print("No file named 'pixet_logs_file' found in form data.")
            return {"Bad request": 400}, 400
    else:
        # Fallback for plain data
        data = request.data
        print(f"Received {len(data)} bytes of raw data.")
        with open('received_archive.zip', 'wb') as f:
            f.write(data)

    return {"status": "ok"}

if __name__ == '__main__':
    app.run(
        host='localhost',
        port=5000,
        ssl_context=('server.crt', 'server.key')
    )
