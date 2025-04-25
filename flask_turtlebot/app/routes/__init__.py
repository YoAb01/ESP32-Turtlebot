from app.routes.home import home_bp
from app.routes.manual import manual_bp

def register_routes(app):
    app.register_blueprint(home_bp)
    app.register_blueprint(manual_bp)

