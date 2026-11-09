import asyncio
import sys
import serial
import websockets

BAUD = 115200
HOST = "127.0.0.1"
PORT = 8765

async def main():
    if len(sys.argv) != 2:
        print("Usage: py bridge.py COMx")
        return

    com_port = sys.argv[1]
    try:
        ser = serial.Serial(com_port, BAUD, timeout=0.1)
    except Exception as exc:
        print(f"Could not open {com_port}: {exc}")
        print("Use the Windows Bluetooth OUTGOING COM port, not necessarily the USB COM port.")
        print("Close Arduino Serial Monitor and any other program using that COM port.")
        return

    print(f"Bluetooth serial connected on {com_port} at {BAUD} baud.")
    print(f"Local WebSocket bridge: ws://{HOST}:{PORT}")
    print("Open the hosted website in this same laptop and click Connect Bluetooth.")

    async def handler(websocket):
        try:
            while True:
                data = await asyncio.to_thread(ser.read, 512)
                if data:
                    await websocket.send(data)
                else:
                    await asyncio.sleep(0.005)
        except websockets.ConnectionClosed:
            pass

    try:
        async with websockets.serve(handler, HOST, PORT, max_size=None):
            await asyncio.Future()
    finally:
        ser.close()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Stopped.")
