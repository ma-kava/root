from flask import Flask, request
import time

app = Flask(__name__)

@app.route('/log', methods=['POST', 'PUT'])
def receive_log():
    print("=== Received ===")
    data = request.data
    print(f"Received {len(data)} bytes.")

    with open('received_archive.txt', 'wb') as f:
        f.write(data)

    # os._exit(1) # to test Error::Read
    # time.sleep(5) # testing timeout
    # return {"Bad request": 400}
    return {"status": "ok"}

if __name__ == '__main__':
    app.run(
        host='localhost',
        port=5000,
        ssl_context=('server.crt', 'server.key')
    )
