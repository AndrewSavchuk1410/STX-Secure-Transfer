import os
import subprocess
import hashlib
import time

SCRIPT_DIR   = os.path.dirname(os.path.abspath(__file__))  
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, os.pardir))
BUILD_DIR    = os.path.join(PROJECT_ROOT, "build")
KEY_DIR      = os.path.join(PROJECT_ROOT, "keys")

RECV_DIR = os.path.join(SCRIPT_DIR, "received_test_files")
os.makedirs(RECV_DIR, exist_ok=True)

RECV_EXE = os.path.join(BUILD_DIR, "stx-recv", "Debug", "stx-recv.exe")
SEND_EXE = os.path.join(BUILD_DIR, "stx-send", "Debug", "stx-send.exe")
RECV_EXE_DIR = os.path.dirname(RECV_EXE)
SEND_EXE_DIR = os.path.dirname(SEND_EXE)

SEND_KEY = os.path.join(KEY_DIR, "send_priv.pem")

FILE_ORIG = os.path.join(SCRIPT_DIR, "original.bin")
FILE_RECV = os.path.join(RECV_DIR, os.path.basename(FILE_ORIG))

def sha256sum(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while chunk := f.read(8192):
            h.update(chunk)
    return h.hexdigest()

def generate_test_file(path, size_mb):
    with open(path, "wb") as f:
        f.write(os.urandom(size_mb * 1024 * 1024))

def cleanup():
    for p in (FILE_ORIG, FILE_RECV):
        try: os.remove(p)
        except OSError: pass

def run_standard_transfer():
    print("[TEST] Standard 5MB transfer")
    generate_test_file(FILE_ORIG, 5)

    recv = subprocess.Popen(
        [RECV_EXE, "--listen", "9000", "--out", RECV_DIR],
        cwd=RECV_EXE_DIR
    )
    time.sleep(1)

    subprocess.run([
        SEND_EXE,
        "127.0.0.1", "9000", FILE_ORIG,
        "--key", SEND_KEY
    ], check=True, cwd=SEND_EXE_DIR)

    time.sleep(1)
    recv.terminate()
    recv.wait()
    time.sleep(1)

    match = sha256sum(FILE_ORIG) == sha256sum(FILE_RECV)
    print("[PASS]" if match else "[FAIL]",
          "Files match" if match else "Mismatch in files")
    return match

def run_resume_transfer():
    print("[TEST] Simulated disconnection/resume")
    generate_test_file(FILE_ORIG, 5)

    recv = subprocess.Popen(
        [RECV_EXE, "--listen", "9000", "--out", RECV_DIR],
        cwd=RECV_EXE_DIR
    )
    time.sleep(1)

    send = subprocess.Popen([
        SEND_EXE,
        "127.0.0.1", "9000", FILE_ORIG,
        "--key", SEND_KEY
    ], cwd=SEND_EXE_DIR)
    time.sleep(2)
    send.kill(); send.wait()
    print("[INFO] Sender killed mid-transfer")

    recv = subprocess.Popen(
        [RECV_EXE, "--listen", "9000", "--out", RECV_DIR],
        cwd=RECV_EXE_DIR
    )
    time.sleep(1)

    subprocess.run([
        SEND_EXE,
        "127.0.0.1", "9000", FILE_ORIG,
        "--key", SEND_KEY
    ], check=True, cwd=SEND_EXE_DIR)
    time.sleep(1)

    recv.terminate()
    recv.wait()
    time.sleep(1)

    match = sha256sum(FILE_ORIG) == sha256sum(FILE_RECV)
    print("[PASS]" if match else "[FAIL]",
          "Files match after resume" if match else "Mismatch after resume")
    return match

def main():
    cleanup()
    results = [run_standard_transfer()]
    cleanup()

    if all(results):
        print("\nAll integration tests passed.")
    else:
        print("\nSome integration tests failed.")
        exit(1)

if __name__ == "__main__":
    main()
