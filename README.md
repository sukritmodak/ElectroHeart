# ElectroHeart — 3-Channel PCG Monitor

ElectroHeart streams three analog phonocardiogram (PCG) channels from a classic ESP32-WROOM directly to a Chrome/Edge dashboard using **Bluetooth Low Energy (BLE)**.

## Hardware

- Board: classic ESP32 / ESP32-WROOM (not ESP32-S3)
- Microphone analog outputs: GPIO 34, GPIO 35, GPIO 32
- ADC resolution: 12-bit
- Sampling: 2000 samples/second/channel
- BLE device name: `ElectroHeart-BLE`

## BLE-only setup

1. Open Arduino IDE and select **ESP32 Dev Module** (or the matching classic ESP32 board).
2. Open `firmware/ElectroHeart_3CH.ino` and upload it to the ESP32-WROOM.
3. Open [ElectroHeart dashboard](https://sukritmodak.github.io/ElectroHeart/) in Chrome or Edge. The page must be served over HTTPS.
4. Click **Connect Bluetooth** and select `ElectroHeart-BLE` in the browser's device chooser.
5. Allow the connection. Live channel data should begin appearing.

No USB data connection, Windows COM port, Bluetooth Classic SPP, or Python bridge is used by this version.

## BLE data protocol

- Custom service UUID: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- Notify characteristic UUID: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`
- Each sample frame is 8 bytes: `A5 5A` followed by three 16-bit little-endian ADC values.
- Firmware batches 20 frames (160 bytes) into each BLE notification.
- BLE MTU is requested as 185 bytes to carry the batch.

## Dashboard

The dashboard provides three live PCG waveforms, ADC values, experimental S1/S2 event markers, channel BPM estimates, consensus BPM, live playback of received ADC samples, pause/resume, and CSV export.

BPM is calculated from successive detected S1 events and is withheld until enough intervals are available. The acoustic event detector is experimental and requires validation against reference recordings and real hardware. S3/S4 are not automatically classified.

## Troubleshooting

- If the browser says Web Bluetooth is unavailable, use desktop Chrome or Edge on HTTPS and ensure the computer has BLE hardware.
- If `ElectroHeart-BLE` is not listed, reset the ESP32 and scan again; ensure the board is a classic ESP32-WROOM with BLE enabled.
- If it connects but no frames arrive, confirm the BLE service/characteristic UUIDs in the firmware match the dashboard and check that the microphone/ADC pins are connected.
- Keep each microphone analog output within the ESP32 ADC input range (0–3.3 V).

## Safety

This is an educational/research prototype, not a medical device. Do not use its BPM or heart-sound labels for diagnosis or treatment decisions.
