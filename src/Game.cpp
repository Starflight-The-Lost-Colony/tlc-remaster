/*
	STARFLIGHT - THE LOST COLONY
	Game.cpp 
    This is the main game control source file. 

    Coding standards used in the project for consistency:
    - All variables/properties: lower first char, then upper first chars
    - Functions/methods: upper first chars on all (or lower single word if you prefer)

*/

#pragma region HEADER

#include "env.h"
#include <allegro.h>
#include <alfont.h>
#include <fmod.hpp>
#include <memory.h>
#include <cstdio>
#include <sstream>
#include "LogFile.h"

#include "Game.h"
#include "GameState.h"
#include "Util.h"
#include "ModeMgr.h"
#include "Module.h"
#include "DataMgr.h"
#include "AudioSystem.h"
#include "QuestMgr.h"
#include "Script.h"
#include "MessageBoxWindow.h"
#include "PauseMenu.h"

#include "ModuleStarmap.h"
#include "ModuleShipConfig.h"
#include "ModuleTitleScreen.h"
#include "ModuleCredits.h"
#include "ModuleInterstellar.h"
#include "ModuleSolarSystem.h"
#include "ModulePlanetOrbit.h"
#include "ModulePlanetSurface.h"
#include "ModuleControlPanel.h"
#include "ModuleCaptainCreation.h"
#include "ModuleCaptainsLounge.h"
#include "ModuleCrewHire.h"
#include "ModuleStarport.h"
#include "ModuleBank.h"
#include "ModuleTradeDepot.h"
#include "ModuleGameOver.h"
#include "ModuleCargoWindow.h"
#include "ModuleCantina.h"
#include "ModuleQuestLog.h"
#include "ModuleEncounter.h"
#include "ModuleStartup.h"
#include "ModuleAuxiliaryDisplay.h"
#include "ModuleMessageGUI.h"
#include "ModuleTopGUI.h"
#include "ModuleMedical.h"
#include "ModuleEngineer.h"
#include "ModuleSettings.h"

//define global objects for project-wide visibility, not dependent on Game class for access
GameState	*Game::gameState = NULL;
ModeMgr		*Game::modeMgr = NULL;
DataMgr		*Game::dataMgr = NULL;
AudioSystem *Game::audioSystem = NULL;
QuestMgr	*Game::questMgr = NULL;

#pragma endregion


void trace(std::string t)
{
    debug << t << std::endl;
}

Game::Game()
{
	timePause= true;		//start paused.
	timeRateDivisor= 2;		//=> 2 seconds == 1 game hour.
//*************************************JJH
	CrossModuleAngle = 0;
//*************************************JJH
	showControls = true;
    m_pause = false;
    m_keepRunning = true;
    m_backbuffer = NULL;
    desktop_width = 0;
    desktop_height = 0;
    desktop_colordepth = 0;
	frameCount = 0;
    startTime = 0;
    frameRate = 0;
	m_numMouseButtons = 0;
	m_mouseButtons = NULL;
	m_prevMouseX = -1;
    m_prevMouseY = -1;
	m_prevMouseZ = 0;
	m_prevMouseButtons = NULL;
	m_mousePressedLocs = NULL;
	vibration = 0;
	globals = NULL;
	messageBox = NULL;
	gameState = NULL;
	modeMgr = NULL;
	questMgr = NULL;
	audioSystem = NULL;
	dataMgr = NULL;
	pauseMenu = NULL;
	cursor = NULL;

	font10 = NULL;
    font12 = NULL;
    font18 = NULL;
    font20 = NULL;
    font22 = NULL;
    font24 = NULL;
    font32 = NULL;

	MsgColors[MSG_INFO   ] = GREEN;
	MsgColors[MSG_ALERT  ] = RED;
	MsgColors[MSG_ERROR  ] = YELLOW;
	MsgColors[MSG_ACK    ] = LTGREEN;
	MsgColors[MSG_FAILURE] = LTRED;
	MsgColors[MSG_SUCCESS] = LTGREEN;
	MsgColors[MSG_TASK_COMPLETED] = BLUE;
	MsgColors[MSG_SKILLUP] = PURPLE;

}

Game::~Game()
{
	//delete messagebox globals
	destroy_bitmap(MessageBoxWindow::bg);
	destroy_bitmap(MessageBoxWindow::bar);
	if (MessageBoxWindow::button1 != NULL)
		delete MessageBoxWindow::button1;
	if (MessageBoxWindow::button2 != NULL)
		delete MessageBoxWindow::button2;

	//destroy the ScrollBox queue
	messages.clear();

    //destroy the video modes list
    videomodes.clear();

	//kill the pause menu
	if (pauseMenu != NULL)
		delete pauseMenu;

	//destroy fonts
	alfont_destroy_font(font10);
	alfont_destroy_font(font12);
	alfont_destroy_font(font18);
	alfont_destroy_font(font20);
	alfont_destroy_font(font22);
	alfont_destroy_font(font24);
	alfont_destroy_font(font32);

}

#pragma region UTILITY_FUNCS

void Game::message(std::string msg)
{
    debug << msg << endl;
	allegro_message(msg.c_str());
}

void Game::fatalerror(std::string error)
{
	try {
		debug << error << endl;
		message(error);
		g_game->modeMgr->CloseCurrentModule();
		this->Stop();
	}
	catch(...){
		//throw an error up to main.cpp
		//throw "serious problem in fatalerror";
	}
}

void Game::shutdown()
{
	try {
		debug << "[shutting down]" << endl;
		g_game->modeMgr->CloseCurrentModule();
		this->Stop();
	}
	catch(...){
		//throw an error up to main.cpp
		throw "serious problem in shutdown";
	}
}

void Game::ShowMessageBoxWindow(
    std::string initHeading,
	std::string initText,
	int initWidth,
	int initHeight,
	int initTextColor,
	int initX,
	int initY,
	bool initCentered,
	bool pauseGame )
{
	m_pause = true; // pauseGame;

	//if we have a msg box window, delete it
	KillMessageBoxWindow();

	messageBox = new MessageBoxWindow(
        initHeading,
		initText,
		initX,
		initY,
		initWidth,
		initHeight,
		initTextColor,
		initCentered);
}

void Game::KillMessageBoxWindow()
{
	if(messageBox != NULL)
	{
		m_pause = false;
		delete messageBox;
		messageBox = NULL;
	}
}

//In the current scheme for storing/displaying stardates, time is set using gameTimeSecs from a
//default date. The baseGameTimeSeconds are added to the current timer seconds to get gameTimeSecs;
//This number is then divided by the timeRateDivisor to get the final stardate. So when we change
//the divisor, we must simultaneously change the baseGameTimeSeconds (following the method used
//in SetTimePaused by dumping everything into baseGameTimeSeconds & resetting the timer makes
//things easier).
void Game::SetTimeRateDivisor(int v) {
	double ratio= v/timeRateDivisor;
	gameState->setBaseGameTimeSecs(gameState->getGameTimeSecs()*ratio);
	globalTimer.reset();	//reset timer.
	timeRateDivisor= v;
}

void Game::SetTimePaused(bool val) {
	if (timePause == val) return;	//do not do anything for redundant call.
	timePause= val;
	if (timePause)			//update base with current data:
		gameState->setBaseGameTimeSecs(gameState->getGameTimeSecs());
	globalTimer.reset();	//reset timer.
}

void Game::TogglePauseMenu()
{
	static bool origTimePause;
	if (!pauseMenu->isEnabled()) return;

	if (pauseMenu->isShowing())
	{
		SetTimePaused(origTimePause);
		//hide pausemenu
		m_pause = false;
		pauseMenu->setShowing(false);
	}
	else {
		origTimePause= getTimePaused();		SetTimePaused(true);
		//show pausemenu
		m_pause = true;
		pauseMenu->setShowing(true);
	}
}

#pragma endregion

#pragma region "Lua script validation and globals"

//these three are identical to Script class methods but are more convenient within g_game
void Game::runGlobalFunction(std::string name)
{
	globals->runFunction(name);
}
std::string Game::getGlobalString(std::string name)
{
	return globals->getGlobalString(name);
}
void Game::setGlobalString(std::string name, std::string value)
{
    globals->setGlobalString(name, value);
}

double Game::getGlobalNumber(std::string name)
{
	return globals->getGlobalNumber(name);
}
void Game::setGlobalNumber(std::string name, double value)
{
	globals->setGlobalNumber(name, value);
}
bool Game::getGlobalBoolean(std::string name)
{
	return globals->getGlobalBoolean(name);
}

//this bogus callback is used for script verification
int voidfunc(lua_State* L) { return 0; }

bool ValidateScripts()
{
	string error,filename,linenum,message;
	int pos,n;

	const int PLANETFUNCS = 103;
	string planet_funcnames[PLANETFUNCS] = {
		"L_Debug",
		"L_LoadImage",
		"L_Move",
		"L_Draw",
		"L_Animate",
		"L_LaunchTV",
		"L_SetActions",
		"L_LoadScript",
		"L_PostMessage",
		"L_LoadPortrait",
		"L_ShowPortrait",
		"L_AddItemtoCargo",
		"L_DeleteSelf",
		"L_LoadPSObyID",
		"L_CreateNewPSO",
		"L_CreateNewPSObyItemID",
		"L_LoadPlayerTVasPSO",
		"L_LoadPlayerShipasPSO",
		"L_Test",
		"L_SetRunPlanetLoadScripts",
		"L_GetRunPlanetLoadScripts",
		"L_SetRunPlanetPopulate",
		"L_GetRunPlanetPopulate",
		"L_CheckInventorySpace",
		"L_KilledAnimal",
		"L_AttackTV",
		"L_TVDestroyed",
		"L_TVOutofFuel",
		"L_PlayerTVisAlive",
		"L_CheckInventoryFor",
		"L_RemoveItemFromInventory",
		"L_GetPlanetID",
		"L_CreateTimer",
		"L_GetPlayerShipPosition",
		"L_GetPlayerTVPosition",
		"L_GetActiveVesselPosition",
		"L_GetScrollerPosition",
		"L_GetPlayerProfession",
		"L_GetPosition",
		"L_GetOffsetPosition",
		"L_GetScreenWidth",
		"L_GetScreenHeight",
		"L_GetScreenDim",
		"L_GetSpeed",
		"L_GetFaceAngle",
		"L_GetPlayerNavVars",
		"L_GetScale",
		"L_GetCounters",
		"L_GetThresholds",
		"L_IsPlayerMoving",
		"L_GetItemID",
		"L_GetState",
		"L_GetVesselMode",
		"L_IsScanned",
		"L_GetName",
		"L_GetValue",
		"L_GetDamage",
		"L_IsBlackMarketItem",
		"L_IsShipRepairMetal",
		"L_IsAlive",
		"L_GetColHalfWidth",
		"L_GetColHalfHeight",
		"L_GetID",
		"L_GetScriptName",
		"L_GetHealth",
		"L_GetMaxHealth",
		"L_GetStunCount",
		"L_GetItemSize",
		"L_GetSelectedPSOid",
		"L_GetObjectType",
		"L_GetDanger",
		"L_GetMinimapColor",
		"L_GetMinimapSize",
		"L_SetPosition",
		"L_SetVelocity",
		"L_SetSpeed",
		"L_SetFaceAngle",
		"L_SetAnimInfo",
		"L_SetAngleOffset",
		"L_SetScale",
		"L_SetCounters",
		"L_SetThresholds",
		"L_SetState",
		"L_SetVesselMode",
		"L_SetScanned",
		"L_SetDamage",
		"L_SetAlive",
		"L_SetColHalfWidth",
		"L_SetColHalfHeight",
		"L_SetScriptName",
		"L_SetAlpha",
		"L_SetHealth",
		"L_SetMaxHealth",
		"L_SetStunCount",
		"L_SetObjectType",
		"L_SetName",
		"L_SetMinimapColor",
		"L_SetMinimapSize",
		"L_SetNewAnimation",
		"L_SetActiveAnimation",
		"L_PlaySound",
		"L_PlayLoopingSound",
		"L_StopSound",
	};

	const int PLANETNUM = 12;
	string planetScripts[PLANETNUM] = {
		"data/planetsurface/Functions.lua",
		"data/planetsurface/stunprojectile.lua",
		"data/planetsurface/mineral.lua",
		"data/planetsurface/basicLifeForm.lua",
		"data/planetsurface/PlanetSurfacePlayerShip.lua",
		"data/planetsurface/PlanetSurfacePlayerTV.lua",
		"data/planetsurface/PopAsteriod.lua",
		"data/planetsurface/PopFrozenPlanet.lua",
		"data/planetsurface/PopMoltenPlanet.lua",
		"data/planetsurface/PopOceanicPlanet.lua",
		"data/planetsurface/PopRockyPlanet.lua",
		"data/planetsurface/artifact.lua",
	};

    debug << "Validating planet scripts..." << endl;

	//validate planet surface scripts
	Script *planetScript;

	//validate all script files
	for (n=0; n < PLANETNUM; n++)
	{
		planetScript = new Script();

		//register all required C++ functions needed by planet scripts
		for (int f=0; f < PLANETFUNCS; f++)
			lua_register(planetScript->getState(), planet_funcnames[f].c_str(), voidfunc);

		//feed the scripts fake planet info
		planetScript->setGlobalString("PLANETSIZE"  , "SMALL");
		planetScript->setGlobalString("TEMPERATURE" , "SUBARCTIC");
		planetScript->setGlobalString("GRAVITY"     , "NEGLIGIBLE");
		planetScript->setGlobalString("ATMOSPHERE"  , "NONE");

		//open the planet script
		if (!planetScript->load(planetScripts[n]))
		{
			error = planetScript->errorMessage;
			pos = (int)error.find(":");
			filename = error.substr(0, pos);
			error = error.substr(pos+1);
			pos = (int)error.find(":");
			linenum = error.substr(0,pos);
			message = error.substr(pos+1);
			error = "Filename: " + planetScripts[n] + "\n\nLine #: " + linenum + "\n\nError: " + filename + "\n" + message;
            debug << "SCRIPT ERROR" << endl;
			debug <<  error << endl;
			delete planetScript;
			return false;
		}
		delete planetScript;
	}


	const int ENCFUNCS = 3;
	string enc_funcnames[ENCFUNCS] = {
		"L_Debug",
		"L_Terminate",
		"L_Attack",
	};

	const int ENCNUM = 12;
	string encounterScripts[ENCNUM] = {
		"data/globals.lua",
		"data/quests.lua",
		"data/encounter/encounter_common.lua",
		"data/encounter/encounter_pirate.lua",
		"data/encounter/encounter_spemin.lua",
		"data/encounter/encounter_tafel.lua",
		"data/encounter/encounter_thrynn.lua",
		"data/encounter/encounter_nyssian.lua",
		"data/encounter/encounter_minex.lua",
		"data/encounter/encounter_elowan.lua",
		"data/encounter/encounter_coalition.lua",
		"data/encounter/encounter_barzhon.lua"
	};

    debug << "Validating encounter scripts..." << endl;

	//validate global and encounter scripts
	Script *scr;
	for (n=0; n < ENCNUM; n++)
	{
		scr = new Script();

		//register all required C++ functions needed by encounter scripts
		for (int f=0; f < ENCFUNCS; f++)
			lua_register(scr->getState(), enc_funcnames[f].c_str(), voidfunc);
		
		if (!scr->load(encounterScripts[n]))
		{
			error = scr->errorMessage;
			pos = (int)error.find(":");
			filename = error.substr(0, pos);
			error = error.substr(pos+1);
			pos = (int)error.find(":");
			linenum = error.substr(0,pos);
			message = error.substr(pos+1);
			error = "Filename: " + encounterScripts[n] + "\n\nLine #: " + linenum + "\n\nError: " + filename + "\n" + message;
            debug << "SCRIPT ERROR" << endl;
			debug << error << endl;
			delete scr;
			return false;
		}
		delete scr;
	}
	return true;
}

/*
   verify that the portraits for all the items in the database do exist + log the missing ones in allegro.log
   return true if all files where found, false if there was at least one file missing.
*/

bool ValidatePortraits()
{
	bool retval = true;
	bool doCheck = true;  //to skip minerals -- we don't do portraits for minerals right now

//TODO: right now ValidatePortraits() skip lifeforms portraits too, since it
// proved itself to be much more work than expected. we need to handle the
// various data/planetsurface/Pop${planetType}Planet.lua for it to work.
// See the commented code below to get an idea of what i envisioned. It would
// require rework of the lua scripts to make them work as i want. Either that
// or let them think we are ModulePlanetSurface.cpp by implementing the lua
// stuff and CreatePSObyItemID() here; and we definitely don't want to do that.
/*
	//the portraits for the lifeForms are scattered in the various planetsurface/Pop${planetType}Planet.lua
	Script script;
	std::string PopScripts[6]={
		"data/planetsurface/PopAcidicPlanet.lua",
		"data/planetsurface/PopAsteriod.lua",
		"data/planetsurface/PopFrozenPlanet.lua",
		"data/planetsurface/PopMoltenPlanet.lua",
		"data/planetsurface/PopOceanicPlanet.lua",
		"data/planetsurface/PopRockyPlanet.lua" };
	for (int n=0; n<6; n++) script.load(PopScripts[n]);
*/

    debug << "Validating portraits..." << endl;

	for (int n=0; n<g_game->dataMgr->GetNumItems(); n++){
		Item *item = g_game->dataMgr->GetItem(n);
		std::string filepath;
		
		switch(item->itemType){
			case IT_INVALID:{
				//this one is not supposed to ever happen
				debug << "[ERROR]: item #" << item->id << " is of invalid type" << endl;
				ASSERT(0);
			}
			case IT_ARTIFACT:{
				doCheck = true;
				filepath = "data/tradedepot/" + item->portrait;
				break;
			}
			case IT_RUIN:{
				doCheck = true;
				filepath = "data/planetsurface/" + item->portrait;
				break;
			}
			case IT_MINERAL:
				//minerals do not have portraits
				doCheck = false;
				break;
			case IT_LIFEFORM:
			{
				//TODO: we skip that too since it doesn't work yet; see top of function for details
				doCheck = false;
				//item->portrait comes from the various Pop${planetType}Planet.lua; those already prepend "data/planetsurface"
				//filepath = item->portrait;
				break;
			}
			case IT_TRADEITEM:
			{
				doCheck = true;
				filepath = "data/tradedepot/" + item->portrait;
				break;
			}
			default:{
				//not supposed to happen either
				debug << "[ERROR]: item # is of unknown type: " << item->id << endl;
				ASSERT(0);
				break;
			}
		}

		if ( doCheck && !file_exists(filepath.c_str(),FA_ALL,NULL) ){
            debug << "[WARNING]: portrait " << filepath << " for item # does not exist: " << item->id << endl;
			retval=false;
		}
	}

	return retval;
}

#pragma endregion


void Game::Run()
{
	//validate scripts
	debug << "Validating Lua scripts..." << endl;
	if (!ValidateScripts()) {
        debug << "Fatal error: script validation failed" << endl;
		return;
	}


	//initialize scripting and load globals.lua
	debug << "Loading startup script..." << endl;
	globals = new Script();
	globals->load("data/globals.lua");

	debug << "Initializing game..." << endl;
	if (!InitGame()) {
		fatalerror("Error during game initialization\n");
		return;
	}

	debug << "Firing up game state..." << endl;
	gameState = new GameState();

	debug << "Firing up mode manager..." << endl;
	modeMgr = new ModeMgr(this);

	debug << "Firing up data manager..." << endl;
	dataMgr = new DataMgr();
	if (!dataMgr->Initialize()) {
		fatalerror("Error initializing data manager");
	}
	//check that all the graphics for items are present
	if (!ValidatePortraits())
		fatalerror("Some image files were missing--see allegro.log for details");

	//initialize gamestate
	gameState->Reset();

	debug << "Firing up quest manager..." << endl;
	questMgr = new QuestMgr();
	if (!questMgr->Initialize()) {
		fatalerror("Error initializing quest manager");
	}

	debug << "Initializing modules..." << endl;
	InitializeModules();
	gameState->m_captainSelected = false;


	//editing this out since starting up in different modules causes problems
	//instead a module jump will be added as a debug feature in the starport

	std::string startupmodule;
	startupmodule = globals->getGlobalString("STARTUPMODULE");
	if (startupmodule.length() == 0) startupmodule = MODULE_STARTUP;
	debug << "\nLaunching Startup Module: " << startupmodule << endl;
	modeMgr->LoadModule(startupmodule);

	//set window caption with title, version
	std::ostringstream s;
	s << p_title << " (V" << p_version << ")";
	set_window_title(s.str().c_str());


	debug << "\nLaunching game loop..." << endl;
	while (m_keepRunning)
	{
		try {
			RunGame();
		}
		catch(std::exception e)
		{
			debug << e.what() << endl;
		}
	}

	debug << "\nBailing..." << endl;
	DestroyGame();
}


void Game::Stop()
{
	m_keepRunning = false;
}


/*
 * Initialize Allegro graphics callable from settings screen to reset mode as needed.
 */
bool Game::Initialize_Graphics()
{
    int gfxmode=0;

    //since this func can be called repeatedly, let's skip redundancies
    if (desktop_width == 0)
    {
        get_desktop_resolution(&desktop_width, &desktop_height);
        debug << "Desktop resolution: " << desktop_width << " x " << desktop_height << endl;

        //this will probably be used regardless of resolution settings
        desktop_colordepth = desktop_color_depth();
	    set_color_depth(desktop_colordepth);
	    set_alpha_blender();
        debug << "Desktop color depth: " << desktop_colordepth << endl;
    }
    else
        debug << "Attempting to reset graphics mode..." << endl;

    //try to get user-selected resolution chosen in the settings screen
    //format will be: resolution = "1024x768" or "1024 x 768";
    string resolution = g_game->getGlobalString("RESOLUTION");
    if (resolution == "") {
        actual_width = desktop_width;
        actual_height = desktop_height;
        g_game->setGlobalString("RESOLUTION", Util::ToString(actual_width) + "x" + Util::ToString(actual_height));
    }
    else {
        std::size_t div = resolution.find_first_of("xX,");
        if (div != string::npos) {
            string ws = resolution.substr(0,div);
            string hs = resolution.substr(div+1);
            actual_width = Util::StringToInt(ws);
            actual_height = Util::StringToInt(hs);
            if (actual_width<1024) actual_width=1024;
            if (actual_height<768) actual_height=768;
        }
        else {
            actual_width = desktop_width;
            actual_height = desktop_height;
        }
    }
    debug << "Settings resolution: " << actual_width << "," << actual_height << endl;

    //try to get user-selected fullscreen toggle from settings screen
    bool fullscreen = g_game->getGlobalBoolean("FULLSCREEN");
    if (fullscreen) {
        gfxmode = GFX_DIRECTX_ACCEL; 
    }
    else {
        gfxmode = GFX_DIRECTX_WIN; 
        //width=SCREEN_WIDTH; height=SCREEN_HEIGHT;
    }
    
    //set text mode to reset graphics
    set_gfx_mode(GFX_TEXT,0,0,0,0);

    //try to set graphics mode
	if (set_gfx_mode(gfxmode, actual_width, actual_height, 0, 0) != 0)
	{
        debug << "Video mode failed (" << resolution << "), attempting default mode..." << endl;
        actual_width = SCREEN_WIDTH;
        actual_height = SCREEN_HEIGHT;
        if (set_gfx_mode(gfxmode, actual_width, actual_height, 0, 0) != 0)
        {
            debug << "Fatal Error: Unable to set graphics mode" << endl;
            return false;
        }
	}
    debug << "Refresh rate: " << get_refresh_rate() << endl;


    //
	//Create the backbuffer surface based on internal fixed resolution.
    //The frame buffer is SCALED to the output of the desired resolution!
    //Since this func can be called repeatedly from Settings, we need to destroy and 
    //recreate the back buffer each time.
    //
    if (m_backbuffer) 
    {
        debug << "Destroying old backbuffer " << m_backbuffer->w << "," << m_backbuffer->h << "..." << endl;
        destroy_bitmap(m_backbuffer);
        m_backbuffer = NULL;
    }
    debug << "Creating back buffer " << SCREEN_WIDTH << "," << SCREEN_HEIGHT << "..." << endl;
	m_backbuffer = create_bitmap(SCREEN_WIDTH,SCREEN_HEIGHT);
	if (!m_backbuffer) {
        debug << "Error creating back buffer" << endl;
        return false;
    }
    

   /*
     * Retrieve complete list of resolutions supported by DirectX driver and
     * populate the global Game::VideoModes list for use in the Settings screen.
     * Minimum is 1024x768 since downscaling does not work properly--most UI
     * code is based on SCREEN_WIDTH/SCREEN_HEIGHT assumptions, such as the mouse.
     */
    if (videomodes.size() == 0)
    {
        GFX_MODE_LIST *list = NULL;
        list = get_gfx_mode_list(GFX_DIRECTX_ACCEL);
        for (int i = list->num_modes; i >= 0; i--)
        {
            //add to list only if bpp matches detected desktop color depth
            if (list->mode[i].bpp == desktop_colordepth)
            {
                VideoMode mode;
                mode.bpp = list->mode[i].bpp;
                mode.width = list->mode[i].width;
                mode.height = list->mode[i].height;
                if (mode.width>=1024 && mode.height>=768)
                    videomodes.push_back(mode);
            }
        }
        destroy_gfx_mode_list(list);

        debug << "Detected video modes:" << endl;
        for (VideoModeIterator mode = videomodes.begin(); mode != videomodes.end(); ++mode)
	    {
            debug << mode->bpp << "," << mode->width << "," << mode->height << endl;
        }
    }

    return true;
}

/*
 * INITIALIZE LOW-LEVEL LIBRARY AND ENGINE RESOURCES
 */
bool Game::InitGame()
{
	Util::Init();

	//get title and version from script
	p_title = getGlobalString("GAME_TITLE");
	p_version = getGlobalString("GAME_VERSION");
	debug << p_title << " v" << p_version << endl;

	debug << "Firing up Allegro..." << endl;
	if (allegro_init() != 0) {
		return false;
	}

	debug << "Firing up Alfont..." << endl;
	if (alfont_init() != ALFONT_OK) {
		g_game->message("Error initializing font system");
		return false;
	}

	debug << "Firing up graphics system..." << endl;
    if (!Initialize_Graphics()) {
        g_game->fatalerror("Error initializing graphics\n");
        return false;
    }

	debug << "Firing up keyboard and mouse handlers..." << endl;
	if (install_keyboard() != 0) {
		g_game->message("Error initializing keyboard");
		return false;
	}
	memset(m_prevKeyState,0,256);
	m_numMouseButtons = install_mouse();
	if (m_numMouseButtons < 0) {
		g_game->message("Error initializing mouse");
		return false;
	}
	m_mouseButtons = new bool[m_numMouseButtons+1];
	m_prevMouseButtons = new bool[m_numMouseButtons+1];
	m_mousePressedLocs = new MousePos[m_numMouseButtons+1];
	for (int button = 0; button < m_numMouseButtons+1; button++)
	{
		m_mouseButtons[button] = false;
		m_prevMouseButtons[button] = false;
		m_mousePressedLocs[button].x = -1;
		m_mousePressedLocs[button].y = -1;
	}

	debug << "Firing up timers..." << endl;
	if (install_timer() != 0) {
		g_game->message("Error initializing timer system");
		return false;
	}

	debug << "Firing up sound system..." << endl;
	audioSystem = new AudioSystem();
	if (!audioSystem->Init()) {
		g_game->message("Error initializing the sound system");
		return false;
	}

	//load up default fonts
    //string fontfile = "data/ORBITBN.TTF";
    string fontfile = "data/gui/Xolonium-Regular.ttf";
	debug << "Creating default fonts..." << endl;
	font10 = alfont_load_font(fontfile.c_str());
	if (font10 == NULL) {
		g_game->message("Error locating font file");
		return false;
	}
	font12 = alfont_load_font(fontfile.c_str());
	alfont_set_font_size(font12, 12);
	font18 = alfont_load_font(fontfile.c_str());
	alfont_set_font_size(font18, 18);
	font20 = alfont_load_font(fontfile.c_str());
	alfont_set_font_size(font20, 20);
	font22 = alfont_load_font(fontfile.c_str());
	alfont_set_font_size(font22, 22);
	font24 = alfont_load_font(fontfile.c_str());
	alfont_set_font_size(font24, 24);
	font32 = alfont_load_font(fontfile.c_str());
	alfont_set_font_size(font32, 32);

	//create the PauseMenu
	pauseMenu = new PauseMenu();

	//hide the default mouse cursor
	show_mouse(NULL);

    if (g_game->getGlobalBoolean("DEBUG_MOUSE"))
    {
        //display system cursor at actual location (not scaled to resolution)
        show_os_cursor(2);
    }


	debug << "Initialization succeeded" << endl;
	return true;
}

void Game::DestroyGame()
{
	debug << "*** DestroyGame" << endl;

	if (cursor != NULL)
	{
		delete cursor;
		cursor = NULL;
	}
	if (globals != NULL)
	{
		delete globals;
		globals = NULL;
	}
	if (modeMgr != NULL)
	{
		delete modeMgr;
		modeMgr = NULL;
	}

	if (gameState != NULL)
	{
		delete gameState;
		gameState = NULL;
	}

	if (dataMgr != NULL)
	{
		delete dataMgr;
		dataMgr = NULL;
	}

	if (questMgr != NULL)
	{
		delete questMgr;
		questMgr = NULL;
	}

	if (audioSystem != NULL)
	{
		delete audioSystem;
		audioSystem = NULL;
	}

	if (m_backbuffer != NULL)
	{
		show_mouse(NULL);
		destroy_bitmap(m_backbuffer);
		m_backbuffer = NULL;
	}

	if (m_mouseButtons != NULL)
	{
		delete [] m_mouseButtons;
		m_mouseButtons = NULL;
	}

	if (m_prevMouseButtons != NULL)
	{
		delete [] m_prevMouseButtons;
		m_prevMouseButtons = NULL;
	}

	if (m_mousePressedLocs != NULL)
	{
		delete [] m_mousePressedLocs;
		m_mousePressedLocs = NULL;
	}

	debug << "\nShutdown completed." << endl;

	allegro_exit();
	alfont_exit();

}


void Game::UpdateAlienRaceAttitudes()
{
	//update alien attitudes (200,000 is 3 1/3 minute, changing from attitude 1 to 10 will require 30 minutes)
	int mins = 1;
	if (globalTimer.getTimer() > g_game->gameState->alienAttitudeUpdate + 200000 * mins) {
		g_game->gameState->alienAttitudeUpdate = globalTimer.getTimer();

		//update alien attitudes, all but for NONE(0) and PIRATE(1)
		for (int n=2; n<NUM_ALIEN_RACES; n++)
		{
			if (g_game->gameState->alienAttitudes[n] < 25)
				g_game->gameState->alienAttitudes[n]++;
			else if (g_game->gameState->alienAttitudes[n] > 65)
				g_game->gameState->alienAttitudes[n]--;
		}
	}
}

/*
WARNING!! The core loop used to have separate update/render timing but Allegro was consuming 
100% of a CPU core no matter what we did with the timing code so the only option is to use a 
SINGLE update timed at 60 fps. Don't change it!

OpenGL has been REMOVED from the project.
*/
void Game::RunGame()
{
	static std::ostringstream os;
	static int timeStart = globalTimer.getTimer();
	static int v;
	float fps_delay = 1000.0f / 60.0f;
	static int coreStartTime = 0;
	static int coreCounter = 0;
	//static double update_interval = 2.0;	//now in structure (private).

	if (globalTimer.getTimer() < timeStart + (int)fps_delay)
	{
		//slow down core loop
		//rest(1);
        fps_delay = 1000.0f / frameRate;

        if (g_game->getGlobalBoolean("UNLIMITED_FRAMERATE") == false)
        {
		    return;
        }
        else {
        }
	}
	else
		timeStart = globalTimer.getTimer();

	//update input
	UpdateKeyboard();
	UpdateMouse();

	if (!timePause) {		//Update the current stardate:
		//base game time is needed to properly restore the date from a savegame file
		double newTime= gameState->getBaseGameTimeSecs() + (double) globalTimer.getStartTimeMillis() / 1000.0;
		gameState->setGameTimeSecs(newTime);
		gameState->stardate.Update(newTime, timeRateDivisor);
	}

	//calculate core framerate
	coreCounter++;
    int gt = globalTimer.getTimer();
	if (gt > coreStartTime + 1000)
	{
		coreStartTime = gt;
		frameRate = coreCounter;
		coreCounter = 0;
	}
        
	if (!m_pause)
	{
		//call update on all modules
		modeMgr->Update();

		//update FMOD
		audioSystem->Update();
        
        //global abort flag to end game
		if (!m_keepRunning) return;

		//tell active module to draw
		modeMgr->Draw();

		//perform generic updates to time-sensitive game data

        UpdateAlienRaceAttitudes();


	} //mpause

	//handle the pause menu
	if (pauseMenu->isShowing()) 
    {
		//tell active module to draw
		modeMgr->Draw();

        //draw the pause popup
		pauseMenu->Draw();
	}


	//handle the messagebox
	if (messageBox != NULL)
	{
		if (messageBox->IsVisible())	
        {
	    	//tell active module to draw
    		modeMgr->Draw();

            //draw messagebox
			messageBox->Draw();
		}
		else {
			KillMessageBoxWindow();
		}
	}

    //draw mouse everywhere but during startup
	if (g_game->gameState->getCurrentModule() != "STARTUP")
	{
		if (cursor != NULL)
		{
            //keep the mouse cursor image on the screen
            int cw = cursor->getWidth(); 
            int ch = cursor->getHeight(); 
            int mx = mouse_x; 
            //if (mx < 0) mx = 0; 
            //if (mx > SCREEN_WIDTH - cw) mx = SCREEN_WIDTH - cw; 
            int my = mouse_y; 
            //if (my < 0) my = 0; 
            //if (my > SCREEN_HEIGHT - ch) my = SCREEN_HEIGHT - ch; 
			cursor->setX(mx); 
			cursor->setY(my); 
			cursor->Draw(m_backbuffer); 
		}
		else {
			//load the custom mouse cursor
			cursor = new Sprite();
			if (!cursor->load("data/gui/cursor.tga")) 
            {
				g_game->message("Error loading mouse cursor");
				g_game->shutdown();
				return;
			}
		}
	}


    //
    //prepare resolution scaling so values are available to debug output
    //
    screen_scaling = (double)screen->h / (double)SCREEN_HEIGHT;
    scale_height = (int)( (double)m_backbuffer->h * screen_scaling );
    scale_width = (int)( (double)m_backbuffer->w * screen_scaling );

    //display debug info on the upper-left corner of screen
    if (g_game->getGlobalBoolean("DEBUG_MODE") && g_game->getGlobalBoolean("DEBUG_CORE"))
    {
        int GRAY = makecol(160,160,160);
        ostringstream oss(""); 
		int y = 3;  int x = 3;
	    // x == 0 doesn't quite work on the Trade Depot Screen - made it a 3 - jjh

        oss.str(""); oss << std::fixed << std::setprecision(2);
        oss << "FPS:  " << frameRate << " (" << fps_delay << " ms)";
        g_game->Print12(m_backbuffer,x,y, oss.str().c_str(), GRAY);

        y+=10; g_game->Print12(m_backbuffer,x,y,"Screen:  " + Util::ToString((int)scale_width) + "," + Util::ToString((int)scale_height) + " (" + Util::ToString(screen_scaling) + "x)" , GRAY);

        int scalemx = (int)((double)mouse_x / screen_scaling);
        int scalemy = (int)((double)mouse_y / screen_scaling);
        oss.str(""); oss << "Mouse: " << mouse_x << "," << mouse_y << " (unscaled: " << scalemx << "," << scalemy << ")";
        y+=10; g_game->Print12(m_backbuffer,x,y, oss.str().c_str(), GRAY); 

		y+=10; g_game->Print12(m_backbuffer,x,y,"Quest: " + Util::ToString( g_game->gameState->getActiveQuest() ) + " (" + Util::ToString( g_game->gameState->getQuestCompleted()) + ")" , GRAY);
		y+=10; g_game->Print12(m_backbuffer,x,y,"Stage: " + Util::ToString(g_game->gameState->getPlotStage()), GRAY);
		y+=10; g_game->Print12(m_backbuffer,x,y,"Date:  " + Util::ToString( gameState->stardate.GetFullDateString() ) , GRAY);
		y+=10; g_game->Print12(m_backbuffer,x,y,"Prof:  " + g_game->gameState->getProfessionString() , GRAY);
		y+=10; g_game->Print12(m_backbuffer,x,y,"Fuel:  " + Util::ToString( g_game->gameState->getShip().getFuel() ) , GRAY);
		y+=10; g_game->Print12(m_backbuffer,x,y,"Cred:  " + Util::ToString(g_game->gameState->getCredits()) , GRAY);
		y+=10; g_game->Print12(m_backbuffer,x,y,"Cargo: " + Util::ToString(g_game->gameState->m_ship.getOccupiedSpace()) + "/" + Util::ToString(g_game->gameState->m_ship.getTotalSpace()), GRAY);
		y+= 10;

		//Print out the aliens' attitude toward us:
		/*   PrintDefault(m_backbuffer,0,y,"Attitudes");
		for (int n=1; n<NUM_ALIEN_RACES; n++)
		{
			y+=10;
			PrintDefault(m_backbuffer,0,y, gameState->player->getAlienRaceName(n) + " : "
				+ Util::ToString( gameState->alienAttitudes[n] ));
		}*/
    }



	//vibrate the screen if needed (occurs near a star or flux)
	if (vibration) v = Util::Random(0, vibration); else v = 0;

    //
	//Copy back buffer to the screen (with resolution scaling)
    //
    int cx = (actual_width-scale_width)/2;
    stretch_blit( m_backbuffer, screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, cx, 0, scale_width, scale_height );

	//slow down!
//	rest(1);
}


#pragma region "UI events"

void Game::UpdateKeyboard()
{
	poll_keyboard();
	for (int k = 0; k < 256; k++)
	{
        if (key[k])
        {
	        OnKeyPress(k);

            if (!m_prevKeyState[k])
	        {
	            OnKeyPressed(k);
	        }
        }
        else if (!key[k])
        {
	        if (m_prevKeyState[k])
	        {
		        OnKeyReleased(k);
	        }
        }
	}

	//save key states for release detection
	memcpy(m_prevKeyState,(char*)key,256);
}

void Game::UpdateMouse()
{
	poll_mouse();

	for (int button = 0; button < (m_numMouseButtons); button++)
	{
		if ((mouse_b & (1 << button)) != 0)
		{
			m_mouseButtons[button] = true;
		}
		else
		{
			m_mouseButtons[button] = false;
		}

		if (m_mouseButtons[button] && (!m_prevMouseButtons[button]))
		{
			OnMousePressed(button, mouse_x, mouse_y);

			m_mousePressedLocs[button].x = mouse_x;
			m_mousePressedLocs[button].y = mouse_y;
		}
		else if ((!m_mouseButtons[button]) && m_prevMouseButtons[button])
		{
			OnMouseReleased(button, mouse_x, mouse_y);

			if ((m_mousePressedLocs[button].x == mouse_x) &&
				 (m_mousePressedLocs[button].y == mouse_y))
			{
				OnMouseClick(button,mouse_x,mouse_y);
			}
		}
	}

	//save mouse button states for release detection
	memcpy(m_prevMouseButtons,m_mouseButtons,sizeof(bool)*(m_numMouseButtons+1));

	if ((mouse_x != m_prevMouseX) || (mouse_y != m_prevMouseY))
	{
		OnMouseMove(mouse_x,mouse_y);
		m_prevMouseX = mouse_x;
		m_prevMouseY = mouse_y;
	}

	// mouse wheel
	if (mouse_z > m_prevMouseZ)
	{
		OnMouseWheelUp( mouse_x, mouse_y );
		m_prevMouseZ = mouse_z;
	}
	else
	if (mouse_z < m_prevMouseZ)
	{
		OnMouseWheelDown( mouse_x, mouse_y );
		m_prevMouseZ = mouse_z;
	}

}

void Game::OnKeyPress(int keyCode)
{
	//send keypress event to messagebox
	if (messageBox != NULL)
	{
		//this stops buttons from under the messagebox from being clicked
		if (messageBox->OnKeyPress(keyCode)) return;
	}

	if (!m_pause)
	{
		modeMgr->OnKeyPress(keyCode);
	}
}

void Game::OnKeyPressed(int keyCode)
{
	if (!m_pause) {
		modeMgr->OnKeyPressed(keyCode);
	}
}

void Game::OnKeyReleased(int keyCode)
{
	if (keyCode == KEY_ESC) TogglePauseMenu();

	if (!m_pause)
    {
		modeMgr->OnKeyReleased(keyCode);
	}
}

void Game::toggleShowControls()
{
	showControls = !showControls;
	Event e(showControls? EVENT_SHOW_CONTROLS : EVENT_HIDE_CONTROLS);
	modeMgr->BroadcastEvent(&e);
}

void Game::OnMouseMove(int x, int y)
{
	//send mouse event to pause menu
	if (pauseMenu->isShowing())	{
		if (pauseMenu->OnMouseMove(x,y)) return;
	}

	//send mouse event to messagebox
	if (messageBox != NULL)
	{
		//this stops buttons from under the messagebox from being clicked
		if (messageBox->OnMouseMove(x,y)) return;
	}

	if (!m_pause)
	{
		modeMgr->OnMouseMove(x,y);
	}
}

void Game::OnMouseClick(int button, int x, int y)
{
	//send mouse event to messagebox
	if (messageBox != NULL)
	{
		//this stops buttons from under the messagebox from being clicked
		if (messageBox->OnMouseClick(button, x, y)) return;
	}
	if (!m_pause)
	{
		modeMgr->OnMouseClick(button,x,y);
	}
}

void Game::OnMousePressed(int button, int x, int y)
{
	//send mouse event to messagebox
	if (messageBox != NULL)
	{
		//this stops buttons from under the messagebox from being clicked
		if (messageBox->OnMousePressed(button, x, y)) return;
	}
	if (!m_pause)
	{
		modeMgr->OnMousePressed(button, x, y);
	}
}

void Game::OnMouseReleased(int button, int x, int y)
{
	//send mouse event to pause menu
	if (pauseMenu->isShowing())	{
		if (pauseMenu->OnMouseReleased(button, x, y)) return;
	}

	//send mouse event to messagebox
	if (messageBox != NULL)
	{
		//this stops buttons from under the messagebox from being clicked
		if (messageBox->OnMouseReleased(button, x, y)) return;
	}
	if (!m_pause)
	{
		modeMgr->OnMouseReleased(button, x, y);
	}
}

void Game::OnMouseWheelUp(int x, int y)
{
	if (!m_pause)
	{
		modeMgr->OnMouseWheelUp(x, y);
	}
}

void Game::OnMouseWheelDown(int x, int y)
{
	if (!m_pause)
	{
		modeMgr->OnMouseWheelDown(x, y);
	}
}

#pragma endregion


bool Game::InitializeModules()
{
	bool result = true;

	// STARTUP MODE
	Module *mode_startup = new Module;
	Module *startup = new ModuleStartup;
	mode_startup->AddChildModule(startup);
	modeMgr->AddMode(MODULE_STARTUP, mode_startup, "data/startup/Starflight.ogg");

	// TITLE SCREEN GAME MODE
	Module *mode_titleScreen = new Module;
	Module *titleScreen = new ModuleTitleScreen;
	mode_titleScreen->AddChildModule(titleScreen);
	modeMgr->AddMode(MODULE_TITLESCREEN, mode_titleScreen, "data/startup/Starflight.ogg");

	// CREDITS GAME MODE
	Module *mode_Credits = new Module;
	Module *credits = new ModuleCredits;
	mode_Credits->AddChildModule(credits);
	modeMgr->AddMode(MODULE_CREDITS, mode_Credits, "data/credits/credits.ogg");

	// STARPORT MODE
	Module *mode_starport = new Module;
	Module *starport = new ModuleStarport;
	mode_starport->AddChildModule(starport);
	modeMgr->AddMode(MODULE_STARPORT, mode_starport, "data/starport/starport.ogg");

	// CAPTAIN CREATION MODE
	Module *mode_captainCreation = new Module;
	Module *captainCreation = new ModuleCaptainCreation;
	mode_captainCreation->AddChildModule(captainCreation);
	modeMgr->AddMode(MODULE_CAPTAINCREATION, mode_captainCreation, "data/startup/Starflight.ogg");

	// CAPTAIN'S LOUNGE MODE
	Module *mode_captainsLounge = new Module;
	Module *captainsLounge = new ModuleCaptainsLounge;
	mode_captainsLounge->AddChildModule(captainsLounge);
	modeMgr->AddMode(MODULE_CAPTAINSLOUNGE, mode_captainsLounge, "data/starport/starport.ogg");

	// SHIPCONFIG GAME MODE
	Module *mode_Shipconfig = new Module;
	Module *shipconfig = new ModuleShipConfig;
	mode_Shipconfig->AddChildModule(shipconfig);
	modeMgr->AddMode(MODULE_SHIPCONFIG, mode_Shipconfig, "data/starport/starport.ogg");


	// INTERSTELLAR (HYPERSPACE) TRAVEL GAME MODE
	Module *mode_hyperspace = new Module();
	mode_hyperspace->AddChildModule(new ModuleInterstellar);
	mode_hyperspace->AddChildModule(new ModuleAuxiliaryDisplay);
	mode_hyperspace->AddChildModule(new ModuleControlPanel);
	mode_hyperspace->AddChildModule(new ModuleStarmap);
	mode_hyperspace->AddChildModule(new ModuleTopGUI);
	mode_hyperspace->AddChildModule(new ModuleQuestLog);
	mode_hyperspace->AddChildModule(new ModuleMedical);
	mode_hyperspace->AddChildModule(new ModuleEngineer);
	mode_hyperspace->AddChildModule(new ModuleCargoWindow);
	mode_hyperspace->AddChildModule(new ModuleMessageGUI);
	modeMgr->AddMode(MODULE_HYPERSPACE, mode_hyperspace, "data/spacetravel/spacetravel.ogg");

	// INTERPLANETARY TRAVEL GAME MODE
	Module *mode_interplanetaryTravel = new Module();
	mode_interplanetaryTravel->AddChildModule(new ModuleSolarSystem);
	mode_interplanetaryTravel->AddChildModule(new ModuleAuxiliaryDisplay);
	mode_interplanetaryTravel->AddChildModule(new ModuleControlPanel);
	mode_interplanetaryTravel->AddChildModule(new ModuleStarmap);
	mode_interplanetaryTravel->AddChildModule(new ModuleTopGUI);
	mode_interplanetaryTravel->AddChildModule(new ModuleQuestLog);
	mode_interplanetaryTravel->AddChildModule(new ModuleMedical);
	mode_interplanetaryTravel->AddChildModule(new ModuleEngineer);
	mode_interplanetaryTravel->AddChildModule(new ModuleCargoWindow);
	mode_interplanetaryTravel->AddChildModule(new ModuleMessageGUI);
	modeMgr->AddMode(MODULE_INTERPLANETARY, mode_interplanetaryTravel, "data/spacetravel/spacetravel.ogg");

	// PLANET ORBIT GAME MODE
	//due to OpenGL rendering, 2D overlays will not work here
	Module *mode_orbit = new Module();
	mode_orbit->AddChildModule(new ModulePlanetOrbit);
	mode_orbit->AddChildModule(new ModuleAuxiliaryDisplay);
	mode_orbit->AddChildModule(new ModuleControlPanel);
	//mode_orbit->AddChildModule(new ModuleTopGUI);
	mode_orbit->AddChildModule(new ModuleMessageGUI);
	modeMgr->AddMode(MODULE_ORBIT, mode_orbit, "data/spacetravel/spacetravel.ogg");

	// PLANET SURFACE MODE
	Module *mode_planet = new Module();
	mode_planet->AddChildModule(new ModulePlanetSurface);
	mode_planet->AddChildModule(new ModuleCargoWindow);
	modeMgr->AddMode(MODULE_SURFACE, mode_planet, "data/planetsurface/planetsurface.ogg");

	// CREW ASSIGNMENT GAME MODE
	Module *mode_crew = new Module;
	Module *crewAssignment = new ModuleCrewHire();
	mode_crew->AddChildModule( crewAssignment );
	modeMgr->AddMode(MODULE_CREWBUY, mode_crew, "data/starport/starport.ogg");

	// BANK MODULE
	Module *mode_bank = new Module;
	Module *bank = new ModuleBank();
	mode_bank->AddChildModule(bank);
	modeMgr->AddMode(MODULE_BANK, mode_bank, "data/starport/starport.ogg");

	// TRADE DEPOT MODULE
	Module *mode_tradedepot = new Module;
	Module *tradedepot = new ModuleTradeDepot;
	mode_tradedepot->AddChildModule(tradedepot);
	g_game->modeMgr->AddMode(MODULE_TRADEDEPOT, mode_tradedepot, "data/starport/starport.ogg");

	// GAME OVER MODULE
	Module *mode_gameover = new Module;
	Module *gameOver = new ModuleGameOver;
	mode_gameover->AddChildModule(gameOver);
	g_game->modeMgr->AddMode(MODULE_GAMEOVER, mode_gameover, "");

	// CANTINA MODULE
	Module *mode_cantina = new Module;
	Module *cantina = new ModuleCantina;
	mode_cantina->AddChildModule(cantina);
	g_game->modeMgr->AddMode(MODULE_CANTINA, mode_cantina, "data/starport/starport.ogg");
	g_game->modeMgr->AddMode(MODULE_RESEARCHLAB, mode_cantina, "data/starport/starport.ogg");
	g_game->modeMgr->AddMode(MODULE_MILITARYOPS, mode_cantina, "data/starport/starport.ogg");

	// ALIEN ENCOUNTER MODULE
	Module *mode_encounter = new Module;
	mode_encounter->AddChildModule(new ModuleEncounter);
	mode_encounter->AddChildModule(new ModuleTopGUI);
	mode_encounter->AddChildModule(new ModuleControlPanel);
	mode_encounter->AddChildModule(new ModuleCargoWindow);
	g_game->modeMgr->AddMode(MODULE_ENCOUNTER, mode_encounter, "data/encounter/combat.ogg");

	// SETTINGS GAME MODE
	Module *mode_Settings = new Module;
	Module *settings = new ModuleSettings;
	mode_Settings->AddChildModule(settings);
	modeMgr->AddMode(MODULE_SETTINGS, mode_Settings, "data/startup/Starflight.ogg");

	return result;
}


#pragma region "Text output"

void Game::PrintDefault(BITMAP *dest,int x,int y, std::string text,int color)
{
	textprintf_ex(dest,font,x,y,color,-1, text.c_str());
}

void Game::Print(BITMAP *dest, ALFONT_FONT *_font, int x,int y,std::string text, int color, bool shadow)
{
	if (shadow) {
		alfont_textprintf_ex(dest, _font, x+2, y+2, BLACK, -1, text.c_str());
	}
	alfont_textprintf_ex(dest, _font, x, y, color, -1, text.c_str());
}

void Game::Print12(BITMAP *dest, int x,int y,std::string text, int color, bool shadow)
{
	Print(dest, font12, x, y, text, color, shadow);
}

void Game::Print18(BITMAP *dest, int x,int y,std::string text, int color, bool shadow)
{
	Print(dest, font18, x, y, text, color, shadow);
}
void Game::Print20(BITMAP *dest, int x,int y,std::string text, int color, bool shadow)
{
	Print(dest, font20, x, y, text, color, shadow);
}
void Game::Print22(BITMAP *dest, int x,int y,std::string text, int color, bool shadow)
{
	Print(dest, font22, x, y, text, color, shadow);
}
void Game::Print24(BITMAP *dest, int x,int y,std::string text, int color, bool shadow)
{
	Print(dest, font24, x, y, text, color, shadow);
}
void Game::Print32(BITMAP *dest, int x,int y,std::string text, int color, bool shadow)
{
	Print(dest, font32, x, y, text, color, shadow);
}

/**
This function tracks all printed messages, each containing a timestamp to prevent messages from 
printing out repeatedly, which occurs frequently in state-based timed sections of code that is 
called repeatedly, where we don't want messages printing repeatedly. Default delay of -1 causes
message to print only once (until ScrollBox is cleared). delay of 0 forces printout.
 **/
void Game::printout(ScrollBox::ScrollBox *scroll, string str, int color, long delay)
{
	bool found = false;

	TimedText message = {str,color,globalTimer.getTimer()+delay};

	//do we care about repeating messages? -1 = one-time only, 0 = always, n = ms delay
	if (delay == 0)
	{
		//just print it without remembering the message
		scroll->Write(message.text, message.color);
	}
	else {
		//scan timestamps of printed messages to see if ready to print again
		for (vector<TimedText>::iterator mess = messages.begin(); mess != messages.end(); ++mess)
		{
			//text found in vector?
			if (mess->text == message.text)
			{
				found = true;

				//print-once code
				if (delay == -1) 
                {
                    mess->delay = -1;
                }

				//ready to print again?
				else if (globalTimer.getTimer() > mess->delay) 
                {
					//print text
					scroll->Write(message.text, message.color);
					//reset delay timer
					mess->delay = globalTimer.getTimer()+delay;
				}
				break;
			}
		}
	}

	//text not found, add to vector and print
	if (!found) 
    {
		if (delay == -1) message.delay = -1;
		messages.push_back(message);
		scroll->Write(message.text, message.color);
	}
}

//will print a message taking into account that a dead officer is replaced by the captain
//and following a color convention for each msgtype.
void Game::PrintMsg(MsgType msgtype, OfficerType officertype, std::string msg, int delay)
{
	std::string buf(msg), s;
	int color = MsgColors[msgtype];
	Officer *tempOfficer = gameState->getCurrentOfficerByType(officertype);

	s = tempOfficer->getLastName() + "-> ";

	if (tempOfficer != gameState->officerCap){
		switch(msgtype){
			case MSG_INFO    : s += "Sir, ";                     break;
			case MSG_ALERT   : s += "Captain! ";                 break;
			case MSG_ERROR   : s += "But Sir, ";                 break;
			case MSG_ACK     : s += "Aye, Sir, ";                break;
			case MSG_FAILURE : s += "I'm sorry, captain, ";      break;

			case MSG_SUCCESS :
			case MSG_TASK_COMPLETED:
			case MSG_SKILLUP :                                   break;

			default: ASSERT(0);
		}
	}

	char c = s[s.size()-2];

	if ( c == '.' || c == '!' || c == '?' || c == '>' ){
		if (islower(buf[0])) buf[0] = toupper(buf[0]);
	}else{
		if (isupper(buf[0])) buf[0] = tolower(buf[0]);
	}

	s += buf;

	printout(g_scrollbox, s, color, delay);
}
#pragma endregion

