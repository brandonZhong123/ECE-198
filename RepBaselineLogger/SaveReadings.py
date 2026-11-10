import argparse
import fcntl
import os
from pathlib import Path
import re
import select
import struct
import sys
import termios
import time
import tty
from datetime import datetime

HEADER = b"trial,label,sample,t_ms,x_raw,y_raw,z_raw,marker"
READING = re.compile(rb"[0-9]+,[rgpn](?:,[0-9]+){6}")


def configure_port(serial_fd):
    fcntl.ioctl(serial_fd, termios.TIOCEXCL)
    tty.setraw(serial_fd)
    settings = termios.tcgetattr(serial_fd)
    settings[2] &= ~(termios.CSTOPB | termios.CRTSCTS)
    settings[2] |= termios.CLOCAL | termios.CREAD
    settings[4] = settings[5] = termios.B115200
    termios.tcsetattr(serial_fd, termios.TCSANOW, settings)
    # The UNO R4 may wait for DTR before sending serial data.
    fcntl.ioctl(serial_fd, termios.TIOCMBIS, struct.pack("i", termios.TIOCM_DTR))


def capture(serial_fd, output):
    keyboard = sys.stdin.fileno()
    saved_terminal = termios.tcgetattr(keyboard)
    pending = b""
    rows = 0
    drain_until = None
    try:
        tty.setcbreak(keyboard)
        time.sleep(2)  # Opening the port can reset the board.
        os.write(serial_fd, b"sh")
        print("Ready. Press r/g/p/n to start, s to stop, m to mark, or q to save and quit.")
        print("No Enter needed. Press h to request the header.")
        next_report = time.monotonic() + 2
        while drain_until is None or time.monotonic() < drain_until:
            inputs = [keyboard, serial_fd] if drain_until is None else [serial_fd]
            ready, _, _ = select.select(inputs, [], [], 0.05)
            if keyboard in ready:
                key = os.read(keyboard, 1).lower()
                if key in (b"q", b""):
                    os.write(serial_fd, b"s")
                    drain_until = time.monotonic() + 0.5
                elif key in b"rgpnsmh":
                    os.write(serial_fd, key)
                    print(f"Sent: {key.decode()}")
            if serial_fd in ready:
                try:
                    chunk = os.read(serial_fd, 4096)
                except BlockingIOError:
                    continue
                if not chunk:
                    raise OSError("Arduino serial port disconnected.")
                *lines, pending = (pending + chunk).split(b"\n")
                for line in lines:
                    line = line.rstrip(b"\r")
                    if READING.fullmatch(line):
                        output.write(line.decode("ascii") + "\n")
                        rows += 1
            if time.monotonic() >= next_report:
                print(f"Saved {rows} readings")
                next_report = time.monotonic() + 2
    finally:
        termios.tcsetattr(keyboard, termios.TCSANOW, saved_terminal)
        if drain_until is None:
            try:
                os.write(serial_fd, b"s")
            except OSError:
                pass
    return rows


def main():
    parser = argparse.ArgumentParser(description="Save Arduino baseline readings to CSV.")
    parser.add_argument("port", nargs="?", help="Serial device, e.g. /dev/cu.usbmodem1101")
    args = parser.parse_args()
    if not sys.stdin.isatty():
        parser.error("Run the logger in Terminal to use its command keys.")

    port = args.port
    if port is None:
        ports = sorted(
            device for device in Path("/dev").glob("cu.*")
            if device.name.startswith(("cu.usbmodem", "cu.usbserial", "cu.wchusbserial"))
        )
        if len(ports) != 1:
            available = ", ".join(map(str, ports)) or "none found"
            parser.error(f"Connect one Arduino or pass its Tools > Port device. Detected: {available}")
        port = ports[0]

    try:
        serial_fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        try:
            configure_port(serial_fd)
            capture_dir = Path(__file__).resolve().parent / "captures"
            capture_dir.mkdir(exist_ok=True)
            timestamp = datetime.now().strftime("%Y%m%d-%H%M%S-%f")[:-3]
            output_path = capture_dir / f"readings-{timestamp}.csv"
            with output_path.open("x", encoding="ascii", buffering=1) as output:
                output.write(HEADER.decode("ascii") + "\n")
                print(f"Connected to {port} at 115200 baud.\nSaving to: {output_path}")
                rows = capture(serial_fd, output)
            print(f"Finished: {rows} readings saved to {output_path}")
        finally:
            os.close(serial_fd)
    except KeyboardInterrupt:
        print("\nLogger stopped. Any readings already written remain in the CSV.")
        return 130
    except OSError as error:
        print(f"Logger stopped: {error}", file=sys.stderr)
        print("Check the USB connection and close Serial Monitor and Plotter before retrying.",
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
