/*
	STARFLIGHT - THE LOST COLONY
	ModuleMessageGUI.cpp 
	Author: 
	Date: 
*/

#include "env.h"
#include <allegro.h>
#include "Util.h"
#include "GameState.h"
#include "Game.h"
#include "Events.h"
#include "Script.h"
#include "ModuleMessageGUI.h"
#include "DataMgr.h"
#include "ModeMgr.h"

using namespace std;

int gmx,gmy,gmw,gmh;



ModuleMessageGUI::ModuleMessageGUI() {}
ModuleMessageGUI::~ModuleMessageGUI(){}

bool ModuleMessageGUI::Init()
{
	//load the datafile
	//data = load_datafile("data/messagegui/messagegui.dat");
	//if (!data) {
	//	g_game->message("MessageGUI: Error loading datafile");
	//	return false;
	//}

	//load the gauges gui
	//img_message = (BITMAP*)data[GUI_MESSAGEWINDOW_BMP].dat;
	//img_socket = (BITMAP*)data[GUI_SOCKET_BMP].dat;
    img_message = (BITMAP*)load_bitmap("data/messagegui/gui_messagewindow.bmp",NULL);
    //img_socket = (BITMAP*)load_bitmap("data/messagegui/gui_socket.bmp",NULL);
    if (!img_message)
    {
        debug << "messagegui: error loading bitmaps" << endl;
        return false;
    }


	gmx = (int)g_game->getGlobalNumber("GUI_MESSAGE_POS_X");
	gmy = (int)g_game->getGlobalNumber("GUI_MESSAGE_POS_Y");
	gmw = (int)g_game->getGlobalNumber("GUI_MESSAGE_WIDTH");
	gmh = (int)g_game->getGlobalNumber("GUI_MESSAGE_HEIGHT");
	//gsx = (int)g_game->getGlobalNumber("GUI_SOCKET_POS_X");
	//gsy = (int)g_game->getGlobalNumber("GUI_SOCKET_POS_Y");

	return true;
}

void ModuleMessageGUI::Close()
{
	try 
    {
		//unload_datafile(data);
		//data = NULL;

        if (img_message!=NULL)
        {
            delete img_message;
            img_message=NULL;
        }
        //if (img_socket!=NULL)
        //{
        //    delete img_socket;
        //    img_socket=NULL;
        //}
	}
	catch(std::exception e) {
		debug << e.what() << endl;
	}
	catch(...) {
		debug << "Unhandled exception in ModuleMessageGUI::Close" << endl;
	}
}
	
void ModuleMessageGUI::Update(){}

void ModuleMessageGUI::Draw()
{
	//draw message gui
	masked_blit(img_message, g_game->GetBackBuffer(), 0, 0, gmx, gmy, img_message->w, img_message->h);

	//draw socket gui
	//masked_blit(img_socket, g_game->GetBackBuffer(), 0, 0, gsx, gsy, img_socket->w, img_socket->h);


    // <<<<<<< THIS SHOULD BE MOVED INTO THE AUX DRAW >>>>>>>>>

    //print stardate
	Stardate date = g_game->gameState->stardate;
	int hour = date.GetHour();
	int day = date.GetDay();
	int month = date.GetMonth();
	int year = date.GetYear();
	string datestr = Util::ToString(year) + "-" + Util::ToString(month,2) + "-" + Util::ToString(day,2)
		+ " " + Util::ToString(hour%12);
	if (hour < 12) datestr += " AM"; else datestr += " PM";

    int gax = g_game->getGlobalNumber("GUI_AUX_POS_X");
    int gay = g_game->getGlobalNumber("GUI_AUX_POS_Y");
	
    g_game->Print22(g_game->GetBackBuffer(), gax+10, gay+10, datestr, STEEL);

}

void ModuleMessageGUI::OnKeyPressed(int keyCode){}
void ModuleMessageGUI::OnKeyPress( int keyCode ){}
void ModuleMessageGUI::OnKeyReleased(int keyCode){}
void ModuleMessageGUI::OnMouseMove(int x, int y){}
void ModuleMessageGUI::OnMouseClick(int button, int x, int y){}
void ModuleMessageGUI::OnMousePressed(int button, int x, int y){}
void ModuleMessageGUI::OnMouseReleased(int button, int x, int y){}
void ModuleMessageGUI::OnMouseWheelUp(int x, int y){}
void ModuleMessageGUI::OnMouseWheelDown(int x, int y){}
void ModuleMessageGUI::OnEvent(Event *event){}
