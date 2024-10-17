# VirtualBox
## Remove deleted virtual disks from Hard Drive Selector
```
vboxmanage list hdds
vboxmanage closemedium disk "{d89ef84a-d754-4da2-b2a1-cc37063d0c6d}" --delete
```
