from flask import Flask
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(
        app=app,
        async_mode="threading",
        ping_timeout=1000,
        ping_interval=0,
    )

from app.routes import register_routes
register_routes(app)

from app import sockets
