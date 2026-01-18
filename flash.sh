#!/bin/bash
# Flash RP2040 keyboard firmware
# Searches for mounted RP2040 bootloader by checking for INFO_UF2.TXT

UF2="/home/dcar/projects/mech-keyboard/qmk_firmware/bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2"

# Search common mount locations for RP2040 bootloader
for dir in /mnt/* /media/$USER/* /run/media/$USER/*; do
    if [ -f "$dir/INFO_UF2.TXT" ]; then
        echo "Found RP2040 bootloader at: $dir"
        cat "$dir/INFO_UF2.TXT"
        echo ""
        echo "Copying firmware..."
        cp "$UF2" "$dir/" && sync
        echo "Done! Keyboard should reboot."
        exit 0
    fi
done

echo "No RP2040 bootloader found."
echo "1. Put keyboard in bootloader mode (double-tap reset)"
echo "2. Mount it somewhere under /mnt/ or /media/"
echo "3. Run this script again"
exit 1
