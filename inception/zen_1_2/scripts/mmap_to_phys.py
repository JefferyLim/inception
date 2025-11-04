#!/usr/bin/env python3
import os
import struct
import sys

PAGE_SIZE = 4096
PAGEMAP_ENTRY_BYTES = 8
PFN_MASK = (1 << 55) - 1  # bits 0-54
PHYS_OFFSET = 0xffff91edc0000000  # adjust for your system (common for x86_64)

def virt_to_pfn(pid, vaddr):
    """Read the PFN for a virtual address from /proc/<pid>/pagemap."""
    pagemap_path = f"/proc/{pid}/pagemap"
    vpn = vaddr // PAGE_SIZE
    offset = vpn * PAGEMAP_ENTRY_BYTES
    with open(pagemap_path, "rb") as f:
        f.seek(offset)
        entry_bytes = f.read(8)
        if len(entry_bytes) < 8:
            return None
        entry = struct.unpack("Q", entry_bytes)[0]
        present = (entry >> 63) & 1
        swapped = (entry >> 62) & 1
        pfn = entry & PFN_MASK
        if present and not swapped:
            return pfn
        return None

def main():
    if len(sys.argv) < 4:
        print(f"Usage: sudo {sys.argv[0]} <pid> <vaddr> <length>")
        sys.exit(1)

    pid = int(sys.argv[1])
    start_vaddr = int(sys.argv[2], 16)
    length = int(sys.argv[3])
    num_pages = (length + PAGE_SIZE - 1) // PAGE_SIZE

    print(f"{'VA':>18} {'PFN':>10} {'PhysAddr':>14} {'Physmap':>18}")
    print("=" * 65)

    for i in range(num_pages):
        vaddr = start_vaddr + i * PAGE_SIZE
        pfn = virt_to_pfn(pid, vaddr)
        if pfn is not None:
            phys = pfn * PAGE_SIZE
            physmap = PHYS_OFFSET + phys
            print(f"0x{vaddr:016x}  0x{pfn:08x}  0x{phys:012x}  0x{physmap:016x}")
        else:
            print(f"0x{vaddr:016x}  {'(not present)':>40}")

if __name__ == "__main__":
    main()
