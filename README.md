Contributions:

- `phantomcall/` - Utilizing the author's code as a base, experimentally tested and verified the results of the paper. Many of the gadgets they utilized were difficult to recreate, and their code was necessary to properly recreate the work
- `inception/scripts` - scripts utilized to gather metrics, find kernel gadgets, and determine location of mmaps/root hash/kernel modules
    - [Script README](inception/zen_1_2/README.md)
- `inception/kmod` - updated the kernel module to accept custom input for demo purposes (so that a student can input a secret that our script will guess)
- `inception/zen_1_2` - Code that the paper's author provided in their GitHub respository that we used 
