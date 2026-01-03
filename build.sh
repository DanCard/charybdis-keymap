#!/bin/bash
# Charybdis 4x6 build script
# Using Elite-Pi (RP2040) converter

qmk compile -kb bastardkb/charybdis/4x6/elitec -km dcar -e CONVERT_TO=elite_pi "$@"
