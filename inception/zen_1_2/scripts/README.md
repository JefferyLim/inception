Descriptions of scripts in this folder

- `search.py`: Code used to search through memory starting at a specific address. Uses all 6 cores to search (incrementing each core at a different address)

- `find_kernel_gadget.sh`: Search the vmlinux for the kernel gadgets the paper used

To gather metrics to compare to paper:
- `run_inception_stats.sh`: Run inception attack on the kmod example 50x and tee to a file
- `inception_results.log`: 50 run example used to gather the metrics
- `analyze_results.py`: Python script used to gather metrics from the log file 

To try and see if we can read `/etc/shadow`. We were trying to determine where it would be in memory
- `get_pfn.py`: Helper script that helped us get the physmap address of a virtual address of a PID
- `mmap_to_phys.py`: Helper script that grabbed the virtual address of a PID (from mmap for example)
 
