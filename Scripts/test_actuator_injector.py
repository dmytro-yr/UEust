import socket
import struct
import time
import math

TARGET_IP = "127.0.0.1"
TARGET_PORT = 9003

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"=== Mock Actuator Injector Sending to {TARGET_IP}:{TARGET_PORT} ===")

start_time = time.time()

try:
    while True:
        elapsed =time.time() - start_time

        base_pwm = int(1500 + 400 * math.sin(elapsed*2.0))  # Oscillate between 1100 and 1900
        
        servo_packet = struct.pack('<8H', 
                                   base_pwm, base_pwm, base_pwm, base_pwm, 
                                   1500, 1500,1500,1500)  # 4 channels + 4 reserved
        sock.sendto(servo_packet, (TARGET_IP, TARGET_PORT))
        time.sleep(0.2)  # 50 Hz update rate
except KeyboardInterrupt:
    print("\n[INFO] Actuator injector stopped by user.")