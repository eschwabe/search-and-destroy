Game Development Project
Eric Schwabe

----------------------------------------------------------------------------------------------------
DESCRIPTION [AI PROJECT 1]
----------------------------------------------------------------------------------------------------
For this project, I implemented several state machines to cover NPC behavior, player controls, and
projectiles. The NPC behavior includes wandering around the map, patrolling the map, and engaging
in 'combat' with the player. The wander functions are implemented by detecting future collisions
for the NPC and steering away from walls. The detection only uses a single point in front of the
NPC, so it occasionally becomes blocked and reverses direction. Patrolling functionality is 
accomplished by using a state machine that moves the NPC to a specified point. The state machine is
then re-queued, and the next state machine moves the NPC to the next point. If the NPC is in range 
of the player or is damaged, the wander and patrol state machines switch to combat. The combat 
state machine handles following the player, taking damage, dying, and switching back to the previous
state machine.

The player and projectile state machines are responsible for managing their respective game objects.
The player state machine restricts how often the player can shoot projectiles and the projectile
state machine checks for collisions with NPCs and removes the projectile from the game after a 
fixed duration.

The instructions for running the game are documented below.

----------------------------------------------------------------------------------------------------
PROJECT EXPERIENCE
----------------------------------------------------------------------------------------------------
The project was not difficult to implement, but required a reasonable amount of effort. Overall, I
probably spent about 10 hours on the project. However, I spent extra time integrating the state
machine into my project since I plan on using many of the features for the 'final version'. I
personally would have found it useful to utilize the state machine code in a previous class. This
would have prevented me from implementing features similar to the state machine on my own. With
that in mind, I probably should have reviewed the code prior to this class to see how it would have
integrated in the future.

----------------------------------------------------------------------------------------------------
PROJECT FEATURES
----------------------------------------------------------------------------------------------------

FEATURES

SMPatrol.cpp:52 		Global Message Response other than MSG_Reset or MSG_MouseClick
SMPatrol.cpp:55 		Data passed in msg
SMPatrol.cpp:111		RequeueStateMachine
SMProjectile.cpp:139 	MarkForDeletion
SMWander.cpp:75			A persistent state variable such as DeclareStateInt
SMWander.cpp:92			PushStateMachine
SMWander.cpp:113		OnPeriodicTimeInState
SMWander.cpp:150		ChangeStateDelayed
SMCombat.cpp:128		OnTimeInState
SMCombat.cpp:134		PopStateMachine

EXTRA CREDIT

1. The game includes NPC patrolling functionality. The green spheres patrol around the edge of 
   the map as long as the player does not get in range of them.
2. I discovered the bug where OnExit is not called if you change state machines.

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
