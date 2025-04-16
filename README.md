# ESP32 Bluetooth Speaker 🔊

## 📖 Overview
This project turns an **ESP32** into a Bluetooth speaker that can stream high-quality audio from any Bluetooth-capable device. It implements the **A2DP** (Advanced Audio Distribution Profile) for stereo audio streaming and **AVRCP** (Audio/Video Remote Control Profile) for media control and metadata. Audio is output through the ESP32's **I2S** interface to a digital-to-analog converter or digital amplifier, providing a true digital audio path for low-noise, high-quality sound.

In essence, the ESP32 acts as a wireless audio receiver: once the firmware is flashed, the device becomes discoverable as **"ESP32-SPEAKER"**. Pair your phone or PC to the ESP32-SPEAKER, and any music or audio played will be heard through the connected speakers. The project is built with the Espressif **ESP-IDF** framework in C, making it highly customizable and updatable.

## ✨ Features
- **🎵 Bluetooth A2DP Audio Streaming:** Receives stereo audio over Classic Bluetooth with high fidelity, suitable for music playback.
- **🎮 AVRCP Control & Metadata:** Supports AVRCP for remote control commands (play, pause, skip) and media information. The ESP32 can handle play/pause from your phone and retrieve track info (title, artist, etc.) if the source provides it.
- **🔊 I2S Digital Audio Output:** Uses the ESP32’s I2S interface to send audio data to an external DAC or I2S amplifier. This ensures a pure digital audio path for better sound quality. (Optionally, the ESP32’s internal DACs on GPIO25/GPIO26 can be used for basic analog output.)
- **📲 Easy Bluetooth Pairing:** Appears as **"ESP32-SPEAKER"** when in pairing mode. No PIN by default (Just Works pairing), making connection simple and quick.
- **⚙️ Configurable I2S Pins & Settings:** Default I2S pins are **GPIO27 (BCLK)**, **GPIO26 (LRCLK)**, and **GPIO25 (DOUT)**, but you can change them in the code or via menuconfig.
- **📝 Logging & Debug Info:** Comprehensive log output over serial (at 115200 baud) for debugging and monitoring. See connection status, AVRCP events, audio packet counts, and more in real-time.
- **📡 Extensible:** Built using ESP-IDF’s official Bluetooth stack, so it’s easy to extend with new features (e.g., adding Wi-Fi, voice assistant integration, or physical controls for volume and playback).

## 🛠️ Hardware Requirements
To build this project, you will need the following hardware:
- **ESP32 Dev Board:** Any ESP32 development board with Bluetooth Classic support (e.g., ESP32-WROOM-32 based boards). Ensure it has the standard dual-mode Bluetooth (most ESP32 except ESP32-S2/C3 will work).
- **I2S DAC or I2S Amplifier:** A module or chip that converts I2S audio to analog or directly drives a speaker. For example:
  - *DAC + Amp:* **MAX98357A** I2S Class-D amplifier breakout (outputs directly to a speaker, simple and fully digital).
  - *DAC only:* **PCM5102A** DAC module (outputs line-level analog audio, use with an external amplifier or powered speakers).
  - (Alternatively, you can use the ESP32’s **internal DACs** on GPIO25/GPIO26 with an analog amplifier, though with lower audio quality.)
- **Speaker:** A speaker or pair of speakers to output the sound. For MAX98357A or similar, a single 4Ω or 8Ω speaker can be directly driven. For a DAC like PCM5102, use powered speakers or an audio amplifier module connected to the DAC’s output.
- **Power Supply:** A 5V Micro-USB cable (to power the ESP32 dev board) or a suitable battery if making it portable. Ensure the I2S module is powered appropriately (3.3V or 5V as required by the module).
- **Connecting Wires:** Jumper wires to connect the ESP32 to the DAC/amp module (for I2S lines, power, and ground). A breadboard can help for prototyping.
- *(Optional)* **LEDs or Buttons:** Not required for core functionality, but you can add an LED for status indication (e.g., blinking when waiting for connection, solid when connected) or buttons to send AVRCP commands (like play/pause, volume) if you want to extend the project.

## 🔌 Wiring Guide
Connect the ESP32 to the I2S DAC or amplifier as shown below. The default GPIO pins used for I2S can be changed in software, but by default make sure to wire as follows:

| ESP32 GPIO      | I2S Signal        | DAC/Amplifier Pin      |
| --------------- | ----------------- | ---------------------- |
| **GPIO27**      | BCLK (Bit Clock)  | **BCK** or **SCK**     |
| **GPIO26**      | LRCK (Word Select)| **LRCK** or **WS**     |
| **GPIO25**      | DOUT (Data Out)   | **DIN** or **SD**      |
| **3.3V** (or 5V)| Power Supply      | **VCC** (module power) |
| **GND**         | Ground            | **GND**                |
| *(If using internal DAC)* | Left Audio Out | GPIO25 (DAC Out) to amplifier input |
| *(If using internal DAC)* | Right Audio Out| GPIO26 (DAC Out) to amplifier input |

**Notes:**
- Double-check the power requirements of your DAC/amp module. Many I2S DACs (like PCM5102) can be powered at 3.3V, while some breakouts have regulators to allow 5V. The MAX98357A amp can be powered with 3.3V or 5V (3.3V logic level compatible).
- Common I2S module pin labels: *BCK* (bit clock), *LRCK* or *WS* (word select or left-right clock), *DIN* or *SD* (data in). Connect these to the corresponding ESP32 GPIOs as above.
- Connect the speaker to the output of your amplifier or DAC module (e.g., the +/- speaker output of MAX98357A, or the L/R audio out pins of a DAC into an amplifier).
- If you choose to use the ESP32’s internal DAC output: you don’t need an external I2S DAC, but you **do** need an external analog amplifier. Connect GPIO25 and GPIO26 from the ESP32 to the left and right audio inputs of an analog amplifier. Keep in mind the internal DAC is 8-bit and will not sound as good as an external DAC solution.

## 💻 Software & Libraries
This project is implemented using the **Espressif IoT Development Framework (ESP-IDF)** in C. It leverages Espressif’s built-in Bluetooth stack and drivers:
- **ESP-IDF Bluetooth Stack (Bluedroid):** Provides the Classic Bluetooth A2DP and AVRCP profiles. We use the A2DP **Sink** role to receive audio, and AVRCP **Controller** role to receive notifications and potentially send control commands.
- **ESP-IDF I2S Driver:** Handles the I2S peripheral to output the audio stream to the DAC/amp. The I2S is configured for stereo 16-bit audio at the sampling rate provided by the source (e.g., typically 44.1 kHz for music).
- **FreeRTOS** (built into ESP-IDF): Used for creating tasks to handle Bluetooth events and I2S audio stream concurrently.
- **No additional external libraries** are required beyond the ESP-IDF. All functionality (Bluetooth, I2S, etc.) uses the native IDF APIs. Just make sure you have ESP-IDF properly installed and set up.

**Development Environment:**
- Developed and tested on **ESP-IDF (v4.x and v5.x)**.
- The project is configured for the **ESP32** target. (If using ESP-IDF 4+, you may need to run `idf.py set-target esp32` initially.)
- Project settings can be adjusted via `menuconfig` (for example, change the Bluetooth device name or I2S pins).

## 📂 Project Structure
ESP32-Bluetooth-Speaker/
├── main/
│   ├── main.c            # Main application: initializes BT, sets up I2S, event handlers
│   ├── bt_app_core.c     # Bluetooth core functions (BT stack initialization, task handling)
│   ├── bt_app_core.h
│   ├── bt_app_av.c       # Bluetooth A2DP/AVRCP event handling (audio data and media control)
│   ├── bt_app_av.h
│   └── (other source files if any, e.g., for util or configuration)
├── CMakeLists.txt        # Project build configuration for ESP-IDF
├── sdkconfig             # Configuration file (generated after running menuconfig)
└── README.md             # Project documentation (this file)


**Main components:**
- **`main.c`:** Entrypoint of the application. Initializes NVS, the Bluetooth controller, and the bluedroid stack. Configures the device name (`"ESP32-SPEAKER"`) and starts the A2DP sink and AVRCP controller. Sets up the I2S driver for audio data.
- **`bt_app_core.c` / `bt_app_core.h`:** Contains tasks and helper functions to initialize the Bluetooth stack and handle events.
- **`bt_app_av.c` / `bt_app_av.h`:** Implements audio data reception via A2DP and media control via AVRCP. Logs media information and handles AVRCP commands.
- *(Additional files)* may include utilities or configuration helpers.

## 🚀 Installation & Build Instructions
1. **Clone the repository:**
   ```sh
   git clone https://github.com/<your-username>/ESP32-Bluetooth-Speaker.git
   cd ESP32-Bluetooth-Speaker
🚀 Installation & Build Instructions
------------------------------------

### Set up ESP-IDF environment:

Plain textANTLR4BashCC#CSSCoffeeScriptCMakeDartDjangoDockerEJSErlangGitGoGraphQLGroovyHTMLJavaJavaScriptJSONJSXKotlinLaTeXLessLuaMakefileMarkdownMATLABMarkupObjective-CPerlPHPPowerShell.propertiesProtocol BuffersPythonRRubySass (Sass)Sass (Scss)SchemeSQLShellSwiftSVGTSXTypeScriptWebAssemblyYAMLXML`   . ~/esp/esp-idf/export.sh   `

_(Refer to the_ [_ESP-IDF Getting Started Guide_](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) _for detailed setup instructions.)_

### Configure the project (optional):

Plain textANTLR4BashCC#CSSCoffeeScriptCMakeDartDjangoDockerEJSErlangGitGoGraphQLGroovyHTMLJavaJavaScriptJSONJSXKotlinLaTeXLessLuaMakefileMarkdownMATLABMarkupObjective-CPerlPHPPowerShell.propertiesProtocol BuffersPythonRRubySass (Sass)Sass (Scss)SchemeSQLShellSwiftSVGTSXTypeScriptWebAssemblyYAMLXML`   idf.py menuconfig   `

Modify settings such as the Bluetooth device name, internal DAC vs external I2S DAC, and I2S pin numbers if needed.

### Build the firmware:

Plain textANTLR4BashCC#CSSCoffeeScriptCMakeDartDjangoDockerEJSErlangGitGoGraphQLGroovyHTMLJavaJavaScriptJSONJSXKotlinLaTeXLessLuaMakefileMarkdownMATLABMarkupObjective-CPerlPHPPowerShell.propertiesProtocol BuffersPythonRRubySass (Sass)Sass (Scss)SchemeSQLShellSwiftSVGTSXTypeScriptWebAssemblyYAMLXML`   idf.py set-target esp32    # (Only needed once)  idf.py build   `

### Flash to the ESP32:

Plain textANTLR4BashCC#CSSCoffeeScriptCMakeDartDjangoDockerEJSErlangGitGoGraphQLGroovyHTMLJavaJavaScriptJSONJSXKotlinLaTeXLessLuaMakefileMarkdownMATLABMarkupObjective-CPerlPHPPowerShell.propertiesProtocol BuffersPythonRRubySass (Sass)Sass (Scss)SchemeSQLShellSwiftSVGTSXTypeScriptWebAssemblyYAMLXML`   idf.py -p /dev/ttyUSB0 flash   # Replace with your serial port   `

### Monitor the serial output (optional but recommended):

Plain textANTLR4BashCC#CSSCoffeeScriptCMakeDartDjangoDockerEJSErlangGitGoGraphQLGroovyHTMLJavaJavaScriptJSONJSXKotlinLaTeXLessLuaMakefileMarkdownMATLABMarkupObjective-CPerlPHPPowerShell.propertiesProtocol BuffersPythonRRubySass (Sass)Sass (Scss)SchemeSQLShellSwiftSVGTSXTypeScriptWebAssemblyYAMLXML`   idf.py -p /dev/ttyUSB0 monitor   `

_Tip: You can combine flash & monitor:_

Plain textANTLR4BashCC#CSSCoffeeScriptCMakeDartDjangoDockerEJSErlangGitGoGraphQLGroovyHTMLJavaJavaScriptJSONJSXKotlinLaTeXLessLuaMakefileMarkdownMATLABMarkupObjective-CPerlPHPPowerShell.propertiesProtocol BuffersPythonRRubySass (Sass)Sass (Scss)SchemeSQLShellSwiftSVGTSXTypeScriptWebAssemblyYAMLXML`   idf.py -p /dev/ttyUSB0 flash monitor   `

After flashing, the ESP32 will reboot and start the Bluetooth speaker application automatically.

▶️ Usage (Step-by-Step)
-----------------------

*   **Power on the ESP32 + Speaker:** Connect the ESP32 to a power source. Ensure the I2S DAC/amplifier and speaker are properly connected.
    
*   **Pair with your Bluetooth device:** Open Bluetooth settings on your phone or computer, and select **ESP32-SPEAKER** from the list. No PIN is required by default.
    
*   **Connect and play audio:** Once connected, any audio played on your device will stream to the ESP32 and output via the I2S interface.
    
*   **Adjust volume / playback:** Use your device’s controls to change volume or playback state. AVRCP commands will be handled by the ESP32.
    
*   **Test and verify:** Monitor the serial output for log messages indicating connection status, audio packets, and AVRCP events.
    

📝 Logging & Notifications
--------------------------

**Serial monitor output example:**

Plain textANTLR4BashCC#CSSCoffeeScriptCMakeDartDjangoDockerEJSErlangGitGoGraphQLGroovyHTMLJavaJavaScriptJSONJSXKotlinLaTeXLessLuaMakefileMarkdownMATLABMarkupObjective-CPerlPHPPowerShell.propertiesProtocol BuffersPythonRRubySass (Sass)Sass (Scss)SchemeSQLShellSwiftSVGTSXTypeScriptWebAssemblyYAMLXML`   I (3010) BT_INIT: Bluetooth initialized, device name set to "ESP32-SPEAKER"  I (5020) BT_AV: A2DP connection state: Connected, [24:6F:28:3A:BC:5E]  I (6050) BT_AV: Audio packet count 100  I (7000) BT_AV: Now playing: "Song Title" by Artist Name  I (8000) BT_AV: Volume set to 80%   `

Logs display connection status, audio packet counts, track info, and volume changes. Useful for debugging and ensuring the system is working as expected.

🤝 Contributing
---------------

*   Fork the repository and create a pull request with your changes.
    
*   Document any new features and test thoroughly.
    
*   Open issues for bugs or suggestions with detailed reproduction steps and logs.
    

📜 License
----------

This project is licensed under the **MIT License**.See the [LICENSE](https://chatgpt.com/c/LICENSE) file for the full license text.

✉️ Contact
----------

*   **Maintainer:** \[Your Name\]**Email:** [youremail@example.com](mailto:youremail@example.com)
    
*   Alternatively, open an issue on GitHub or connect via [LinkedIn](https://www.linkedin.com/) or [Twitter](https://www.twitter.com/)
    

Enjoy building and using the ESP32 Bluetooth Speaker!**Happy hacking!** 🔊🎶