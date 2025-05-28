from flask import Flask, request, jsonify, render_template
from db_connector_rasp import get_db_connection
from pyngrok import ngrok

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/control', methods=['POST'])
def set_command():
    conn = get_db_connection()
    if conn is None:
        return jsonify({'error': 'Database connection failed'}), 500

    try:
        data = request.get_json()
        command = data.get('command')
        if command is None:
            return jsonify({'error': 'Missing command parameter'}), 400

        cursor = conn.cursor()
        cursor.execute("UPDATE lamps SET state = %s WHERE id = 1", (command,))
        conn.commit()

        return jsonify({'status': f'Command set to {command}'})

    except Exception as e:
        return jsonify({'error': str(e)}), 500

    finally:
        cursor.close()
        conn.close()

@app.route('/api/control', methods=['GET'])
def get_command():
    conn = get_db_connection()
    if conn is None:
        return jsonify({'error': 'Database connection failed'}), 500

    try:
        cursor = conn.cursor()
        cursor.execute("SELECT state FROM lamps WHERE id = 1")
        row = cursor.fetchone()
        command = row[0] if row else 'unknown'

        return jsonify({'command': command})

    except Exception as e:
        return jsonify({'error': str(e)}), 500

    finally:
        cursor.close()
        conn.close()

if __name__ == '__main__':
    # public_url = ngrok.connect(5000)
    # print(f" * Public URL: {public_url}")
    app.run(host='0.0.0.0', port=5000, debug=True)
