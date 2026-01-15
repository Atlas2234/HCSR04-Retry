import subprocess
import time

N = 600  # how many times
DELAY = 0 # seconds between runs

for i in range(N):
    r = subprocess.run(["./test"], capture_output=True, text=True)
    if r.returncode == 0:
        print(r.stdout.strip())
    else:
        print(f"run {i}: error (code {r.returncode})")
        if r.stderr:
            print(r.stderr.strip())
    time.sleep(DELAY)
