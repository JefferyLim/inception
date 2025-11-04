kernel_text=0xffffffff81000000

# linked_text_base_hex was set above
linked_text_base_hex=$(readelf -S vmlinux | awk '/\.text/ {print "0x"$6}')

# compute slide
slide=$(( kernel_text - linked_text_base_hex ))
printf "KASLR slide = 0x%x\n" $slide

# gadget offsets from paper
PHANTOM=0x41db94
LEAK_BYTE=0x70c4a3-7
LEAK_PHYS=0xf22a41
LEAK_PHYSMAP=0xf22a48

# compute runtime addresses
phantom=$(( runtime_text + PHANTOM ))
leak_byte=$(( runtime_text + LEAK_BYTE ))
leak_phys=$(( runtime_text + LEAK_PHYS ))
leak_physmap=$(( runtime_text + LEAK_PHYSMAP ))

printf "phantom:      0x%016x\n" $phantom

# disassemble phantom
objdump -Mintel -d ~/vmlinux --adjust-vma=$slide \
      --start-address=$phantom --stop-address=$((phantom + 0x20))
printf "leak_byte:    0x%016x\n" $leak_byte

# disassemble leak_byte
objdump -Mintel -d ~/vmlinux --adjust-vma=$slide \
      --start-address=$leak_byte --stop-address=$((leak_byte + 0x20))

printf "leak_phys:    0x%016x\n" $leak_phys
# disassemble leak_phys
objdump -Mintel -d ~/vmlinux --adjust-vma=$slide \
      --start-address=$leak_phys --stop-address=$((leak_phys + 0x20))

printf "leak_physmap: 0x%016x\n" $leak_physmap
# disassemble leak_physmap
objdump -Mintel -d ~/vmlinux --adjust-vma=$slide \
      --start-address=$leak_physmap --stop-address=$((leak_physmap + 0x20))
