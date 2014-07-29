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
	LAU->Call(6, 0); //call hook.Call with our  6 args and 0 returns
	LAU->Pop(2); //pop hook and Call from the stack
}

SpewRetval_t spewHandler(SpewType_t type, const char *msg) {
	#ifdef _WIN32
		if (GetCurrentThreadId() != gThread)
			return SPEW_CONTINUE;
	#endif // _WIN32

	
	if (!msg) //checking if the message is valid
		return SPEW_CONTINUE;

	const Color *color = GetSpewOutputColor();

	

	startHookCall();
		LAU->PushNumber(type); //pushing hook args: type, msg, group, level
		LAU->PushString(msg);
		LAU->PushString(GetSpewOutputGroup());
		LAU->PushNumber(GetSpewOutputLevel());
	endHookCall();

	return oldSpewFunc(type, msg); //pass it back to the default handler
	
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