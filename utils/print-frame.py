import serial
import struct

board = serial.Serial('/dev/ttyUSB0', 230600)

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
    if all(x == 0xFF for x in list(frame)):
        continue

    unpacked = struct.unpack('<LLL', frame)
    sensor_id = unpacked[0] & 0x03
    lfsr = unpacked[0] >> 2 & 0x1F
    lfsr_loc = unpacked[1] & 0x01FFFF
    timestamp = unpacked[2]


    print(f"Sensor ID: {sensor_id}, LFSR: {lfsr}, Loc: {lfsr_loc}, Timestamp: {timestamp}")
