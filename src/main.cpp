//taking heavy inspiration from Chris (STEAM_0:1:26262689)
//https://christopherthorne.googlecode.com/svn/trunk/gm_enginespew/gm_enginespew/main.cpp

#include "deps.h";

using namespace GarrysMod::Lua;


ILuaBase* LAU; /*since the spew handler func doesn't get provided with the LUA state / interface / w/e
			   we need to grab it on module load and store it here*/

//the old spew func will be stored here before we replace it with ours
SpewOutputFunc_t oldSpewFunc;

//will be our thread so weird things don't happen when stuff gets called from another thread
int gThread;

void startHookCall() {
	LAU->PushSpecial(SPECIAL_GLOB);
	LAU->GetField(-1, "hook");
		LAU->GetField(-1, "Call"); //getting the hook.Call function
			LAU->PushString("EngineSpew"); //passing the hook name
			LAU->PushNil(); //passing no GM table
}

void endHookCall() {
	LAU->Call(6, 1); //call hook.Call with our  6 args and 2 returns
}

const char *msgModification(const char *msg)
{
	if (!LAU->IsType(-1, Type::NIL)) 
	{
		if (LAU->IsType(-1, Type::STRING)) 
		{
			return LAU->GetString(-1); //make you able to modify the string
		}

		return ""; //just returning SPEW_CONTINUE will sometimes crash the game - this is just as good (hopefully)
	}
	return msg;
}

SpewRetval_t spewHandler(SpewType_t spewType, const char *msg) {
	#ifdef _WIN32
		if (GetCurrentThreadId() != gThread)
			return SPEW_CONTINUE;
	#endif // _WIN32
	
	if (!msg) //checking if the message is valid
		return SPEW_CONTINUE;

	startHookCall();
		LAU->PushNumber(spewType); //pushing hook args: type, msg, group, level
		LAU->PushString(msg);
		LAU->PushString(GetSpewOutputGroup());
		LAU->PushNumber(GetSpewOutputLevel());
	endHookCall();
	msg = msgModification(msg);
	LAU->Pop(3); //popping hook, call and our return

	return oldSpewFunc(spewType, msg); //pass it back to the default handler
	
}

GMOD_MODULE_OPEN() {
	LAU = LUA; //making it available in our whole module

	#ifdef _WIN32
		gThread = GetCurrentThreadId();
	#endif // _WIN32


	oldSpewFunc = GetSpewOutputFunc(); //saving the old func

	SpewOutputFunc(spewHandler); //making our function the spew handler

	return 0;
}

GMOD_MODULE_CLOSE() {
	SpewOutputFunc(oldSpewFunc); //making sure we get out before out lua thingy gets closed and everything breaks

	return 0;
}