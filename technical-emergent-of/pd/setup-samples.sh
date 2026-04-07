#!/usr/bin/env bash
# Symlink Salamander WAVs to safe names (n0-n4 x v01-v16) for Pure Data.
# Pentatonic: C4, D#4, F#4, A4, C5
set -euo pipefail
SAL="${1:-$HOME/Desktop/SalamanderGrandPianoV3_48khz24bit/48khz24bit}"
PDW="$(cd "$(dirname "$0")" && pwd)/wav"
mkdir -p "$PDW"
notes=(C4 "D#4" "F#4" A4 C5)
for i in 0 1 2 3 4; do
	note="${notes[$i]}"
	for v in $(seq 1 16); do
		src="$SAL/${note}v${v}.wav"
		dst="$PDW/n${i}_v$(printf '%02d' "$v").wav"
		if [[ ! -f "$src" ]]; then
			echo "Missing: $src" >&2
			exit 1
		fi
		ln -sf "$src" "$dst"
	done
done
echo "Linked 80 files into $PDW"
