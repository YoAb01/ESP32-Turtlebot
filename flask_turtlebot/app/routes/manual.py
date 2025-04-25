from flask import Blueprint, render_template

manual_bp = Blueprint('manual', __name__)

@manual_bp.route('/manual')
def manual():
    return render_template('manual.html')
