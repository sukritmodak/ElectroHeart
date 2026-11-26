"""Legacy Bluetooth Classic serial bridge placeholder.

The current ElectroHeart firmware streams directly to the browser using
Bluetooth Low Energy (BLE). The dashboard does not use this file, a COM
port, pyserial, or a local WebSocket bridge.

See README.md for BLE setup instructions.
"""

if __name__ == "__main__":
    print("ElectroHeart now uses direct BLE from the browser.")
    print("No Python bridge is required. See README.md for setup.")
