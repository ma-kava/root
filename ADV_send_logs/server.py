from flask import Flask, request

app = Flask(__name__)

@app.route('/log', methods=['POST', 'PUT'])
def receive_log():
    print("=== Received ===")
    data = request.data
    print(f"Received {len(data)} bytes.")

    with open('received_archive.zip', 'wb') as f:
        f.write(data)

    return {"status": "ok"}

if __name__ == '__main__':
    app.run(
        host='127.0.0.1',
        port=5000,
        ssl_context=('server.crt', 'server.key')
    )
