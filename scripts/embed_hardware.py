#!/usr/bin/env python3
"""Embed the optimized photo in one translation unit; no runtime file dependency."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
data=(root/'Assets/Hardware/hardware-base.jpg').read_bytes()
assert len(data)<512*1024 and data.startswith(b'\xff\xd8')
(root/'Source/GUI/HardwareArtwork.h').write_text('#pragma once\n// Generated from Assets/Hardware/hardware-base.jpg; decode once, share across editors.\nnamespace hardware {\ninline constexpr unsigned char data[] = {\n'+'\n'.join(','.join(str(x) for x in data[i:i+40])+',' for i in range(0,len(data),40))+'\n};\ninline constexpr int size=sizeof(data);\n}\n')
