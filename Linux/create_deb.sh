#!/bin/bash
cp ../TacitTexView tacittexview_1.0-7/usr/bin/tacittexview
sudo dpkg-deb --build tacittexview_1.0-7
sleep 2s
sudo dpkg -r tacittexview
sleep 2s
sudo dpkg -i tacittexview_1.0-7.deb

