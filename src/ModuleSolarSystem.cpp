/*
	STARFLIGHT - THE LOST COLONY
	ModuleSolarSystem.cpp - Handles interstellar space travel in the main window
	Author: J.Harbour
	Date: January, 2007
*/

#pragma region HEADER

#include <sstream>
#include <string>
#include <exception>
#include "env.h"
#include "ModuleSolarSystem.h"
#include "Button.h"
#include "ModeMgr.h"
#include "GameState.h"
#include "Util.h"
#include "Events.h"
#include "Game.h"
#include "ModuleControlPanel.h"
#include "Label.h"
#include "QuestMgr.h"
#include "PauseMenu.h"

using namespace std;

//#define GUI_AUX_BMP                      0        /* BMP  */
//#define GUI_VIEWER_BMP                   1        /* BMP  */
//#define IP_TILES_BMP                     2        /* BMP  */
//#define IS_TILES_BMP                     3        /* BMP  */
//#define SHIELD_TGA                       4        /* BMP  */
//#define STARFIELD_BMP                    5        /* BMP  */


//scroller properties
#define PLANET_SCROLL_X		0
#define PLANET_SCROLL_Y		0
#define PLANET_SCROLL_WIDTH	SCREEN_WIDTH
#define PLANET_SCROLL_HEIGHT SCREEN_HEIGHT
#define PLANETTILESIZE 256
#define PLANETTILESACROSS 100
#define PLANETTILESDOWN 100

const int FlyingHoursBeforeSkillUp = 168;

ModuleSolarSystem::ModuleSolarSystem(void){}
ModuleSolarSystem::~ModuleSolarSystem(void){}

#pragma endregion

#pragma region INPUT

void ModuleSolarSystem::OnKeyPress(int keyCode)
{
    //Note: fuel consumption in a star system should be negligible since its the impulse engine
    //whereas we're using the hyperspace engine outside the system
	switch (keyCode) {
        case KEY_RIGHT:
        case KEY_D:
			flag_nav = false;
			ship->TurnRight();
			break;
		case KEY_LEFT:
        case KEY_A:
			flag_nav = false;
			ship->TurnLeft();
			break;
		case KEY_DOWN:
        case KEY_S:
			flag_nav = false;
			ship->ApplyBraking();
			break;
		case KEY_UP:
        case KEY_W:
			flag_nav = flag_thrusting = true;
			ship->ApplyThrust();
			g_game->gameState->m_ship.ConsumeFuel();
			break;
		case KEY_Q:
			flag_nav = true;
			if (!flag_thrusting) ship->ApplyBraking();
			ship->Starboard();
			g_game->gameState->m_ship.ConsumeFuel();
			break;
		case KEY_E:
			flag_nav = true;
			if (!flag_thrusting) ship->ApplyBraking();
			ship->Port();
			g_game->gameState->m_ship.ConsumeFuel();
			break;

	}
}


void ModuleSolarSystem::OnKeyPressed(int keyCode){}

void ModuleSolarSystem::OnKeyReleased(int keyCode)
{
	Module::OnKeyReleased(keyCode);

	switch (keyCode) {

		//reset ship anim frame when key released
		case KEY_LEFT:
		case KEY_RIGHT:
		case KEY_DOWN:
        case KEY_A:
        case KEY_D:
        case KEY_S:
			flag_nav = false;
            ship->Cruise();
			break;

		case KEY_UP:
        case KEY_W:
			flag_nav = flag_thrusting = false;
			ship->ApplyBraking();
			ship->Cruise();
			break;

		case KEY_Q:
		case KEY_E:
			flag_nav = false;
			ship->ApplyBraking();
			ship->Cruise();	
			break;

	}
}

void ModuleSolarSystem::OnMouseMove(int x, int y)
{
	Module::OnMouseMove(x,y);
	text->OnMouseMove(x,y);

	//*** need to add a flag that skips all this time consuming code unless mouse is actually in
	// the aux window for the planet name tooltip to be useful

	//check if mouse is over a planet

//debug
	
	std::string dian4 = "";

	for (int i = 0; i < star->GetNumPlanets(); i++)  {
		int planet = planets[i].tilenum;
		if (planet > 0) {
			//the various negative offsets after the parenthetised expressions are mere empirical tweaks, so don't expect too much of them
			if(x > (asx + planets[i].tilex * 2.3)-4 - planets[i].radius  && x < (asx + planets[i].tilex * 2.3)-2 + planets[i].radius
			&& y > (asy + planets[i].tiley * 2.3)-2 - planets[i].radius &&  y < (asy + planets[i].tiley * 2.3) + planets[i].radius){
				planet_label->SetX( x + 10);
				planet_label->SetY( y );
				planet_label->SetText( star->GetPlanetByID(planets[i].planetid)->name );	//jjh

				dian4 = star->GetPlanetByID(planets[i].planetid)->name ;

				m_bOver_Planet = true;
				break;
			}else{
				m_bOver_Planet = false;
			}
		}
	}
	//check if mouse is over a star
	if(m_bOver_Planet == false){
		int systemCenterTileX = scroller->GetTilesAcross() / 2;
		int systemCenterTileY = scroller->GetTilesDown() / 2;
		if(x > (asx + systemCenterTileX * 2.3)-6  && x < (asx + systemCenterTileX * 2.3)+6
		&& y > (asy + systemCenterTileY * 2.3)-6 &&  y < (asy + systemCenterTileY * 2.3)+6){
			planet_label->SetX( x + 10);
			planet_label->SetY( y );
			planet_label->SetText( star->name );
			m_bOver_Planet = true;
		}else{
			m_bOver_Planet = false;
		}
	}
}

void ModuleSolarSystem::OnMouseClick(int button, int x, int y)
{
	Module::OnMouseClick(button,x,y);
	text->OnMouseClick(button,x,y);
}

void ModuleSolarSystem::OnMousePressed(int button, int x, int y)
{
	Module::OnMousePressed(button, x, y);
	text->OnMousePressed(button,x,y);
}

void ModuleSolarSystem::OnMouseReleased(int button, int x, int y)
{
	Module::OnMouseReleased(button, x, y);
	text->OnMouseReleased(button,x,y);
}

void ModuleSolarSystem::OnMouseWheelUp(int x, int y)
{
	Module::OnMouseWheelUp(x, y);
	text->OnMouseWheelUp(x,y);
}

void ModuleSolarSystem::OnMouseWheelDown(int x, int y)
{
	Module::OnMouseWheelDown(x, y);
	text->OnMouseWheelDown(x,y);
}

#pragma endregion

void ModuleSolarSystem::OnEvent(Event * event)
{
	Ship ship;
	std::string escape = "";

	int evtype = event->getEventType();
	switch(evtype) {
		case 0xDEADBEEF + 2: //save game
			g_game->gameState->AutoSave();
			g_game->printout(text, "<Game Saved>", WHITE, 5000);
			break;
		case 0xDEADBEEF + 3: //load game
			g_game->gameState->AutoLoad();
			break;
		case 0xDEADBEEF + 4: //quit game
			g_game->setVibration(0);
			escape = g_game->getGlobalString("ESCAPEMODULE");
			g_game->modeMgr->LoadModule(escape);
			break;

		case EVENT_CAPTAIN_LAUNCH:   g_game->printout(text, nav + "Sir, we are not on a planet.",       BLUE,8000); break;
		case EVENT_CAPTAIN_DESCEND:  g_game->printout(text, nav + "Sir, we are not orbiting a planet.", BLUE,8000); break;
		//case EVENT_CAPTAIN_LOG:      g_game->printout(text, "<Log planet not yet implemented>", YELLOW,8000);       break;
		case EVENT_CAPTAIN_QUESTLOG: break;

		case EVENT_SCIENCE_SCAN:     g_game->printout(text, sci + "Sir, we are not near any planets or vessels.", BLUE,8000); break;
		case EVENT_SCIENCE_ANALYSIS: g_game->printout(text, sci + "Sir, I have not scanned anything.",            BLUE,8000); break;

		case EVENT_NAVIGATOR_ORBIT:      flag_DoOrbit      = true; break;
		case EVENT_NAVIGATOR_DOCK:       flag_DoDock       = true; break;
		case EVENT_NAVIGATOR_HYPERSPACE: flag_DoHyperspace = true; break;

		case EVENT_TACTICAL_COMBAT:  g_game->printout(text, tac + "Sir, we are not in range of any other ships.", BLUE,8000); break;
		case EVENT_TACTICAL_SHIELDS: g_game->printout(text, tac + "Sir, we are not in combat.", BLUE,8000); break;
		case EVENT_TACTICAL_WEAPONS: g_game->printout(text, tac + "Sir, we are not in combat.", BLUE,8000); break;

		case EVENT_ENGINEER_REPAIR:  break;
		case EVENT_ENGINEER_INJECT:  g_game->gameState->getShip().injectEndurium(); break;

		case EVENT_COMM_HAIL:        g_game->printout(text, com + "We are not in range of any other ships.", BLUE,8000); break;
		case EVENT_COMM_STATEMENT:   g_game->printout(text, com + "We are not communicating with anyone!",   BLUE,8000); break;
		case EVENT_COMM_QUESTION:    g_game->printout(text, com + "We are not communicating with anyone!",   BLUE,8000); break;
		case EVENT_COMM_POSTURE:     g_game->printout(text, com + "We are not communicating with anyone!",   BLUE,8000); break;
		case EVENT_COMM_TERMINATE:   g_game->printout(text, com + "We are not communicating with anyone!",   BLUE,8000); break;
		case EVENT_COMM_DISTRESS:    g_game->gameState->m_ship.SendDistressSignal(); break;

		case EVENT_DOCTOR_EXAMINE:	break;
		case EVENT_DOCTOR_TREAT:	break;

		default: break;
	}
}

#pragma region INIT_CLOSE

void ModuleSolarSystem::Close()
{
	debug << "*** Interplanetary Closing" << endl << endl;

	try {
		if (text!=NULL) { delete text; text = NULL;}
        if (scroller!=NULL) { delete scroller; scroller=NULL; }
        if (ship!=NULL) { delete ship; ship=NULL; }
		if (planet_label!=NULL) { delete planet_label; planet_label = NULL; }

        for (int i=0; i<MAX_PLANETS; i++)
        {
            if (this->planets[i].pbody)
            {
                delete this->planets[i].pbody;
                this->planets[i].pbody = NULL;
            }
        }

	}
	catch(std::exception e) {
		debug << e.what() << endl;
	}
	catch(...) {
		debug << "Unhandled exception in InterplanetaryTravel::Close" << endl;
	}

}

bool ModuleSolarSystem::Init()
{
	g_game->SetTimePaused(false);	//game-time normal in this module.

	debug << "  SolarSystem Initialize" << endl;

	//enable the Pause Menu
	g_game->pauseMenu->setEnabled(true);


	//reset flags
	flag_nav = flag_thrusting = false;

	planetFound = 0;
	distance = 0.0f;
	flag_DoOrbit = false;
	flag_DoHyperspace = false;
	flag_DoDock = false;
	asx = 0; asy = 0;
	distressSignal = false;
	burning = 0;
	g_game->setVibration(0);
	m_bOver_Planet = false;
	planet_label = new Label("",0,0,100,22,ORANGE,g_game->font18);


	//create the ScrollBox for message window
	static int gmx = (int)g_game->getGlobalNumber("GUI_MESSAGE_POS_X");
	static int gmy = (int)g_game->getGlobalNumber("GUI_MESSAGE_POS_Y");
	static int gmw = (int)g_game->getGlobalNumber("GUI_MESSAGE_WIDTH");
	static int gmh = (int)g_game->getGlobalNumber("GUI_MESSAGE_HEIGHT");

	text = new ScrollBox::ScrollBox(g_game->font20, ScrollBox::SB_TEXT,
		gmx + 38, gmy + 18, gmw - 38, gmh - 32, 999);
	text->DrawScrollBar(false);

	//point global scrollbox to local one in this module for sub-module access
	g_game->g_scrollbox = text;


	//create tile scroller object
	this->scroller = new TileScroller();
	this->scroller->SetTileSize(PLANETTILESIZE,PLANETTILESIZE);
	this->scroller->SetTileImageColumns(10); //used to be 9x1
    this->scroller->SetTileImageRows(1);
	this->scroller->SetRegionSize(PLANETTILESACROSS,PLANETTILESDOWN);


    //switching to rendered planets but load ip_tiles for convenience
    //10 planets max at 256x256, 10 columns

    BITMAP *tileImage = (BITMAP*)load_bitmap("data/spacetravel/ip_tiles.bmp",NULL);
    //BITMAP *tileImage = (BITMAP*)create_bitmap(256*10, 256*1);
	if (!tileImage) 
    {
        g_game->fatalerror("SolarSystem: Error creating tilescroller source image");
		return false;
	}
	this->scroller->SetTileImage(tileImage);



	if (!this->scroller->CreateScrollBuffer(PLANET_SCROLL_WIDTH, PLANET_SCROLL_HEIGHT)) {
		g_game->message("Interplanetary: Error creating scroll buffer");
		return false;
	}

	//create the ship sprite
	this->ship = new PlayerShipSprite();
	this->ship->AllStop();


	//try to read star system data...
	this->star = g_game->dataMgr->GetStarByID(g_game->gameState->player->currentStar);
	if (!this->star) 
    {
		g_game->message("Interplanetary: Error locating star system!");
		return false;
	}

	//create a tile map of this star system
	if (!LoadStarSystem(g_game->gameState->player->currentStar)) 
    {
        debug << "Error loading star system: ID = " << g_game->gameState->player->currentStar << endl;
        return false;
    }

	//set player's location
	this->scroller->SetScrollPosition( g_game->gameState->player->posSystem);


	//notify questmgr that star system has been entered
	g_game->questMgr->raiseEvent(10, g_game->gameState->player->currentStar);

	//shortcuts to crew last names to simplify code
	com = g_game->gameState->getCurrentCom()->getLastName() + "-> ";
	sci = g_game->gameState->getCurrentSci()->getLastName() + "-> ";
	nav = g_game->gameState->getCurrentNav()->getLastName() + "-> ";
	tac = g_game->gameState->getCurrentTac()->getLastName() + "-> ";
	eng = g_game->gameState->getCurrentEng()->getLastName() + "-> ";
	doc = g_game->gameState->getCurrentDoc()->getLastName() + "-> ";

	return true;
}

#pragma endregion

bool ModuleSolarSystem::checkSystemBoundary(int x,int y)
{
	int leftEdge = 0;
	int rightEdge = scroller->GetTilesAcross() * scroller->GetTileWidth();
	int topEdge = 0;
	int bottomEdge = scroller->GetTilesDown() * scroller->GetTileHeight();

	//if ship reaches edge of system, exit to interstellar space
	if (x < leftEdge || x > rightEdge || y < topEdge || y > bottomEdge) return false;

	return true;
}

void ModuleSolarSystem::Update()
{
	static Timer timerHelp;
	std::ostringstream s;

	//update the ship's position based on velocity
	float fx = g_game->gameState->player->posSystem.x + (ship->GetVelocityX() * 6);
	float fy = g_game->gameState->player->posSystem.y + (ship->GetVelocityY() * 6);

	//exit star system when edge is reached
	if (!checkSystemBoundary((int)fx,(int)fy))
	{
		ship->AllStop();
		flag_DoHyperspace = true;
		return;
	}

	//store ship position
	g_game->gameState->player->posSystem.x = fx;
	g_game->gameState->player->posSystem.y = fy;

	//update scrolling and draw tiles on the scroll buffer
	scroller->SetScrollPosition(fx, fy);
	scroller->UpdateScrollBuffer();

	//check if any planet is located near ship
	checkShipPosition();


	//display some text
	//text->Clear();
	//alfont_set_font_size(g_game->font10, 20);

	//reset vibration danger value
	g_game->setVibration(0);

	//display any planet under ship
	if (planetFound) {
		if (planet->name.length() > 0)
		{
			s << nav << "Captain, we are in orbital range of " << planet->name << ".";
		}
		else
		{
			s << nav << "Captain, planet " << planet->name << " is not in our data banks.";
		}
		g_game->printout(text, s.str() + " Awaiting order to establish orbit.", ORANGE, 15000);
	}
	//getting too close to the star?
	else if (burning) {
		if (burning > 240) {
			g_game->setVibration(20);
			g_game->printout(text, "AAARRRRRGGGGHHHHH!!!", YELLOW,500);
			ship->AllStop();
			if ( Util::ReentrantDelay(2000)) {
				g_game->setVibration(0);
				g_game->modeMgr->LoadModule(MODULE_GAMEOVER);
				return;
			}
		}
		else if (burning > 160) {
			g_game->setVibration(14);
			g_game->printout(text, sci + "Hull temperature is critical!", RED,8000);
		}
		else if (burning > 80) {
			g_game->setVibration(10);
			g_game->printout(text, sci + "Hull temperature is rising!", ORANGE,8000);
		}
		else if (burning > 2) {
			g_game->setVibration(6);
			g_game->printout(text, sci + "Captain! We're getting too close!", ORANGE,8000);
		}
		else if (burning == 2) {
			g_game->setVibration(4);
			g_game->printout(text, nav + "Sir, we must keep our distance from the sun.", LTGREEN,8000);
		}
		else {
			g_game->setVibration(2);
			g_game->printout(text, nav + "Captain, we should steer clear of the sun.", LTGREEN,8000);
		}

	}


	//when player tells navigator to orbit planet, we want to display a message before immediately
	//launching the planet orbit module. this displays the message and pauses for 2 seconds.
	if (flag_DoOrbit) {
      checkShipPosition();
      if (!planetFound) {
		   g_game->printout(text, nav + "Unable to comply. Nothing to orbit here.", ORANGE,8000);
		   if ( Util::ReentrantDelay(2000)) {
			   flag_DoOrbit = false;
		   }
      } else {
		   g_game->printout(text, nav + "Aye, captain.", ORANGE,8000);
		   text->ScrollToBottom();
		   if ( Util::ReentrantDelay(2000)) {
			   g_game->printout(text, nav + "Entering orbital trajectory.", ORANGE,2000);
			   g_game->modeMgr->LoadModule(MODULE_ORBIT);
				return;
		   }
      }
	}


	//player orders navigator to exit system back into hyperspace   
	//use delay mechanism to show message before lauching module
    //this also occurs if player reaches edge of star system
	if (flag_DoHyperspace)
	{
		Ship ship = g_game->gameState->getShip();    
		//float fuelUsage = 0.025;

		if(g_game->gameState->player->hasHyperspacePermit() == false){
			g_game->printout(text, nav + "Captain, we can't enter hyperspace without a hyperspace permit.", ORANGE,30000);
			if (Util::ReentrantDelay(2000)) {
				flag_DoHyperspace = false;
			}
		}else if (planetFound) {
			g_game->printout(text, nav + "Captain, we can't enter hyperspace due to the nearby planet's gravity well.", ORANGE,8000);
			if (Util::ReentrantDelay(2000)) {
				flag_DoHyperspace = false;
			}
		} else if (ship.getFuel() <= 0.0f)
		{
			g_game->printout(text, nav + "Sir, we do not have enough fuel to enter hyperspace!", ORANGE, 10000);
			if (Util::ReentrantDelay(2000)) {
				flag_DoHyperspace = false;
			}
		}else {
			//okay we're not near a planet
			g_game->printout(text, nav + "Yes, sir! Engaging hyperspace engine.", ORANGE,8000);
			if (Util::ReentrantDelay(2000))
			{
                if (g_game->gameState->getShip().getFuel() >= 0.01f) //1% of fuel required
                {
				    g_game->gameState->m_ship.ConsumeFuel(20); //hyperspace engine consumes 20 units    
				    g_game->modeMgr->LoadModule(MODULE_HYPERSPACE);
				    return;
                }
                else
                {
                    g_game->printout(text, nav + "We do not have enough fuel to enter hyperspace!", RED, 8000);
                }
			}
		}
	}

	if (flag_DoDock)
	{
		//display any planet under ship
		if (planetFound) {
			//planet id #8 = Myrrdan in the database
			if (g_game->gameState->player->currentPlanet == 8){
				//okay we're near myredan - however it's spelled
				g_game->printout(text, nav + "Yes, sir! Docking with Starport.", ORANGE,8000);
				text->ScrollToBottom();
				if (Util::ReentrantDelay(1000)) 
				{
					g_game->modeMgr->LoadModule(MODULE_STARPORT);
					return;
				}
			}
			else
			{
				//okay we're near a planet without a starport
				g_game->printout(text, nav + "Sorry sir! There are no Starports in the vicinity.", ORANGE,8000);
				text->ScrollToBottom();
				flag_DoDock = false;
			}
		}
		else
		{
			//okay we're not near a planet
			g_game->printout(text, nav + "Sorry sir! We are not close enough to a planet to scan for Starports.", ORANGE,8000);
			text->ScrollToBottom();
			flag_DoDock = false;
		}
	}

    //check fuel level
	if(g_game->gameState->getShip().getFuel() <= 0.00f)
	{
        g_game->gameState->m_ship.injectEndurium();
	}

	//increase navigation skill every FlyingHoursBeforeSkillUp hours spent in space (the speed check is there to prevent the obvious abuse)
	Officer *currentNav = g_game->gameState->getCurrentNav();
	if (currentNav->CanSkillCheck() && ship->GetCurrentSpeed() > 0.0){

		currentNav->FakeSkillCheck();
		currentNav->attributes.extra_variable++;

		if (currentNav->attributes.extra_variable >= FlyingHoursBeforeSkillUp) {
			currentNav->attributes.extra_variable = 0;
			if(currentNav->SkillUp(SKILL_NAVIGATION))
				g_game->printout(text, nav + "I think I'm getting better at this.", PURPLE, 5000);
		}
	}

	//slow down the ship automatically
	if (!flag_nav)	ship->ApplyBraking();

	//refresh messages
	text->ScrollToBottom();

}

void ModuleSolarSystem::Draw()
{
	static bool help1 = true;

	//draw the scrolling view
	scroller->DrawScrollWindow(g_game->GetBackBuffer(), PLANET_SCROLL_X, PLANET_SCROLL_Y, PLANET_SCROLL_WIDTH, PLANET_SCROLL_HEIGHT);

	//draw the ship
	ship->Draw(g_game->GetBackBuffer());

	//draw message window gui
	text->Draw(g_game->GetBackBuffer());

	//redraw the mini map
	updateMiniMap();

	//display tutorial help messages for beginners
	if ( (!g_game->gameState->firstTimeVisitor || g_game->gameState->getActiveQuest() > 10) ) help1 = false;
	if ( help1 )
	{
		help1 = false;
		string str = "You are now travelling in the star system. To move the ship use the same keys you used in the starport.";
		g_game->ShowMessageBoxWindow("",  str, 400, 300, YELLOW, 10, 200, false);
	}
	//JJH - added CrossModuleAngle so that ship's heading stays consistent between entering/leaving systems. 
	//same mod in ModuleInterstellar and some changes in PlayerShipSprite.  
	g_game->CrossModuleAngle = ship->GetRotationAngle();	

    if (g_game->getGlobalBoolean("DEBUG_MODE") && g_game->getGlobalBoolean("DEBUG_CORE"))
    {
	    int x = 910, y = 90;
	    g_game->Print12(g_game->GetBackBuffer(), x, y, "flag_nav: " + Util::ToString(flag_nav), GRAY1);
	    y+=10;g_game->Print12(g_game->GetBackBuffer(), x, y, "flag_thrusting: " + Util::ToString(flag_thrusting), GRAY1);
	    y+=10;g_game->Print12(g_game->GetBackBuffer(), x, y, "velocity: " + Util::ToString(ship->GetVelocityX()) + "," + Util::ToString(ship->GetVelocityY()), GRAY1);
	    y+=10;g_game->Print12(g_game->GetBackBuffer(), x, y, "speed: " + Util::ToString(ship->GetCurrentSpeed()), GRAY1);
	    y+=10;g_game->Print12(g_game->GetBackBuffer(), x, y, "planetFound: " + Util::ToString(planetFound), GRAY1);
	    y+=10;g_game->Print12(g_game->GetBackBuffer(), x, y, "navcounter: " + Util::ToString(g_game->gameState->getCurrentNav()->attributes.extra_variable), GRAY1);
		y+=10;g_game->Print12(g_game->GetBackBuffer(), x, y, "angle: " + Util::ToString(ship->GetRotationAngle()), GRAY1);   
    }
}


void ModuleSolarSystem::checkShipPosition()
{

	//break up window into an evenly-divisible grid of tiles
	//note the height is based on top of GUI, since that is where scroller stops
	int windowWidth = (SCREEN_WIDTH / scroller->GetTileWidth()) * scroller->GetTileWidth();
	int windowHeight = (540 / scroller->GetTileHeight()) * scroller->GetTileHeight();

	//adjust for ship's position at center of main window
	int actualx = (int)(g_game->gameState->player->posSystem.x + windowWidth / 2);
	int actualy = (int)(g_game->gameState->player->posSystem.y + windowHeight / 2);

	//get tile based on ship's coordinates
	tilex = actualx / scroller->GetTileWidth();
	tiley = actualy / scroller->GetTileHeight();
	tilenum = scroller->GetTile(tilex,tiley);


	//is ship burning up in the star?
	if (scroller->GetTile(tilex,tiley) == 1)
	{
		burning++;
	}
	else {
		//calculate distance from ship to star
		int starTileX = scroller->GetTilesAcross() / 2;
		int starTileY = scroller->GetTilesDown() / 2;
		distance = sqrt( (float) pow( (float)(starTileX - tilex), 2) + (float) pow( (float)(starTileY - tiley), 2) );
		if (distance > 2.0f)
			burning = 0;
		else if (distance > 1.5f)
			burning = 1;
		else if (distance > 0.5f)
			burning = 2;
	}


	//see if ship is over planet tile
	planetFound = 0;
	for (int i=0; i<10; i++) {

		if (planetFound) break;

		//check tilex,tiley,and tilenum for a planet match
		if (tilex == planets[i].tilex && tiley == planets[i].tiley && tilenum == planets[i].tilenum)
		{
			planet = star->GetPlanetByID(planets[i].planetid);
			if (planet) 
            {
				planetFound = 1;

				//store current planet in global player object
				g_game->gameState->player->currentPlanet = planets[i].planetid;

			}
		}

	}
	if (!planetFound)
		g_game->gameState->player->currentPlanet = -1;
}

void ModuleSolarSystem::updateMiniMap()
{
	//get AUX_SCREEN gui values from globals
	asx = (int)g_game->getGlobalNumber("AUX_SCREEN_X");
	asy = (int)g_game->getGlobalNumber("AUX_SCREEN_Y");
	static int asw = (int)g_game->getGlobalNumber("AUX_SCREEN_WIDTH");
	static int ash = (int)g_game->getGlobalNumber("AUX_SCREEN_HEIGHT");

	//clear aux window
	rectfill(g_game->GetBackBuffer(), asx, asy, asx + asw, asy + ash , makecol(0,0,0));

	//draw ellipses representing planetary orbits
	int rx,ry,cx,cy;
	for (int i = 0; i < star->GetNumPlanets(); i++)  
    {
		planet = star->GetPlanet(i);
		if (planet) 
        {
			cx = asx + asw / 2;
			cy = asy + ash / 2;
			rx = (int)( (2 + i) * 8.9 );
			ry = (int)( (2 + i) * 8.9 );
			ellipse(g_game->GetBackBuffer(), cx, cy, rx, ry, makecol(12,12,24));
		}
	}

	//draw the star in aux window
	int systemCenterTileX = scroller->GetTilesAcross() / 2;
	int systemCenterTileY = scroller->GetTilesDown() / 2;

	int color;
	switch(star->spectralClass) {
		case SC_A: color = WHITE; break;
		case SC_B: color = LTBLUE; break;
		case SC_F: color = LTYELLOW; break;
		case SC_G: color = YELLOW; break;
		case SC_K: color = ORANGE; break;
		case SC_M: color = RED; break;
		case SC_O: color = BLUE; break;
		default: color = ORANGE; break;
	}

    //draw the sun at the center
	float starx = (int)(asx + systemCenterTileX * 2.3);
	float stary = (int)(asy + systemCenterTileY * 2.3);
	circlefill(g_game->GetBackBuffer(), starx, stary, 8, color);

	//draw planets in aux window
	color = 0;
	int planet=-1, px=0, py=0;
	for (int i = 0; i < star->GetNumPlanets(); i++)  
    {
			planet = planets[i].tilenum;
			if (planet > 0) 
            {
				switch(planet) 
                {
					//case 1: color = makecol(255,182,0);	planets[i].radius = 8;		break; //sun
					case 2: color = makecol(100,0,100);	planets[i].radius = 5;		break; //gas giant
					case 3: color = makecol(160,12,8);	planets[i].radius = 2;		break; //molten
					case 4: color = makecol(200,200,255); planets[i].radius = 2;	break; //frozen
					case 5: color = makecol(30,100,240); planets[i].radius = 2;		break; //oceanic
					case 6: color = makecol(134,67,30);	planets[i].radius = 2;		break; //rocky
					case 7: color = makecol(95,93,93);	planets[i].radius = 1;		break; //asteroid
					case 8: color = makecol(55,147,84);	planets[i].radius = 2;		break; //acidic
					default: color = makecol(90,90,90);	planets[i].radius = 1;		break; //none
				}
				px = (int)(asx + planets[i].tilex * 2.28);
				py = (int)(asy + planets[i].tiley * 2.28);
				circlefill(g_game->GetBackBuffer(), px, py, planets[i].radius, color);
			}
		}

	//draw text
	if(m_bOver_Planet == true)
    {
		planet_label->Refresh();
		planet_label->Draw(g_game->GetBackBuffer());		
	}

	//draw player's location on minimap
	float fx = asx + g_game->gameState->player->posSystem.x / 256 * 2.3;
	float fy = asy + g_game->gameState->player->posSystem.y / 256 * 2.3;
	rect(g_game->GetBackBuffer(), (int)fx-1, (int)fy-1, (int)fx+2, (int)fy+2, BLUE);
}

bool ModuleSolarSystem::LoadStarSystem(int id)
{
	int i;

	debug << "  Loading star system: id = " << id << endl;
	srand(time(NULL));


	//save current star id in global player object
	g_game->gameState->player->currentStar = id;

	//clear the temp array of planets (used to simplify searches)
	for (i=0; i < this->MAX_PLANETS; i++) 
    {
		this->planets[i].tilex = -1;
		this->planets[i].tiley = -1;
		this->planets[i].tilenum = -1;
		this->planets[i].planetid = -1;
        this->planets[i].pbody = NULL;
	}

	//clear the tile map
	this->scroller->ResetTiles();

	//calculate center of tile map
	int systemCenterTileX = this->scroller->GetTilesAcross() / 2;
	int systemCenterTileY = this->scroller->GetTilesDown() / 2;


	//position star tile image at center
	this->scroller->SetTile(systemCenterTileX, systemCenterTileY, 1);

	//read starid passed over by the interstellar module
	this->star = g_game->dataMgr->GetStarByID(id);
	if (!this->star) 
    {
		g_game->message("Interplanetary: Error loading star info");
		return false;
	}

    //are there any planets?
	if (this->star->GetNumPlanets() == 0) return false;

	//seed random number generator with star id #
	srand(id);

	//add planets to the solar system from the planet database
	//calculate position of each planet in orbit around the star
	float radius,angle;
	int rx,ry;
	for (i = 0; i < this->star->GetNumPlanets(); i++) 
    {
		this->planet = this->star->GetPlanet(i);
		if (this->planet) 
        {
			this->planets[i].planetid = this->planet->id;

            //position the planet at a random location in it's circular orbit
			radius = (2 + i) * 4;
			angle = rand() % 360;
			rx = (int)( cos(angle) * radius );
			ry = (int)( sin(angle) * radius );
			this->planets[i].tilex = systemCenterTileX + rx;
			this->planets[i].tiley = systemCenterTileY + ry;


            // ********
            // CHANGE from tiles to rendered planet images
            // ********

			switch(planet->type) 
            {
                case PT_GASGIANT_RED:
                case PT_GASGIANT_BLUE:
                case PT_GASGIANT_GREEN:
                case PT_GASGIANT_PURPLE:    planets[i].tilenum = 2;	break;
			    case PT_MOLTEN:		        planets[i].tilenum = 3;	break;
			    case PT_FROZEN:		        planets[i].tilenum = 4;	break;
			    case PT_OCEANIC:	        planets[i].tilenum = 5;	break;
			    case PT_ROCKY:		        planets[i].tilenum = 6;	break;
			    case PT_ASTEROID:	        planets[i].tilenum = 7;	break;
			    case PT_ACIDIC:		        planets[i].tilenum = 8;	break;
			    default:
				    planets[i].tilenum = 2; 
                    debug << "loadStarSystem: Unknown planet type: " << planet->type << endl;
			}

            //set up the tile data for the planets rendered onto the source image...
			this->scroller->SetTile( this->planets[i].tilex, this->planets[i].tiley, this->planets[i].tilenum );


            //and then render the planets onto that source tile image

            this->planets[i].pbody = new PlanetaryBody();
            PlanetaryBody *pb = this->planets[i].pbody; //just a pointer shortcut
            pb->planetid = this->planets[i].planetid;
            pb->starid = this->star->id;
            pb->planetType = this->planet->type;


            //set the PLANET RADIUS for sphere rendering

			switch(planet->size) {
				case PS_HUGE:
                    pb->planetRadius = 120; //220
                    //pbody->lightmapOffsetX = -250;
                    //pbody->lightmapOffsetY = -250;
                    //pbody->lightmapFilename = "lightmap_planet_500.tga";
					break;
				case PS_LARGE:
					pb->planetRadius = 100; // 172;
                    //pbody->lightmapOffsetX = -200;
                    //pbody->lightmapOffsetY = -200;
                    //pbody->lightmapFilename = "lightmap_planet_400.tga";
					break;
				case PS_MEDIUM:
					pb->planetRadius = 80; // 124;
                    //pbody->lightmapOffsetX = -150;
                    //pbody->lightmapOffsetY = -150;
                    //pbody->lightmapFilename = "lightmap_planet_300.tga";
					break;
				case PS_SMALL:
					pb->planetRadius = 60; // 76;
                    //pbody->lightmapOffsetX = -100;
                    //pbody->lightmapOffsetY = -100;
                    //pbody->lightmapFilename = "lightmap_planet_200.tga";
					break;
				default: //asteroid or error
					pb->planetRadius = 30; // 28;
                    //pbody->lightmapOffsetX = -50;
                    //pbody->lightmapOffsetY = -50;
                    //pbody->lightmapFilename = "lightmap_planet_100.tga";
		    }
                
            //create the planet TEXTURE

            if (!pb->CreatePlanetTextures(pb->starid, pb->planetid))
            {
                g_game->fatalerror("Error creating planet texture: id = " + Util::ToString(this->planet->id));
            }
            else
            {

                //render planet onto scratch image, then copy to the tilescroller source image

                BITMAP *planetImage = NULL;
                planetImage = (BITMAP*)create_bitmap(256,256);
                if (!planetImage) 
                {
                    g_game->fatalerror("LoadStarSystem: error creating planet image");
                }
                clear_to_color(planetImage, makecol(255,0,255));
                

                //rotate planet randomly
                int rotation = Util::Random(0, 255);

                //center the planet on the 256x256 image
                int cx = 128, cy = 128;

                //render the planet using TexturedSphere
                pb->planetRenderObj->Draw( planetImage, 0, 0, rotation, pb->planetRadius, cx, cy );


                BITMAP* scratch = (BITMAP*)create_bitmap(256,256);
                clear_to_color(scratch, makecol(255,0,255));
                masked_blit( planetImage, scratch, 0, 0, 0, 0, 256, 256 );


                //grab the tilescroller source image for modification
                BITMAP* tileImage = this->scroller->GetTileImage();

                int tile = planets[i].tilenum;
		        draw_sprite( tileImage, scratch, 256*tile, 0 );

                destroy_bitmap(scratch);


                //copy the source image back into the tilescroller
                this->scroller->SetTileImage( tileImage );
                
            }
        }
	}

	return true;
}
