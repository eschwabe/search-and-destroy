Game Development Project
Eric Schwabe

----------------------------------------------------------------------------------------------------
DESCRIPTION [AI PROJECT 3]
----------------------------------------------------------------------------------------------------
This project required implementing terrain analysis grids that are displayable in the world. The
three grids implemented in this project are openness, occupancy, and line-of-fire. The openness 
layer shows how close a cell on the grid is to a wall. The further away from walls, the smaller the 
value of the openness layer (and therefore the lighter the color). The occupancy layer shows how 
many players and NPCs are near a cell. A cell with a player or NPC directly on it displays a dark
color. The line-of-fire layer shows if a cell is in sight of the player. This indicates if the 
player is able to shoot a projectile at the cell.

The cell color mapping is as follows:
Green - Openness 
Red   - Occupancy
Blue  - Line Of Fire

The terrain analysis debugging may be enabled by either using the 'L' key or the button on screen.
The hotkey and button toggle through the various terrain analysis grids.

----------------------------------------------------------------------------------------------------
PROJECT EXPERIENCE
----------------------------------------------------------------------------------------------------
This project was simple to implement. Previous projects required implementing similar functions
such as drawing quads and analyzing the world grid. The only difficult part of the project was
determining the proper algorithms for computing the terrain analysis grids. In particular, the
line-of-fire algorithm took a few different attempts to find an acceptable solution. The amount of
time spent on the project in total was approximately 6 hours.

----------------------------------------------------------------------------------------------------
GAME INSTRUCTIONS
----------------------------------------------------------------------------------------------------
INTRODUCTION
The source, executables, and media in this package were created as a part of the University of 
Washington Game Development Certificates Course. The game uses example code, models, and textures 
provided in the Direct3D SDK. The game was built on the Windows platform with the Direct3D SDK 
installed and Visual Studio 2008. Some additional textures were obtained from cgtextures.com and 
cannot be sold.


GAME
The game provides a simple grid-based level for the player to explore. Several enemies exist in the
world, but cannot attack you (at this time!). The enemies will wander and patrol around the map
trying to find the player. If the player is in range, the enemies will start to pursue the player.
The player can stun and kill the enemies with a particle balls.


BASIC CONTROLS
[N]             Next View           Switches the view to the next camera
[P]             Previous View       Switches the view to the previous camera
[R]             Reset View          Switches to the default camera (Third-Person Player Camera)
[L]				Terrain Analysis	Toggles through the terrain analysis debug views

PLAYER CONTROLS
[A or Left]     Turn Left           Rotates the player left
[D or Right]    Turn Right          Rotates the player right
[W or Up]       Move Forward        Moves the character forward
[S or Down]     Move Backwards      Moves the character backwards
[Left Mouse]	Shoot				Launches a small projectile from the player
[Right Mouse]	Shoot Big			Launches a large, but slower projectile from the player


DEBUG CAMERA CONTROLS
The debug camera supports several duplicate controls for ease of use depending on user preference. 
The first scheme uses keyboard arrow keys and the second scheme uses the keyboard and mouse.

SCHEME ONE
[A]             Strafe Left         Strafes the camera left
[D]             Strafe Right        Strafes the camera right
[W]             Move Forward        Moves the camera forward
[S]             Move Backwards      Moves the camera backwards
[Space]         Raise Camera        Raises the camera position
[Mouse]         Rotate View         Rotates the camera view when any mouse button is held down

SCHEME TWO
[Left]          Turn Left           Rotates the camera left
[Right]         Turn Right          Rotates the camera right
[Up]            Move Forward        Moves the camera forward
[Down]          Move Backwards      Moves the camera backwards
[PageUp]        Pitch Up            Turns the camera pitch up
[PageDown]      Pitch Down          Turns the camera pitch down
[Z]             Raise Camera        Raises the camera position
[X]             Lower Camera        Lowers the camera position
