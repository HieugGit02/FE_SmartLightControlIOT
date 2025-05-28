from flask import Flask
from pyngrok import ngrok

# Create Flask app
app = Flask(__name__)

@app.route('/')
def index():
    return "Your API is working!"

# Start app and Ngrok tunnel
if __name__ == '__main__':
    # Start Ngrok tunnel to localhost:5000
    public_url = ngrok.connect(5000)
    print(f" * Public URL: {public_url}")

    # Start Flask app
    app.run(port=5000)
