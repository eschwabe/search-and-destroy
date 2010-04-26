/* Copyright Steve Rabin, 2008. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2008"
 */

// these message names are processed inside msg.h

REGISTER_MESSAGE_NAME(MSG_NULL)							// reserved message name
REGISTER_MESSAGE_NAME(MSG_GENERIC_TIMER)				// reserved message name
REGISTER_MESSAGE_NAME(MSG_CHANGE_STATE_DELAYED)			// reserved message name
REGISTER_MESSAGE_NAME(MSG_CHANGE_SUBSTATE_DELAYED)		// reserved message name

// add new messages here
REGISTER_MESSAGE_NAME(MSG_FireProjectile)
REGISTER_MESSAGE_NAME(MSG_FireBigProjectile)

// used for zombie and human demo state machines
REGISTER_MESSAGE_NAME(MSG_CheckTouch)
REGISTER_MESSAGE_NAME(MSG_Tagged)
REGISTER_MESSAGE_NAME(MSG_SetTargetPosition)
REGISTER_MESSAGE_NAME(MSG_Arrived)
REGISTER_MESSAGE_NAME(MSG_Reset)
REGISTER_MESSAGE_NAME(MSG_MouseClick)

// unit test messages
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage2)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage3)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage4)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage5)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage6)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage7)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage8)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage9)
REGISTER_MESSAGE_NAME(MSG_UnitTestBroken)
REGISTER_MESSAGE_NAME(MSG_UnitTestPing)
REGISTER_MESSAGE_NAME(MSG_UnitTestAck)
REGISTER_MESSAGE_NAME(MSG_UnitTestDone)
REGISTER_MESSAGE_NAME(MSG_UnitTestTimer)
