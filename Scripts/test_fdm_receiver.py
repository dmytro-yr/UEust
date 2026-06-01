import socket
import json
import math

UDP_IP = "127.0.0.1"
UDP_PORT = 9002

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"=== Mock ArduPilot FDM Receiver Listening on Port {UDP_PORT} ===")

while True:
    data, addr = sock.recvfrom(4096)
    try:
        payload = json.loads(data.decode('utf-8'))
        
        # Extract Vectors
        pos = payload.get("position", [0, 0, 0])
        vel = payload.get("velocity", [0, 0, 0])
        att = payload.get("attitude", [0, 0, 0]) # Radians
        ang_vel = payload.get("angular_velocity", [0, 0, 0])
        
        # Convert radians back to degrees for easy human validation check
        roll_deg = math.degrees(att[0])
        pitch_deg = math.degrees(att[1])
        yaw_deg = math.degrees(att[2])
        
        # Clear screen/line trick for a clean dashboard view
        print(f"\rTS: {payload.get('timestamp'):.2f}s | "
              f"POS (NED m): [{pos[0]:.2f}, {pos[1]:.2f}, {pos[2]:.2f}] | "
              f"ATT (Deg): R:{roll_deg:6.1f}° P:{pitch_deg:6.1f}° Y:{yaw_deg:6.1f}°", end="", flush=True)
              
    except json.JSONDecodeError:
        print(f"\n[ERROR] Received malformed non-JSON payload: {data}")
    except Exception as e:
        print(f"\n[ERROR] Parsing issue: {e}")