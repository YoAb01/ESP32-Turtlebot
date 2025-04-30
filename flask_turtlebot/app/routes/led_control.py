import json
from flask import Blueprint, render_template, request, jsonify
import socket
import time
import logging
import threading

led_control_bp = Blueprint('led_control_bp', __name__)
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Global vars
ESP32_IP = '192.168.1.10'
ESP32_PORT = 4210
CONNECTION_TIMEOUT = 10  # Consider ESP32 disconnected if no heartbeat for 10 seconds

# Connection state tracking
esp32_connected = False
last_heartbeat_time = 0
esp32_sock = None  # Persistent socket for communication
socket_lock = threading.Lock()  # Lock for thread-safe socket usage

# LED pin definitions
LED_PINS = {
    'red': 23,
    'blue': 22,
    'green': 21,
    'yellow': 19,
}

def initialize_socket():
    """Create and configure the UDP socket for ESP32 communication"""
    global esp32_sock

    if esp32_sock is not None:
        try:
            esp32_sock.close()
        except:
            pass

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(2)  # 2 second timeout for all operations

    # Bind to any available port for receiving responses
    try:
        sock.bind(('0.0.0.0', 0))
        logger.info(f"Socket bound to port {sock.getsockname()[1]}")
    except Exception as e:
        logger.error(f"Failed to bind socket: {e}")
        sock.close()
        return None

    esp32_sock = sock
    return sock


def heartbeat_listener():
    """Background thread that listens for heartbeats from the ESP32"""
    global esp32_connected, last_heartbeat_time, esp32_sock

    logger.info("Starting heartbeat listener thread")

    while True:
        # Make sure we have a valid socket
        if esp32_sock is None:
            with socket_lock:
                initialize_socket()
                if esp32_sock is None:
                    logger.error("Failed to initialize socket, retrying in 5 seconds")
                    time.sleep(5)
                    continue

        # Listen for incoming data (both heartbeats and responses)
        try:
            with socket_lock:
                esp32_sock.settimeout(1)
                try:
                    data, addr = esp32_sock.recvfrom(1024)
                    if data:
                        data_str = data.decode('utf-8')
                        # Check if it's a heartbeat message
                        if data_str.startswith("HEARTBEAT:"):
                            heartbeat_id = data_str.split(":")[1]
                            logger.debug(f"Received heartbeat {heartbeat_id} from {addr}")
                            last_heartbeat_time = time.time()
                            esp32_connected = True
                        elif data_str == "PONG":
                            logger.debug(f"Received PONG from {addr}")
                            last_heartbeat_time = time.time()
                            esp32_connected = True
                        elif data_str == "ACK":
                            logger.debug(f"Received ACK from {addr}")
                        else:
                            logger.info(f"Received message from ESP32: {data_str}")
                except socket.timeout:
                    # This is expected, just continue
                    pass
        except Exception as e:
            logger.error(f"Error in heartbeat listener: {e}")
            with socket_lock:
                if esp32_sock:
                    try:
                        esp32_sock.close()
                    except:
                        pass
                esp32_sock = None

        # Check if we've gone too long without a heartbeat
        if esp32_connected and time.time() - last_heartbeat_time > CONNECTION_TIMEOUT:
            logger.warning(f"No heartbeat from ESP32 for {CONNECTION_TIMEOUT} seconds, marking as disconnected")
            esp32_connected = False

        # Send a PING if we think we're disconnected
        if not esp32_connected:
            try:
                with socket_lock:
                    if esp32_sock:
                        esp32_sock.sendto(b"PING", (ESP32_IP, ESP32_PORT))
                        logger.debug(f"Sent PING to ESP32")
            except Exception as e:
                logger.error(f"Error sending PING: {e}")

        # Sleep briefly before next check
        time.sleep(0.5)


def send_udp_command(command):
    """Send a command to the ESP32 using the persistent socket"""
    global esp32_sock, esp32_connected

    if not esp32_connected:
        logger.warning("Trying to send command while ESP32 is disconnected")

    try:
        with socket_lock:
            if esp32_sock is None:
                initialize_socket()
                if esp32_sock is None:
                    raise Exception("Failed to initialize socket")

            logger.info(f"Sending command to ESP32 ({ESP32_IP}): {command}")
            esp32_sock.sendto(command.encode(), (ESP32_IP, ESP32_PORT))

            # Wait for acknowledgment with timeout
            try:
                esp32_sock.settimeout(2)
                data, addr = esp32_sock.recvfrom(1024)
                if data == b"ACK":
                    logger.info("Command acknowledged by ESP32")
                    return True
                else:
                    logger.info(f"Received unexpected response: {data}")
                    return True  # Still count as success
            except socket.timeout:
                logger.warning("No acknowledgment received for command")
                return False
    except Exception as e:
        logger.error(f"Error sending command: {e}")
        # Reset socket on error
        with socket_lock:
            if esp32_sock:
                try:
                    esp32_sock.close()
                except:
                    pass
            esp32_sock = None
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
        if send_udp_command(command):
            return jsonify({"status": "success", "message": f"LED {color} turned {'ON' if state else 'OFF'}"}), 200
        else:
            return jsonify({"status": "error", "message": "Command sent but not acknowledged"}), 500
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
        if send_udp_command(command):
            return jsonify({"message": f"RGB LED set to R: {r}, G: {g}, B: {b}"}), 200
        else:
            return jsonify({"status": "error", "message": "Command sent but not acknowledged"}), 500
    except Exception as e:
        logger.error(f"Error in control_rgb: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500


# Connection status route
@led_control_bp.route('/api/connection_status', methods=['GET'])
def connection_status():
    global esp32_connected

    return jsonify({
        "connected_to_esp32": esp32_connected,
        "esp32_ip": ESP32_IP,
        "last_heartbeat": int(time.time() - last_heartbeat_time) if last_heartbeat_time > 0 else None
    })

initialize_socket()
heartbeat_thread = threading.Thread(target=heartbeat_listener, daemon=True)
heartbeat_thread.start()
