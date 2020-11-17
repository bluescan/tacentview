It is important that when you run snapcraft you are in the Linux/snapcraft directory, not the snap directory.


If you ever need to modify a .snap, it is simply a squashfs and can be modified using the below commands:

mkdir mnt
sudo mount -t squashfs -o ro ./tacentview_1.0.16_amd64.snap ./mnt/
sudo cp -r mnt/ mnt2/
sudo umount mnt
mv tacentview_1.0.16_amd64.snap tacentview_1.0.16_amd64.snapZZZ
cd mnt2/meta
sudo mkdir gui
cd gui
sudo cp ../../../gui/* .
cd back to snap dir
mksquashfs mnt2/ tacentview_1.0.16_amd64.snap -noappend -comp xz -all-root -no-xattrs -no-fragments
# CAN ALSO TRY: snapcraft pack <DIR>

