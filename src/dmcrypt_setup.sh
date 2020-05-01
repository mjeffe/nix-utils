# This is how I did it.  If you rerun these commands, it will BLOW AWAY 
# any data currently on these partitions. 

# see /usr/local/bin/mount_encrypted_filesystems.sh

exit

modprobe twofish

cryptsetup remove c02
cryptsetup -y -c twofish-cbc-essiv:sha256 create c02 /dev/sdc1
mkfs.ext3 -b 4096 -N 100000 -m 0 /dev/mapper/c02
mkdir -p /c02
mount /dev/mapper/c02 /c02

cryptsetup remove c01
cryptsetup -y -c twofish-cbc-essiv:sha256 create c01 /dev/sdb1
mkfs.ext3 -b 4096 -N 100000 -m 0 /dev/mapper/c01
mkdir -p /c01
mount /dev/mapper/c01 /c01

cat > /usr/local/bin/mount_encrypted_filesystems.sh <<!
modprobe twofish

cryptsetup -y -c twofish-cbc-essiv:sha256 create c01 /dev/sdb1
mount /dev/mapper/c01 /c01

cryptsetup -y -c twofish-cbc-essiv:sha256 create c02 /dev/sdc1
mount /dev/mapper/c02 /c02
!

