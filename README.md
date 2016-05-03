#questroomcontrol
Quest Room hardware control GUI.

DESCRIPTION

GUI interface to manipulate special designed Quest Room control hardwaree

CONSTACTS

Roman Bulygin <mailto:rbulygin@gmail.com>

REQUIREMENTS

Qt 4 requires QtSerialPort library additionaly insatlled. 
Before build you should install it as described here:
http://wiki.qt.io/QtSerialPort

Qt 5 has QSerialPort class out of the box.

NOTES ABOUT WINDOWS STATIC BUILD QT5

1. Compile static libs and add static kit to QT Creator as described here
https://wiki.qt.io/Building_a_static_Qt_for_Windows_using_MinGW

2. Due Qt Creator fails to recognize static libraries to detect ABI correctly (yellow sign in the ).
As a workaround copy a QtCore*.dll matching your compiler and architecture next to libQtCore.a in the "static" folder. 
That one will be picked up first and make the ABI detection succeed. The Qt SDK should have a matching QtCore dll somewhere.

3. Add static bild to project
