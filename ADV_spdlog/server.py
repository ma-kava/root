from flask import Flask, request

app = Flask(__name__)

@app.route('/log', methods=['POST'])
def receive_log():
    print("=== Received POST ===")
    print("Body:", request.data.decode('utf-8'))
    return {"status": "ok"}

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=5000)
    