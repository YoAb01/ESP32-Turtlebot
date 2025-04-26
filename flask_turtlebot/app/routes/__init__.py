from app.routes.home import home_bp
from app.routes.manual import manual_bp
from app.routes.led_control import led_control_bp

def register_routes(app):
    app.register_blueprint(home_bp)
    app.register_blueprint(manual_bp)
    app.register_blueprint(led_control_bp)

