Game Development Project
Eric Schwabe

----------------------------------------------------------------------------------------------------
DESCRIPTION [AI PROJECT 2 PART 2]
----------------------------------------------------------------------------------------------------
For the second part of the project, I ported the A* pathfinding code to my game. Each NPC uses
pathfinding to explore the world. The green NPCs patrol around the edge of the map and the blue
NPCs path to random locations on the map. By default, debugging mode is enabled showing all the NPC
paths with lines and arrows. Debugging mode can be enabled/disabled using the toggle button.

The instructions for running the game are documented below.

----------------------------------------------------------------------------------------------------
PROJECT EXPERIENCE
----------------------------------------------------------------------------------------------------
Porting the pathfinding code to my game was a fairly simple task. The only difficult part was
creating a queue for pathfinding requests and keeping the generating paths accessible for
displaying debug lines. Overall, the port took about 4 hours to complete.

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
