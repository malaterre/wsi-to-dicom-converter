# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script updates environment and build wsi2dcm by steps:
# 1: update of ubuntu deb repos
# 2: install of tools and libs for build
# 3: install libjpeg turbo
# 4: install opencv
# 5: install abseil
# 6: install dcmtk
# 7: download and unpack source code of libs for build
# 8: build

#1
echo "deb  http://old-releases.ubuntu.com cosmic universe" | tee -a /etc/apt/sources.list
apt-get update
#2
DEBIAN_FRONTEND="noninteractive" apt-get install wget libtiff-dev unzip build-essential libjsoncpp-dev libgdk-pixbuf2.0-dev libcairo2-dev libsqlite3-dev cmake libglib2.0-dev libxml2-dev libopenjp2-7-dev g++-8 libgtest-dev -y
ls -l
#3
# installing in /workspace
wget -O libjpeg_turbo.zip https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.2.zip
unzip libjpeg_turbo.zip
rm libjpeg_turbo.zip
mkdir -p ./libjpeg-turbo-2.1.2/build
cd ./libjpeg-turbo-2.1.2/build
cmake -G"Unix Makefiles" ..
make -j12
make install
cd ..
cd ..
#4
wget -O opencv.zip https://github.com/opencv/opencv/archive/refs/tags/4.5.4.zip > /dev/null
unzip opencv.zip  > /dev/null
rm opencv.zip
mkdir -p ./opencv-4.5.4/build
cd ./opencv-4.5.4/build
cmake .. -DBUILD_LIST=core,imgproc,imgcodecs
make -j12
make install
cd ..
cd ..
#5
wget -O abseil.zip https://github.com/abseil/abseil-cpp/archive/refs/tags/20211102.0.zip > /dev/null
unzip abseil.zip > /dev/null
rm abseil.zip
mkdir -p ./abseil-cpp-20211102.0/build
cd ./abseil-cpp-20211102.0/build
cmake ..  -DCMAKE_INSTALL_PREFIX=/abseil/install
cmake  --build . --target install
cd ..
cd ..
#6
wget -O dcmtk-3.6.6.zip https://github.com/DCMTK/dcmtk/archive/refs/tags/DCMTK-3.6.6.zip > /dev/null
unzip dcmtk-3.6.6.zip > /dev/null
rm dcmtk-3.6.6.zip
mkdir -p ./dcmtk-DCMTK-3.6.6/build
cd ./dcmtk-DCMTK-3.6.6/build
cmake -DDCMTK_FORCE_FPIC_ON_UNIX:BOOL=TRUE -DDCMTK_ENABLE_CXX11:BOOL=TRUE -DDCMTK_ENABLE_CHARSET_CONVERSION:BOOL=FALSE ..
make -j12
make DESTDIR=/dcmtk install
export DCMDICTPATH=/dcmtk/usr/local/share/dcmtk/dicom.dic
export PATH=/dcmtk/usr/local/bin:$PATH
cd ..
cd ..
#7
cp /usr/lib/x86_64-linux-gnu/glib-2.0/include/glibconfig.h /usr/include/glib-2.0/glibconfig.h
mkdir build
cd build
wget https://github.com/uclouvain/openjpeg/archive/v2.3.0.zip > /dev/null
unzip v2.3.0.zip  > /dev/null
rm v2.3.0.zip
wget https://boostorg.jfrog.io/artifactory/main/release/1.69.0/source/boost_1_69_0.tar.gz  > /dev/null
tar xvzf boost_1_69_0.tar.gz  > /dev/null
rm boost_1_69_0.tar.gz
wget https://github.com/open-source-parsers/jsoncpp/archive/0.10.7.zip > /dev/null
unzip 0.10.7.zip > /dev/null
rm 0.10.7.zip
wget https://github.com/openslide/openslide/releases/download/v3.4.1/openslide-3.4.1.tar.gz > /dev/null
tar xvzf openslide-3.4.1.tar.gz  > /dev/null
rm openslide-3.4.1.tar.gz
#remove documentation to remove flagged javascript security issues.
set +e
find /workspace/build -path "*/doc/html" -type d -exec rm -rf {} \;
find /workspace/build -path "*/doc/*" -type f -name "*.js" -exec rm -f {} \;
set -e
#8
cmake -DSTATIC_BUILD=ON -DTESTS_BUILD=ON ..
make -j12
./gTests
