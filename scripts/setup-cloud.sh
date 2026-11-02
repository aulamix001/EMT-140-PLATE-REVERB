#!/usr/bin/env bash
set -euo pipefail
# Debian 13 non-root cloud setup. Retain HTTPS proxy and CA trust.
project_dir=/workspace/EMT-140-PLATE-REVERB
tools_dir=/workspace/tools
mkdir -p "$tools_dir"/debs "$tools_dir"/sysroot "$tools_dir"/apt/lists/partial "$tools_dir"/apt/cache/archives/partial "$tools_dir"/apt/empty
python -m pip install --target "$tools_dir/python" --upgrade cmake==4.4.4 ninja==1.13.2
cat > "$tools_dir/apt/sources.list" <<'SOURCES'
deb [signed-by=/usr/share/keyrings/debian-archive-keyring.gpg] https://deb.debian.org/debian trixie main
SOURCES
cat > "$tools_dir/apt/apt.conf" <<'APTCONF'
Dir::Etc::parts "/workspace/tools/apt/empty";
Dir::Etc::sourcelist "/workspace/tools/apt/sources.list";
Dir::Etc::sourceparts "/workspace/tools/apt/empty";
Dir::State::lists "/workspace/tools/apt/lists";
Dir::Cache "/workspace/tools/apt/cache";
APT::Get::List-Cleanup "false";
Acquire::https::Proxy "http://proxy:8080";
APTCONF
export APT_CONFIG="$tools_dir/apt/apt.conf"
apt-get update
cd "$tools_dir/debs"
packages=(libasound2-dev libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev libxrender-dev libxfixes-dev libfreetype-dev libfontconfig-dev libpng-dev libbrotli-dev libbz2-dev zlib1g-dev x11proto-dev xorg-sgml-doctools xtrans-dev xvfb xserver-common libxfont2 libgl-dev libglx-dev libice-dev libsm-dev libxcb1-dev libxau-dev libxdmcp-dev)
for i in "${!packages[@]}"; do packages[$i]="${packages[$i]}/trixie"; done
apt-get download "${packages[@]}"
for f in ./*.deb; do dpkg-deb -x "$f" "$tools_dir/sysroot"; done
python - <<'PY'
from pathlib import Path
p=Path('/workspace/tools/sysroot/usr/lib/x86_64-linux-gnu')
for f in p.glob('*.so'):
 if f.is_symlink() and not f.exists():
  for candidate in [Path('/usr/lib/x86_64-linux-gnu')/f.readlink().name,Path('/usr/lib/x86_64-linux-gnu')/f.name]:
   if candidate.exists():f.unlink();f.symlink_to(candidate);break
PY
mkdir -p /workspace/dependencies
if [ ! -d /workspace/dependencies/JUCE/.git ]; then
 git clone --depth 1 --branch 8.0.6 https://github.com/juce-framework/JUCE.git /workspace/dependencies/JUCE
fi
test "$(git -C /workspace/dependencies/JUCE rev-parse HEAD)" = 51a8a6d7aeae7326956d747737ccf1575e61e209
cat > "$tools_dir/build-env.sh" <<'BUILDENV'
export PATH=/workspace/tools/python/cmake/data/bin:/workspace/tools/python/bin:$PATH
export PKG_CONFIG_PATH=/workspace/tools/sysroot/usr/lib/x86_64-linux-gnu/pkgconfig:/workspace/tools/sysroot/usr/share/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=/workspace/tools/sysroot
export CPLUS_INCLUDE_PATH=/workspace/tools/sysroot/usr/include:/workspace/tools/sysroot/usr/include/freetype2:/workspace/tools/sysroot/usr/include/x86_64-linux-gnu
export C_INCLUDE_PATH=$CPLUS_INCLUDE_PATH
export LIBRARY_PATH=/workspace/tools/sysroot/usr/lib/x86_64-linux-gnu
export LD_LIBRARY_PATH=/workspace/tools/sysroot/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}
BUILDENV
printf 'Dependencies prepared. Source /workspace/tools/build-env.sh before CMake.\n'
