# SpaceShipGame2
Simple space ship game created with opengl

This is a simple game with contralable space ship via the arrow keys. As you pick up power-ups the object will dissapear 
along with its obstacle, and the space ship will become larger. Works a lot with transformations of multiple objects.

## Game Logic:
The game starts with the space ship in the middle and 8 power-ups (diamond) throughout the window. Get close to the power-ups to 
'eat' them, increasing your score by 1 and making your space ship larger. Be careful of the moving obstacles around the power-ups 
(fire) as touching those object will kill you and restart the game. If all power-ups are collected, you have won the game!

## Running Game:
Open the 453-skeleton.exe file in \SpaceShipGame\out\build\x64-Debug folderpath

## Controls:
Arrow UP: Moves space ship forward (the direction it is facing)

Arrow BACK: Moves space ship backward (opposite the direction it is facing)

Cursor: Use your cursor to determine the space ship direction. It'll face to wherever the cursor is at in the window.

Enter: Press the [ENTER] key at anytime to restart the game.

## Compiler and Platform
Compiler: Clang++

Platform: Microsoft Windows 11, 64bit

Build Tools: Cmake

Application: Microsoft Visual Studio 2022

![SpaceShip Game Preview](https://github.com/juandiegovil/SpaceShipGame2/assets/66028457/da792985-5e48-499e-80b7-747f70eb09d2)

