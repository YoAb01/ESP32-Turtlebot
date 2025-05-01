from flask import Blueprint, render_template

monitoring_bp = Blueprint('monitoring', __name__)

@monitoring_bp.route('/monitoring')
def monitoring():
    return render_template('monitoring.html')
