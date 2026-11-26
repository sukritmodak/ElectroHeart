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

## BLE discovery diagnostic

If Chrome's chooser still does not show the ESP32, isolate the radio/advertising layer before troubleshooting the waveform dashboard:

1. In Arduino IDE open `firmware/BLE_Advertising_Test/BLE_Advertising_Test.ino` (open the sketch from its own folder).
2. Select **ESP32 Dev Module** for a classic ESP32-WROOM board and upload this test sketch.
3. Open Serial Monitor at **115200 baud**. It should print `BLE advertising start requested with the local name in the primary packet.`
4. On an Android phone, install a BLE scanner such as nRF Connect and scan for **ElectroHeart-TEST**. Also try the Chrome dashboard on a computer with Bluetooth LE.
5. If the phone sees ElectroHeart-TEST but Windows Chrome does not, the ESP32 is advertising; troubleshoot the Windows Bluetooth adapter/driver and Chrome permissions.
6. If neither phone nor computer sees it, check that the selected board is a classic ESP32 with BLE support, confirm upload success, and power-cycle the board.
7. After the test, upload the normal `firmware/ElectroHeart_3CH.ino` sketch again to restore the three-channel monitor.

The test sketch does not read microphones or stream PCG data. It only advertises a minimal BLE service to test discovery. The normal firmware places its device name in the primary advertising packet (rather than relying on a scan response), then prints boot/advertising diagnostics to Serial Monitor at 115200 baud. This serial output is for diagnostics only, not the data stream.

## BLE protocol

- Service UUID: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- Notify characteristic UUID: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`
- Each sample frame: 8 bytes, `A5 5A` + three uint16 little-endian ADC values.
- Firmware batches 20 frames (160 bytes) per notification and requests ATT MTU 185 to reduce BLE notification overhead. The browser parser accepts partial or combined notifications and reassembles complete 8-byte frames.

## Dashboard

Three live PCG waveforms plotted from the original raw ADC samples (0–4095), with visible per-channel raw ADC, tracked baseline and envelope readouts. The additional Signal channel comparison graph overlays all three baseline-corrected channels on a shared ADC-count scale; the Signal envelope graph plots |raw ADC − baseline| in ADC counts. The original raw ADC buffer and CSV values remain untouched. The display uses the newest samples in chronological order and display-only autoscaling. The untouched ADC readings are preserved in the recording CSV. A separate DC-offset-corrected copy is used internally for event detection and speaker playback; no denoising or smoothing is applied to the recorded raw samples.

The **Analysis channel** selector controls the selected BPM and signal-level readouts: LEFT, CENTRE, RIGHT, or the arithmetic average of the three DC-corrected channels. The average signal has its own experimental detector; averaging microphones can attenuate signals when their phases differ.

The dashboard also includes **Heart**, **Lung**, and **Fetal Heart** data-collection views. Respiratory rate and fetal heart rate are not calculated. The CSV export includes elapsed time, ADC values, S1/S2 event counts, and available BPM estimates for each microphone and the average channel. Recording duration, frame count, and estimated file size are shown while recording.

BPM is estimated from successive detected S1-to-S1 intervals and remains blank until enough intervals are available. A stale estimate is cleared after signal loss. Detection is experimental and must be validated against reference recordings; it is not a clinical heart-rate measurement. S3/S4 are not automatically classified.

## Troubleshooting

- If Web Bluetooth is unavailable, use desktop Chrome or Edge on HTTPS and confirm the computer has BLE hardware.
- The chooser intentionally shows all nearby BLE devices so a missing/filtered advertised name does not hide the ESP32. Select the ESP32 device; the dashboard verifies its ElectroHeart service after connection.
- If no ESP32 device appears at all, use the BLE advertising diagnostic sketch below to determine whether the ESP32 or computer adapter is responsible.
- If the browser reports connected but the frame counter stays at zero, re-upload the latest firmware (16-byte notifications), then check the service/characteristic UUIDs and reset the ESP32.
- If ADC readings are stuck near 0 or 4095, check wiring, module output voltage, and potentiometer settings.
- If waveform amplitude is very small, adjust microphone placement/sensitivity and check the module's AO signal with an oscilloscope or ADC test.

## Safety

This is an educational/research prototype, not a medical device. Do not use its BPM or event labels for diagnosis or treatment decisions.
