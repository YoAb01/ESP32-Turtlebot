import json
from flask import Blueprint, render_template, request, jsonify
import socket, socketio
import time
import logging

led_control_bp = Blueprint('led_control_bp', __name__)
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Global vars
ESP32_IP = '192.168.1.10'
ESP32_PORT = 4210
LED_PINS = {
    'red', 23,
    'blue', 22,
    'green', 21,
    'yellow', 19,
}

def send_udp_command(command):
    try:
        logger.info(f"Sending command to ESP32 ({ESP32_IP}) : {command}")
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(command.encode(), (ESP32_IP, ESP32_PORT))
        logger.info("Command sent successfully")
        sock.close()
    except Exception as e:
        logger.error(f"Error sending command: {e}")
        raise

# Route to control individual LEDs
@led_control_bp.route('/api/led', methods=['POST'])
def control_led():
    try:
        data = request.json
        color = data.get('color')
        state = data.get('state')

        logger.debug(f"Received request: color={color}, state={state}")

        if color not in LED_PINS:
            return jsonify({"error": "Invalid LED color"}), 400

        # Send the command to the ESP32
        command = f"LED {color} {'ON' if state else 'OFF'}"
        send_udp_command(command)

        return jsonify({"status": "success", "message": f"LED {color} turned {'ON' if state else 'OFF'}"}), 200
    except Exception as e:
        logger.error(f"Error in control_led: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

# Route to control RGB LED
@led_control_bp.route('/api/rgb', methods=['POST'])
def control_rgb():
    try:
        data = request.json
        r = data.get('r', 0)
        g = data.get('g', 0)
        b = data.get('b', 0)

        # Send the RGB command to the ESP32
        command = f"RGB {r} {g} {b}"
        send_udp_command(command)

        return jsonify({"message": f"RGB LED set to R: {r}, G: {g}, B: {b}"}), 200
    except Exception as e:
        logger.error(f"Error in control_rgb: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

# Connection status route
@led_control_bp.route('/api/connection_status', methods=['GET'])
def connection_status():
    current_ip = ESP32_IP
    is_connected = False
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(1)  # 1 second timeout

        # Send a ping packet
        sock.sendto(b"PING", (current_ip, ESP32_PORT))

        # Try to receive a response
        try:
            data, addr = sock.recvfrom(1024)
            if data == b"PONG" or data:  # Accept any response as confirmation
                is_connected = True
        except socket.timeout:
            is_connected = False

        sock.close()
    except Exception as e:
        logging.error(f"Error checking ESP32 connection: {e}")
        is_connected = False

    return jsonify({
        "connected_to_esp32": is_connected,
        "esp32_ip": current_ip
    })
