from app import app, socketio

if __name__ == '__main__':
    try:
        socketio.run(
            app=app,
            host='127.0.0.1',
            port=5050,
            debug=True
        )
    except KeyboardInterrupt:
        print('🟥Terminating flask app...')
