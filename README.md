[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/OpenVR-driver-for-DIY/blob/master/README.md) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/OpenVR-driver-for-DIY/blob/master/README.RU.md) 
# OpenVR driver for DIY
Driver for OpenVR / SteamVR allowing change HMD and controllers rotation and position with buttons.<br><br>
![](https://user-images.githubusercontent.com/9499881/27448173-cff2d362-5794-11e7-9514-8b10fb966c92.gif) ![](https://user-images.githubusercontent.com/9499881/43992283-2df39dc6-9d8e-11e8-8e6d-8b76e459f0e4.gif)

## Buttons HMD
NumPad 2 & 8 - turn up and down.<br>
NumPad 4 & 6 - turn left and right.<br>
NumPad 1 & 3 - rotation of the head left and right.<br>
NumPad 9 - resetting of turns and rotation.
Up & Down - move forward and back.<br>
Left & Right - move left and right.<br>
PageUp и PageDown - move up and down.<br>
End - reset position.

## Controller buttons 1
W, A, S, D - move left, right, forward and back.<br>
Q, E - move up and down.<br>
R - reset the move.<br>
Z, X, C, V, 1 - сontroller buttons.<br>
T, G, F, H - rotation of both controllers.<br>
B - reset the rotation of both controllers.

## Controller buttons 2
I, K, J, L - move left, right, forward and back.<br>
U, O - move up and down.<br>
P - reset the move.<br>
N, M, <, >, 4 - сontroller buttons.

## Setup

### Windows
1. copy the folder "driver" from the archive into the folder "<steam-install-path>\steamapps\common\SteamVR
2. start steamVR

### Linux
1. Build Driver:
 ```
 git clone git@github.com:r57zone/OpenVR-driver-for-DIY.git openvr-driver-for-diy
 mkdir openvr-driver-for-diy-build
 cd openvr-driver-for-diy-build
 cmake ../openvr-driver-for-diy
 make
 ```
2. Install Driver:
 ```
 make install
 ```
3. Start [SteamVR](https://store.steampowered.com/steamvr)

## Activate driver
The driver is activated by default and requires no further settings. If it was previously disabled, open the file
"<steam-install-path>\steamapps\common\SteamVR\drivers\sample\recources\settings\default.vrsettings"
in an editor and set the "enable" attribute to true, then save and restart SteamVR.

## Disable driver
Open the file "<steam-install-path>\steamapps\common\SteamVR\drivers\sample\recources\settings\default.vrsettings"
in an editor and set the attribute "enable" to false, then save and restart SteamVR.

## Remove driver
To remove, delete the directory "<steam-install-path>\steamapps\common\SteamVR\drivers\sample".

## Download
>Windows Version for x86 & x64.<br>
**[Download](https://github.com/r57zone/OpenVR-driver-for-DIY/releases)**<br>

## Feedback
`r57zone[at]gmail.com`
