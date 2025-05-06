import serial
import struct

board = serial.Serial('/dev/ttyUSB0', 115200)

SYNC_FRAME = [0xFF] * 12

# Wait for sync (12 bytes at 0xff)
print("Waiting for sync...")
buffer = [0] * 12
while True:
    b = board.read(1)
    buffer[0:11] = buffer[1:12]
    buffer[11] = b[0]
    if buffer == SYNC_FRAME:
        break
print("Sync received!")

while True:
    # Read the frame
    frame = board.read(12)
    # print(frame)

    # print(frame.hex())

    # Ignore sync frames
    # if buffer == SYNC_FRAME:
        # continue

    unpacked = struct.unpack('<BH', frame[0:3])
    sensor_id = unpacked[0] & 0x03
    lfsr = unpacked[0] >> 2
    width = unpacked[1]

    print(f"Sensor ID: {sensor_id}, LFSR: {lfsr}, Width: {width}")