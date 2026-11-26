"""ElectroHeart Bluetooth Classic SPP -> local WebSocket bridge.

Install: py -m pip install pyserial websockets
Run:     py heart_bridge.py COM7
The COM port must be the Windows Bluetooth outgoing port for ESP32-ElectroHeart.
The website connects to ws://127.0.0.1:8765 and receives raw binary 8-byte frames.
"""
import asyncio
import sys
import threading
import serial
import websockets

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM7"
BAUD = 921600
clients = set()
loop = None

def serial_worker():
    global loop
    try:
        with serial.Serial(PORT, BAUD, timeout=0.1) as ser:
            while True:
                chunk = ser.read(ser.in_waiting or 1)
                if chunk and loop:
                    asyncio.run_coroutine_threadsafe(broadcast(bytes(chunk)), loop)
    except Exception as exc:
        print(f"Serial error on {PORT}: {exc}")

async def broadcast(data):
    if clients:
        await asyncio.gather(*(client.send(data) for client in tuple(clients)),
                             return_exceptions=True)

async def handler(websocket):
    clients.add(websocket)
    print("ElectroHeart dashboard connected")
    try:
        await websocket.wait_closed()
    finally:
        clients.discard(websocket)

async def main():
    global loop
    loop = asyncio.get_running_loop()
    threading.Thread(target=serial_worker, daemon=True).start()
    async with websockets.serve(handler, "127.0.0.1", 8765, max_size=None):
        print(f"Bridge listening at ws://127.0.0.1:8765; reading {PORT} at {BAUD} baud")
        await asyncio.Future()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass