# ElectroHeart

Three-channel ESP32 phonocardiogram (PCG) acquisition dashboard.

## Hardware
- Classic ESP32 / ESP32-WROOM (Bluetooth Classic SPP required)
- Microphone analog outputs to ADC1 GPIO34 (left), GPIO35 (centre), GPIO32 (right)
- Keep every analog output within 0–3.3 V. Use suitable biasing/conditioning for AC microphone signals.

## Firmware and packet protocol
Open `firmware/ElectroHeart_3CH.ino` in Arduino IDE and select a classic ESP32 board such as **ESP32 Dev Module**.
- Sample rate: 2000 frames/s
- USB serial: 921600 baud
- Bluetooth device: `ESP32-ElectroHeart`
- Each frame is 8 bytes: `A5 5A`, then MIC1, MIC2, MIC3 as unsigned 16-bit little-endian ADC values.
- Do not add Serial.print debug text while binary streaming.

## Website
Open [ElectroHeart](https://sukritmodak.github.io/ElectroHeart/) in desktop Chrome or Edge.
- USB: Connect USB and choose the ESP32 COM port.
- Bluetooth Classic on Windows: pair `ESP32-ElectroHeart`, identify its outgoing COM port, install bridge dependencies, and run the bridge below. Then choose Connect Bluetooth in the website.

## Bluetooth bridge
From the `bridge` directory:
```powershell
py -m pip install pyserial websockets
py heart_bridge.py COM7
```
Replace COM7 with the Bluetooth outgoing COM port shown by Windows. Keep the bridge running while using the browser.

## Dashboard features
- Three independently displayed live ADC waveforms and raw ADC values
- Per-channel relative dBFS level
- Experimental S1/S2 event labelling and S1-to-S1 BPM estimate per channel
- Consensus BPM from channels with valid estimates
- Live playback of incoming microphone samples
- Pause/resume waveform display
- CSV recording and export

**Research prototype only:** acoustic event detection and BPM are experimental, not clinically validated. Do not use for diagnosis or treatment decisions. S3/S4 are intentionally not auto-labelled without a validated detector.