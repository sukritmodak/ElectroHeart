# ElectroHeart — 3-Channel PCG Monitor

ElectroHeart streams three analog phonocardiogram (PCG) channels from a classic ESP32-WROOM directly to a Chrome/Edge dashboard using Bluetooth Low Energy (BLE).

## Hardware and wiring

Use three KY-037 or KY-038 microphone modules in analog-output mode:

| Microphone | Module pin | ESP32 |
|---|---|---|
| Left | AO | GPIO 34 |
| Centre | AO | GPIO 35 |
| Right | AO | GPIO 32 |
| All modules | GND | ESP32 GND |
| All modules | VCC | 3V3 |

- Connect **AO (analog output)**, not DO (digital comparator output).
- Power the modules from 3.3 V and share a common ground.
- Keep every AO signal within the ESP32 ADC input range (0–3.3 V). Never connect a 5 V output directly to an ESP32 pin.
- Adjust the module's sensitivity potentiometer carefully. The dashboard estimates and removes the DC bias independently for each channel.
- KY-037/KY-038 are inexpensive sound-sensor modules, not calibrated medical PCG microphones. Validate the signal quality against reference equipment.

## BLE-only setup

1. In Arduino IDE select **ESP32 Dev Module** (or the matching classic ESP32 board).
2. Open `firmware/ElectroHeart_3CH.ino` and upload it to the ESP32-WROOM. USB is used only for firmware upload.
3. Open [ElectroHeart dashboard](https://sukritmodak.github.io/ElectroHeart/) in desktop Chrome or Edge over HTTPS.
4. Click **Connect Bluetooth**. The browser lists nearby BLE devices; select `ElectroHeart-BLE` (or the ESP32 device name if it is displayed differently).
5. Allow the browser connection. The dashboard then checks for the ElectroHeart GATT service. Live data should appear when notifications begin.

No USB data streaming, Bluetooth Classic SPP, COM port, or Python bridge is used.

## BLE protocol

- Service UUID: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- Notify characteristic UUID: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`
- Each sample frame: 8 bytes, `A5 5A` + three uint16 little-endian ADC values.
- Firmware batches 2 frames (16 bytes) per notification. This deliberately stays below the default ATT MTU payload limit of 20 bytes, so data transfer does not depend on successful MTU negotiation.

## Dashboard

Three live PCG waveforms, raw ADC values, baseline-corrected signal plots, experimental S1/S2 event markers, per-channel BPM estimates, consensus BPM, playback from received analog samples, pause/resume, and CSV export.

BPM is estimated from successive S1-to-S1 intervals and remains blank until enough intervals are available. Detection is experimental and must be validated against reference recordings. S3/S4 are not automatically classified.

## Troubleshooting

- If Web Bluetooth is unavailable, use desktop Chrome or Edge on HTTPS and confirm the computer has BLE hardware.
- The chooser intentionally shows all nearby BLE devices so a missing/filtered advertised name does not hide the ESP32. Select the ESP32 device; the dashboard verifies its ElectroHeart service after connection.
- If no ESP32 device appears at all, confirm the latest firmware was uploaded to a classic ESP32-WROOM, power-cycle it, and check that Bluetooth is enabled on the computer.
- If the browser reports connected but the frame counter stays at zero, re-upload the latest firmware (16-byte notifications), then check the service/characteristic UUIDs and reset the ESP32.
- If ADC readings are stuck near 0 or 4095, check wiring, module output voltage, and potentiometer settings.
- If waveform amplitude is very small, adjust microphone placement/sensitivity and check the module's AO signal with an oscilloscope or ADC test.

## Safety

This is an educational/research prototype, not a medical device. Do not use its BPM or event labels for diagnosis or treatment decisions.
