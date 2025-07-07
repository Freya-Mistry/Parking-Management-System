# Arduino Parking Management System (Dec 2023)

## Overview

This project implements a **Parking Management System** on an Arduino, which receives and processes vehicle-related messages via the Serial Interface. It displays parking data on an LCD and allows interaction through button inputs. This system is fully compliant with the coursework specification and uses a **Finite State Machine (FSM)** for managing system states and transitions.

## Features

- Vehicle entry and payment logging via Serial commands.
- LCD display showing vehicle information:
  - Registration number
  - Parking location (up to 7 visible chars)
  - Vehicle type
  - Payment status (PD/NPD)
  - Entry and Exit timestamps
- LCD backlight changes color based on payment status:
  - **Yellow** for unpaid (NPD)
  - **Green** for paid (PD)
- Navigation through vehicle list using UP/DOWN buttons.
- Select button (held >1s) shows student ID and clears screen (purple backlight), while continuing to process serial input.
- Synchronisation phase with host (using 'Q' and 'X' handshake).
- Strict validation of incoming messages according to custom protocol.
- Error handling and debug outputs via Serial monitor.

## Setup & Usage

### Hardware Requirements

- Arduino Uno/Nano
- 16x2 LCD Display with RGB backlight
- Push buttons (UP, DOWN, SELECT)
- Serial interface (via USB or Bluetooth)

### Uploading the Code

1. Open the Arduino IDE.
2. Connect your Arduino to your computer.
3. Load the sketch (source file) containing your code.
4. Select the correct board and port under **Tools > Board / Port**.
5. Upload the code.

### Communication Protocol

#### Synchronisation Phase

- Arduino sends `Q` every second (no newline).
- Waits for `X` response to proceed to main phase.

#### Command Format

All messages are newline-terminated and follow a strict format:

- **Add Vehicle:** `A-REGNUM-TYPE-LOCATION`
- **Set Payment Status:** `S-REGNUM-[PD|NPD]`
- **Change Vehicle Type:** `T-REGNUM-TYPE`
- **Change Location:** `L-REGNUM-LOCATION`
- **Remove Vehicle:** `R-REGNUM`

#### Example Commands

```text
A-GR04XFB-C-GranbySt.     # Add vehicle
S-GR04XFB-PD              # Set as Paid
T-GR04XFB-B               # Change type to Bus
L-GR04XFB-BrownsLane      # Change location
R-GR04XFB                 # Remove if Paid
```

#### LCD Display Format
```
+----------------+
|^ABCDEFG HIJKLMN|
|vS TUV HHMM HHMM|
+----------------+
```

## Error Handling
Errors are sent to the serial monitor as: `ERROR: <description>`

## Notes
- Maximum number of vehicles is limited to avoid SRAM exhaustion
- Serial inputs are processed even during ID display screen
- Input is strictly parsed, all format violations raise errors

## Documentation
Addition documents include a FSM diagram, debugging process explanation and code reflection and limitations
