from flask import Blueprint, render_template, jsonify
import random
import math
import time
from datetime import datetime

monitoring_bp = Blueprint('monitoring', __name__)

sensor_data = {
    'gyro': {'x': 0.0, 'y': 0.0, 'z': 0.0},
    'accel': {'x': 0.0, 'y': 0.0, 'z': 0.0},
    'baro': {'pressure': 1013.25, 'altitude': 0.0},
    'mag': {'x': 0.0, 'y': 0.0, 'z': 0.0},
    'gps': {
        'lat': 0.0, 'lat_dir': 'N',
        'lon': 0.0, 'lon_dir': 'E',
        'alt': 0.0, 'satellites': 0
    },
}

def initialize_sensor_data():
    global sensor_data
    sensor_data = {
        'gyro': {'x': 0.05, 'y': -0.12, 'z': 0.03},
        'accel': {'x': 0.02, 'y': 0.15, 'z': 0.98},
        'baro': {'pressure': 1013.25, 'altitude': 120.5},
        'mag': {'x': 23.5, 'y': -12.8, 'z': 42.1},
        'gps': {
            'lat': 47.6062, 'lat_dir': 'N',
            'lon': 122.3321, 'lon_dir': 'W',
            'alt': 125.0, 'satellites': 9
        },
    }

# Simulation functions for each sensor type
def simulate_gyro():
    return {
        'x': random.uniform(-0.2, 0.2),
        'y': random.uniform(-0.2, 0.2),
        'z': random.uniform(-0.2, 0.2)
    }

def simulate_accel():
    return {
        'x': random.uniform(-0.05, 0.05),
        'y': random.uniform(-0.05, 0.05),
        'z': random.uniform(0.95, 1.05)
    }

def simulate_baro():
    pressure = sensor_data['baro']['pressure'] + random.uniform(-0.5, 0.5)
    altitude = 44330 * (1 - (pressure / 1013.25) ** 0.1903)
    return {
        'pressure': pressure,
        'altitude': altitude
    }

def simulate_mag():
    return {
        'x': sensor_data['mag']['x'] + random.uniform(-0.5, 0.5),
        'y': sensor_data['mag']['y'] + random.uniform(-0.5, 0.5),
        'z': sensor_data['mag']['z'] + random.uniform(-0.5, 0.5)
    }

def simulate_gps():
    lat = sensor_data['gps']['lat'] + random.uniform(-0.0001, 0.0001)
    lon = sensor_data['gps']['lon'] + random.uniform(-0.0001, 0.0001)
    alt = sensor_data['gps']['alt'] + random.uniform(-0.5, 0.5)
    satellites = min(12, max(6, sensor_data['gps']['satellites'] + random.randint(-1, 1)))

    return {
        'lat': lat,
        'lat_dir': 'N' if lat >= 0 else 'S',
        'lon': lon,
        'lon_dir': 'E' if lon >= 0 else 'W',
        'alt': alt,
        'satellites': satellites
    }

def update_sensor_data():
    global sensor_data
    sensor_data['gyro'] = simulate_gyro()
    sensor_data['accel'] = simulate_accel()
    sensor_data['baro'] = simulate_baro()
    sensor_data['mag'] = simulate_mag()
    sensor_data['gps'] = simulate_gps()


@monitoring_bp.route('/monitoring')
def monitoring():
    initialize_sensor_data()
    update_sensor_data()
    return render_template('monitoring.html', sensors=sensor_data)

@monitoring_bp.route('/api/sensors')
def get_sensors():
    update_sensor_data()
    return jsonify(sensor_data)
