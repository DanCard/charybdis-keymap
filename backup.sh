#!/bin/bash
# Backup keymap files from qmk_firmware to version-controlled keymap/ directory

SRC="qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar"
DEST="keymap"

cp "$SRC"/* "$DEST"/
git add "$DEST"/
git status
