#!/bin/bash
set -e

# GYP (conflicts with the installed version of gyp)
svn export -q http://gyp.googlecode.com/svn/trunk/ -r 1845 gyp
# ninja
wget https://github.com/martine/ninja/releases/download/v1.4.0/ninja-linux.zip
unzip -d /usr/local/bin/ ninja-linux.zip
# boost, g++-4.8, sqlite3
add-apt-repository ppa:ubuntu-toolchain-r/test -y
add-apt-repository ppa:boost-latest/ppa -y
apt-get update -qq
apt-get install libboost-filesystem1.55-dev libboost-test1.55-dev
apt-get install libsqlite3-dev
apt-get install -qq g++-4.8
