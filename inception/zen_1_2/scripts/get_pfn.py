#!/usr/bin/env python3
import sys, os, struct

if len(sys.argv) != 4:
    print("Usage: sudo ./get_pfn.py <PID> <VADDR_hex> <PHYSMAP>")
    sys.exit(1)

pid = int(sys.argv[1])
vaddr = int(sys.argv[2], 16)
PAGE_SIZE = os.sysconf("SC_PAGE_SIZE")
pagemap_path = f"/proc/{pid}/pagemap"

# Current physmap base
physmap_base = 0xffff91edc0000000

index = (vaddr // PAGE_SIZE) * 8

with open(pagemap_path, "rb") as f:
    f.seek(index)
    entry_bytes = f.read(8)
    if len(entry_bytes) != 8:
        print("Failed to read pagemap entry")
        sys.exit(1)
    entry = struct.unpack("Q", entry_bytes)[0]

present = (entry >> 63) & 1
pfn = entry & ((1 << 55) - 1)

print(f"pid={pid} vaddr=0x{vaddr:x} page_index={vaddr // PAGE_SIZE}")
print(f"pagemap entry=0x{entry:016x}")
print(f"present={present}")

if present:
    phys = pfn * PAGE_SIZE
    print(f"PFN = 0x{pfn:x}  physical addr = 0x{phys:x}")
    # adjust this physmap_base to your kernel's base if known
    print(f"physmap addr = 0x{physmap_base + phys:x}  (assumes physmap_base=0x{physmap_base:x})")
else:
    print("Page not present in RAM (present bit == 0).")

