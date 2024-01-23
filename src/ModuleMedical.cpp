/*
	STARFLIGHT - THE LOST COLONY
	ModuleMedical.cpp 
	Author: Keith Patch
	Date: April 2008
*/

#include <sstream>
#include "env.h"
#include <allegro.h>
#include "Util.h"
#include "GameState.h"
#include "Game.h"
#include "Events.h"
#include "DataMgr.h"
#include "Script.h"
#include "ModuleMedical.h"

using namespace std;

int gvl = 0, gvr = 0; //global viewer right and left
const int RIGHT_TARGET_OFFSET	= 675;  //destination position after sliding
const int LEFT_TARGET_OFFSET	= 400;
const int VIEWER_MOVE_RATE		= 30;
#define CATBTN_X 633
#define CATBTN_Y 120
#define CATSPACING 40

int right_offset = 1024;   //starting position x before sliding
int left_offset = -400;
int left_offset2 = -400;


//replaced with new images
//#define GUI_VIEWER_BMP                   3        /* BMP  */
//#define GUI_VIEWER_RIGHT_BMP             4        /* BMP  */
//#define RIGHT_VIEWER_BG_BMP              18       /* BMP  */
//#define MED_WINDOW_DATA_BMP              14       /* BMP  */


#define BTN_DIS_BMP                      0        /* BMP  */
#define BTN_HOV_BMP                      1        /* BMP  */
#define BTN_NORM_BMP                     2        /* BMP  */
#define MED_BAR_COMMUNICATION_BMP        5        /* BMP  */
#define MED_BAR_DURABILITY_BMP           6        /* BMP  */
#define MED_BAR_ENGINEER_BMP             7        /* BMP  */
#define MED_BAR_HEALTH_BMP               8        /* BMP  */
#define MED_BAR_LEARN_BMP                9        /* BMP  */
#define MED_BAR_MEDICAL_BMP              10       /* BMP  */
#define MED_BAR_NAVIGATION_BMP           11       /* BMP  */
#define MED_BAR_SCIENCE_BMP              12       /* BMP  */
#define MED_BAR_TACTICAL_BMP             13       /* BMP  */
#define MEDICAL_CAPTBTN_BMP              15       /* BMP  */
#define MEDICAL_CAPTBTN_DIS_BMP          16       /* BMP  */
#define MEDICAL_CAPTBTN_HOV_BMP          17       /* BMP  */



DATAFILE *meddata;


ModuleMedical::ModuleMedical() {}

ModuleMedical::~ModuleMedical(){}

void ModuleMedical::disable_others(int officer)
{
	if(selected_officer != NULL || officer != -1)
    {
		for (int i=0; i < 7; i++){
			if(i != officer)//this officer is not me
			{
				OfficerBtns[i]->SetEnabled(false);
				HealBtns[i]->SetEnabled(false);
				HealBtns[i]->SetVisible(false);
			}
		}
	}else{
		for (int i=0; i < 7; i++)
		{
			OfficerBtns[i]->SetEnabled(true);
			HealBtns[i]->SetEnabled(true);
			HealBtns[i]->SetVisible(true);
		}
	}
}

void ModuleMedical::cease_healing()
{
	g_game->gameState->officerCap->Recovering(false);
	g_game->gameState->officerSci->Recovering(false);
	g_game->gameState->officerEng->Recovering(false);
	g_game->gameState->officerNav->Recovering(false);
	g_game->gameState->officerDoc->Recovering(false);
	g_game->gameState->officerTac->Recovering(false);
	g_game->gameState->officerCom->Recovering(false);
}

void ModuleMedical::OnEvent(Event *event)
{
	Officer *currentDoc = g_game->gameState->getCurrentDoc();
	string med = currentDoc->getLastName() + "-> ";
	string other = "";
	
	switch(event->getEventType())
	{
		/**
			CREW SELECTED EVENTS
		**/
		case -100: //captain selected
			if(selected_officer != g_game->gameState->officerCap){
				selected_officer = g_game->gameState->officerCap;
				disable_others(0);
			}else{
				selected_officer = NULL;
				disable_others(-1);
			}
			break;
		case -101: //science selected
			if(selected_officer != g_game->gameState->officerSci){
				selected_officer = g_game->gameState->officerSci;
				disable_others(1);
			}else{
				selected_officer = NULL;
				disable_others(-1);
			}
			break;
		case -102: //navigation selected
			if(selected_officer != g_game->gameState->officerNav){
				selected_officer = g_game->gameState->officerNav;
				disable_others(2);
			}else{
				selected_officer = NULL;
				disable_others(-1);
			}
			break;
		case -103: //tactical selected
			if(selected_officer != g_game->gameState->officerTac){
				selected_officer = g_game->gameState->officerTac;
				disable_others(3);
			}else{
				selected_officer = NULL;
				disable_others(-1);
			}
			break;
		case -104: //engineer selected
			if(selected_officer != g_game->gameState->officerEng){
				selected_officer = g_game->gameState->officerEng;
				disable_others(4);
			}else{
				selected_officer = NULL;
				disable_others(-1);
			}
			break;
		case -105: //communications selected
			if(selected_officer != g_game->gameState->officerCom){
				selected_officer = g_game->gameState->officerCom;
				disable_others(5);
			}else{
				selected_officer = NULL;
				disable_others(-1);
			}
			break;
		case -106: //medical selected
			if(selected_officer != g_game->gameState->officerDoc){
				selected_officer = g_game->gameState->officerDoc;
				disable_others(6);
			}else{
				selected_officer = NULL;
				disable_others(-1);
			}
			break;

		/**
			CREW TREATMENT EVENTS
		**/
		case -200: //captain treat
			cease_healing();
			if(g_game->gameState->officerCap->isBeingHealed() == false 
			&& g_game->gameState->officerCap->attributes.getVitality() < 100
			&& g_game->gameState->officerCap->attributes.getVitality() >   0)
            {
				g_game->gameState->officerCap->Recovering(true);
				other = "Captain " + g_game->gameState->officerCap->getLastName();
				(currentDoc == g_game->gameState->officerCap)?
					g_game->printout(g_game->g_scrollbox, med + "Okay, I'll patch myself up", GREEN, 1000):
					g_game->printout(g_game->g_scrollbox, med + "Okay, I'm treating " + other, GREEN, 1000);
			} 
			break;
		case -201: //science treat
			cease_healing();
			if(g_game->gameState->officerSci->isBeingHealed() == false 
			&& g_game->gameState->officerSci->attributes.getVitality() < 100
			&& g_game->gameState->officerSci->attributes.getVitality() >   0)
			{
				g_game->gameState->officerSci->Recovering(true);
				other = "Science Officer " + g_game->gameState->officerSci->getLastName();
				g_game->printout(g_game->g_scrollbox, med + "Okay, I'm treating " + other, GREEN, 1000);
			}
			break;
		case -202: //navigation treat
			cease_healing();
			if(g_game->gameState->officerNav->isBeingHealed() == false 
			&& g_game->gameState->officerNav->attributes.getVitality() < 100
			&& g_game->gameState->officerNav->attributes.getVitality() >   0)
			{
				g_game->gameState->officerNav->Recovering(true);
				other = "Navigator " + g_game->gameState->officerNav->getLastName();
				g_game->printout(g_game->g_scrollbox, med + "Okay, I'm treating " + other, GREEN, 1000);
			}
			break;
		case -203: //tactical treat
			cease_healing();
			if(g_game->gameState->officerTac->isBeingHealed() == false 
			&& g_game->gameState->officerTac->attributes.getVitality() < 100
			&& g_game->gameState->officerTac->attributes.getVitality() >   0)
			{
				g_game->gameState->officerTac->Recovering(true);
				other = "Tactical Officer " + g_game->gameState->officerTac->getLastName();
				g_game->printout(g_game->g_scrollbox, med + "Okay, I'm treating " + other, GREEN, 1000);
			}
			break;
		case -204: //engineer treat
			cease_healing();
			if(g_game->gameState->officerEng->isBeingHealed() == false 
			&& g_game->gameState->officerEng->attributes.getVitality() < 100
			&& g_game->gameState->officerEng->attributes.getVitality() >   0)
			{
				g_game->gameState->officerEng->Recovering(true);
				other = "Engineer " + g_game->gameState->officerEng->getLastName();
				g_game->printout(g_game->g_scrollbox, med + "Okay, I'm treating " + other, GREEN, 1000);
			}
			break;
		case -205: //communications treat
			cease_healing();
			if(g_game->gameState->officerCom->isBeingHealed() == false 
			&& g_game->gameState->officerCom->attributes.getVitality() < 100
			&& g_game->gameState->officerCom->attributes.getVitality() >   0)
			{
				g_game->gameState->officerCom->Recovering(true);
				other = "Comm Officer " + g_game->gameState->officerCom->getLastName();
				g_game->printout(g_game->g_scrollbox, med + "Okay, I'm treating " + other, GREEN, 1000);
			}
			break;
		case -206: //medical treat
			cease_healing();
			if(g_game->gameState->officerDoc->isBeingHealed() == false 
			&& g_game->gameState->officerDoc->attributes.getVitality() < 100
			&& g_game->gameState->officerDoc->attributes.getVitality() >   0)
			{
				g_game->gameState->officerDoc->Recovering(true);
				other = "Doctor " + g_game->gameState->officerDoc->getLastName();
				(currentDoc == g_game->gameState->officerDoc)?
					g_game->printout(g_game->g_scrollbox, med + "Okay, I'll patch myself up", GREEN, 1000) :
					g_game->printout(g_game->g_scrollbox, med + "Okay, I'm treating " + other, GREEN, 1000);

			}
			break;

		case 7001: //EVENT_DOCTOR_TREAT
			if(b_examine == false && viewer_active == false)
			{
				viewer_active = true;
				//b_examine = false;
			} 
			else if(viewer_active == true && b_examine == true)
			{
				b_examine = false;
			}
            else {
				viewer_active = false;
				b_examine = true;
			}
			break;

		case 7000: //EVENT_DOCTOR_EXAMINE
			if(b_examine == true){
				viewer_active = false;
			}else{
				viewer_active = true;
				b_examine = true;
			}
			break;
		case 0: default:
			break;
	}
}


bool ModuleMedical::Init()
{
	debug << "  ModuleMedical::Init" << endl;

	b_examine = false;
	selected_officer = NULL;

	//disable all healing
	//cease_healing();


	//sound effects
	g_game->audioSystem->Load("data/medical/buttonclick.ogg", "click");

	//load data file
	meddata = load_datafile("data/medical/medical.dat");
	if (!meddata) {
		g_game->message("Medical: Error loading data file");
		return false;
	}


    //CREW LIST VIEWER (right side)
	img_crewlist_viewer = load_bitmap("data/medical/gui_viewer_right.bmp",NULL);
	if (!img_crewlist_viewer) {
		debug << "Medical: Error loading gui_viewer_right image" << endl;
		return false;
	}



    //CREW HEALTH VIEWER (left side)
	img_crewhealth_viewer = load_bitmap("data/medical/gui_viewer_left.bmp",NULL);
	if (!img_crewhealth_viewer) 
    {
		g_game->message("Medical: Error loading gui_viewer_left image");
		return false;
	}



    //CREW SKILLS VIEWER (left side)
	img_crewskills_viewer = load_bitmap("data/medical/gui_viewer.bmp",NULL);
	if (!img_crewskills_viewer) 
    {
		g_game->message("Medical: Error loading gui_viewer image");
		return false;
	}



	//img_health_bar = load_bitmap("data/medical/med_bar_health.bmp", NULL);
	img_health_bar = (BITMAP*)meddata[MED_BAR_HEALTH_BMP].dat;
	if (!img_health_bar) {
		g_game->message("Medical: Error loading med_bar_health image");
		return false;
	}
	
	//img_science_bar = load_bitmap("data/medical/med_bar_science.bmp", NULL);
	img_science_bar = (BITMAP*)meddata[MED_BAR_SCIENCE_BMP].dat;
	if (!img_science_bar) {
		g_game->message("Medical: Error loading med_bar_science image");
		return false;
	}
	
	//img_nav_bar = load_bitmap("data/medical/med_bar_navigation.bmp", NULL);
	img_nav_bar = (BITMAP*)meddata[MED_BAR_NAVIGATION_BMP].dat;
	if (!img_nav_bar) {
		g_game->message("Medical: Error loading med_bar_navigation image");
		return false;
	}
	
	//img_medical_bar = load_bitmap("data/medical/med_bar_medical.bmp", NULL); 
	img_medical_bar = (BITMAP*)meddata[MED_BAR_MEDICAL_BMP].dat;
	if (!img_medical_bar) {
		g_game->message("Medical: Error loading med_bar_medical image");
		return false;
	}
	
	//img_engineer_bar = load_bitmap("data/medical/med_bar_engineer.bmp", NULL); 
	img_engineer_bar = (BITMAP*)meddata[MED_BAR_ENGINEER_BMP].dat;
	if (!img_engineer_bar) {
		g_game->message("Medical: Error loading med_bar_engineer image");
		return false;
	}
	
	//img_dur_bar = load_bitmap("data/medical/med_bar_durability.bmp", NULL);
	img_dur_bar = (BITMAP*)meddata[MED_BAR_DURABILITY_BMP].dat;
	if (!img_dur_bar) {
		g_game->message("Medical: Error loading med_bar_durability image");
		return false;
	}
	
	//img_learn_bar = load_bitmap("data/medical/med_bar_learn.bmp", NULL);
	img_learn_bar = (BITMAP*)meddata[MED_BAR_LEARN_BMP].dat;
	if (!img_learn_bar) {
		g_game->message("Medical: Error loading med_bar_learn image");
		return false;
	}
	
	//img_comm_bar = load_bitmap("data/medical/med_bar_communication.bmp", NULL);
	img_comm_bar = (BITMAP*)meddata[MED_BAR_COMMUNICATION_BMP].dat;
	if (!img_comm_bar) {
		g_game->message("Medical: Error loading med_bar_communication image");
		return false;
	}
	
	//img_tac_bar = load_bitmap("data/medical/med_bar_tactical.bmp", NULL);
	img_tac_bar = (BITMAP*)meddata[MED_BAR_TACTICAL_BMP].dat;
	if (!img_tac_bar) {
		g_game->message("Medical: Error loading med_bar_tactical image");
		return false;
	}

	//load crew button images
	img_button_crew = (BITMAP*)meddata[MEDICAL_CAPTBTN_BMP].dat;
	if (!img_button_crew) {
		g_game->message("Medical: Error loading crew button image");
		return false;
	}

	img_button_crew_hov = (BITMAP*)meddata[MEDICAL_CAPTBTN_HOV_BMP].dat;
	if (!img_button_crew_hov) {
		g_game->message("Medical: Error loading crew button hover image");
		return false;
	}

	img_button_crew_dis = (BITMAP*)meddata[MEDICAL_CAPTBTN_DIS_BMP].dat;
	if (!img_button_crew_dis) {
		g_game->message("Medical: Error loading crew button disabled image");
		return false;
	}

	for (int i=0; i < 7; i++)
	{
		//Create and initialize the crew buttons
		OfficerBtns[i] = new Button(img_button_crew, img_button_crew_hov, img_button_crew_dis,
			CATBTN_X, CATBTN_Y + (i * CATSPACING), 0, -100 - i, g_game->font22,"", makecol(255,255,255), "click");

		if (OfficerBtns[i] == NULL){return false;}
		if (!OfficerBtns[i]->IsInitialized()){return false;}
	}

	//set labels for each crew member
	OfficerBtns[0]->SetButtonText("CAP. " + g_game->gameState->officerCap->name);
	OfficerBtns[1]->SetButtonText("SCI. " + g_game->gameState->officerSci->name);
	OfficerBtns[2]->SetButtonText("NAV. " + g_game->gameState->officerNav->name);
	OfficerBtns[3]->SetButtonText("TAC. " + g_game->gameState->officerTac->name);
	OfficerBtns[4]->SetButtonText("ENG. " + g_game->gameState->officerEng->name);
	OfficerBtns[5]->SetButtonText("COM. " + g_game->gameState->officerCom->name);
	OfficerBtns[6]->SetButtonText("DOC. " + g_game->gameState->officerDoc->name);

	//load plus button images
	img_treat = (BITMAP*)meddata[BTN_NORM_BMP].dat;
	if (!img_treat) {
		g_game->message("Medical: Error loading plus button image");
		return false;
	}
	img_treat_hov = (BITMAP*)meddata[BTN_HOV_BMP].dat;
	if (!img_treat_hov) {
		g_game->message("Medical: Error loading hov button image");
		return false;
	}
	img_treat_dis = (BITMAP*)meddata[BTN_DIS_BMP].dat;
	if (!img_treat_dis) {
		g_game->message("Medical: Error loading plus button disabled image");
		return false;
	}

	//Create and initialize the heal buttons
	//they share the same location but are unique for each crew to make events simpler
	for (int i=0; i < 7; i++)
	{
		HealBtns[i] = new Button( img_treat, img_treat_hov, img_treat_dis,
			CATBTN_X + 142, 2 + CATBTN_Y + (i * CATSPACING), 0, -200 - i, g_game->font24, "TREAT", BLACK, "click");
		if (HealBtns[i] == NULL){return false;}
		if (!HealBtns[i]->IsInitialized()){return false;}
	}


	//DEBUG CODE
	//g_game->gameState->officerCap->attributes.setVitality(90);
	//g_game->gameState->officerSci->attributes.setVitality(70);
	//g_game->gameState->officerNav->attributes.setVitality(55);
	//g_game->gameState->officerTac->attributes.setVitality(40);
	//g_game->gameState->officerEng->attributes.setVitality(25);
	//g_game->gameState->officerCom->attributes.setVitality(10);
	//g_game->gameState->officerDoc->attributes.setVitality(0);  //kill the doctor
	//g_game->gameState->officerDoc->attributes.setMedical(50);
	//g_game->gameState->officerCap->attributes.setMedical(20);

	return true;
}

void ModuleMedical::Close()
{
	//disable all healing
	//cease_healing();

	try {
        if (img_crewlist_viewer!=NULL)
        {
            delete img_crewlist_viewer;
            img_crewlist_viewer=NULL;
        }
        if (img_crewhealth_viewer!=NULL)
        {
            delete img_crewhealth_viewer;
            img_crewhealth_viewer=NULL;
        }
        if (img_crewskills_viewer!=NULL)
        {
            delete img_crewskills_viewer;
            img_crewskills_viewer=NULL;
        }

		unload_datafile(meddata);
		meddata = NULL;

		for (int i=0; i < 7; i++){
			delete OfficerBtns[i];
			OfficerBtns[i] = NULL;
		}

		for (int i=0; i < 7; i++)
        {
			delete HealBtns[i];
			HealBtns[i] = NULL;
		}
		selected_officer = NULL;

	}
	catch (std::exception e) {
		debug << e.what() << endl;
	}
	catch(...) {
		debug << "Unhandled exception in ModuleMedical::Close" << endl;
	}
}

void ModuleMedical::Update(){}

void ModuleMedical::MedicalUpdate()
{
	Officer* currentDoc = g_game->gameState->getCurrentDoc();
	string med = currentDoc->getLastName() + "-> ";
	string other = "";

	//heal medical_skill/50 vitality point every iteration
	float heal_rate = g_game->gameState->CalcEffectiveSkill(SKILL_MEDICAL) / 50.0;

	//award one skill point every 12 iterations
	if(currentDoc->attributes.extra_variable >= 12){
		currentDoc->attributes.extra_variable = 0;

		if(currentDoc->SkillUp(SKILL_MEDICAL))
			g_game->printout(g_game->g_scrollbox, med + "I think I'm getting better at this.", PURPLE, 5000);
	}

	if(g_game->gameState->getCurrentSelectedOfficer() != OFFICER_MEDICAL)
		viewer_active = false;

    //CAPTAIN TREAT
	if(g_game->gameState->officerCap->attributes.getVitality() <= 0)
	{
		OfficerBtns[0]->SetTextColor(BLACK);
	}
	else if(g_game->gameState->officerCap->isBeingHealed() == true)
	{
		OfficerBtns[0]->SetTextColor(LTGREEN);
		if(currentDoc->CanSkillCheck()){
			currentDoc->FakeSkillCheck();
			currentDoc->attributes.extra_variable++;
			g_game->gameState->officerCap->attributes.augVitality(heal_rate);
		}
		if(g_game->gameState->officerCap->attributes.getVitality() == 100)
		{
			g_game->gameState->officerCap->Recovering(false);
			other = "Captain " + g_game->gameState->officerCap->getLastName();
			(currentDoc == g_game->gameState->officerCap)?
				g_game->printout(g_game->g_scrollbox, med + "I'm feeling much better now.", BLUE, 1000) :
				g_game->printout(g_game->g_scrollbox, other + " has fully recovered.", BLUE, 1000);
		}
	} else if(g_game->gameState->officerCap->attributes.getVitality() < 30)
	{
		OfficerBtns[0]->SetTextColor(RED2);
	}
	else if(g_game->gameState->officerCap->attributes.getVitality() < 70)
	{
		OfficerBtns[0]->SetTextColor(YELLOW2);
	}
	else {
		OfficerBtns[0]->SetTextColor(WHITE);
	}

    
    //SCIENCE TREAT
    if(g_game->gameState->officerSci->attributes.getVitality() <= 0)
	{
		OfficerBtns[1]->SetTextColor(BLACK);
	}
	else if( g_game->gameState->officerSci->isBeingHealed() == true)
	{
		OfficerBtns[1]->SetTextColor(LTGREEN);
		if(currentDoc->CanSkillCheck() == true){
			currentDoc->FakeSkillCheck();
			currentDoc->attributes.extra_variable++;
			g_game->gameState->officerSci->attributes.augVitality(heal_rate);
		}
		if(g_game->gameState->officerSci->attributes.getVitality() == 100)
		{
			g_game->gameState->officerSci->Recovering(false);
			other = "Science Officer " + g_game->gameState->officerSci->getLastName();
			g_game->printout(g_game->g_scrollbox, other + " has fully recovered.", BLUE, 1000);
		}
	}else if(g_game->gameState->officerSci->attributes.getVitality() < 30){
		OfficerBtns[1]->SetTextColor(RED2);
	}else if(g_game->gameState->officerSci->attributes.getVitality() < 70){
		OfficerBtns[1]->SetTextColor(YELLOW2);
	}else{
		OfficerBtns[1]->SetTextColor(WHITE);
	}


    //NAVIGATION TREAT
	if(g_game->gameState->officerNav->attributes.getVitality() <= 0){
		OfficerBtns[2]->SetTextColor(BLACK);
	}else if(g_game->gameState->officerNav->isBeingHealed() == true){
		OfficerBtns[2]->SetTextColor(LTGREEN);
		if(currentDoc->CanSkillCheck() == true){
			currentDoc->FakeSkillCheck();
			currentDoc->attributes.extra_variable++;
			g_game->gameState->officerNav->attributes.augVitality(heal_rate);
		}
		if(g_game->gameState->officerNav->attributes.getVitality() == 100)
		{
			g_game->gameState->officerNav->Recovering(false);
			other = "Navigator " + g_game->gameState->officerNav->getLastName();
			g_game->printout(g_game->g_scrollbox, other + " has fully recovered.", BLUE, 1000);
		}
	}else if(g_game->gameState->officerNav->attributes.getVitality() < 30){
		OfficerBtns[2]->SetTextColor(RED2);
	}else if(g_game->gameState->officerNav->attributes.getVitality() < 70){
		OfficerBtns[2]->SetTextColor(YELLOW2);
	}else{
		OfficerBtns[2]->SetTextColor(WHITE);
	}


    //TACTICAL TREAT
	if(g_game->gameState->officerTac->attributes.getVitality() <= 0)
	{
		OfficerBtns[3]->SetTextColor(BLACK);
	}else if(g_game->gameState->officerTac->isBeingHealed() == true)
	{
		OfficerBtns[3]->SetTextColor(LTGREEN);
		if(currentDoc->CanSkillCheck() == true)
		{
			currentDoc->FakeSkillCheck();
			currentDoc->attributes.extra_variable++;
			g_game->gameState->officerTac->attributes.augVitality(heal_rate);
		}
		if(g_game->gameState->officerTac->attributes.getVitality() == 100)
		{
			g_game->gameState->officerTac->Recovering(false);
			other = "Tactical Officer " + g_game->gameState->officerTac->getLastName();
			g_game->printout(g_game->g_scrollbox, other + " has fully recovered.", BLUE, 1000);
		}
	}else if(g_game->gameState->officerTac->attributes.getVitality() < 30)
	{
		OfficerBtns[3]->SetTextColor(RED2);
	}else if(g_game->gameState->officerTac->attributes.getVitality() < 70)
	{
		OfficerBtns[3]->SetTextColor(YELLOW2);
	}else{
		OfficerBtns[3]->SetTextColor(WHITE);
	}
    

    //ENGINEER TREAT
	if(g_game->gameState->officerEng->attributes.getVitality() <= 0)
	{
		OfficerBtns[4]->SetTextColor(BLACK);
	}
	else if(g_game->gameState->officerEng->isBeingHealed() == true)
	{

		OfficerBtns[4]->SetTextColor(LTGREEN);
		if(currentDoc->CanSkillCheck() == true){
			currentDoc->FakeSkillCheck();
			currentDoc->attributes.extra_variable++;
			g_game->gameState->officerEng->attributes.augVitality(heal_rate);
		}
		if(g_game->gameState->officerEng->attributes.getVitality() == 100)
		{
			g_game->gameState->officerEng->Recovering(false);
			other = "Engineer " + g_game->gameState->officerEng->getLastName();
			g_game->printout(g_game->g_scrollbox, other + " has fully recovered.", BLUE, 1000);
		}
	}else if(g_game->gameState->officerEng->attributes.getVitality() < 30){
		OfficerBtns[4]->SetTextColor(RED2);
	}else if(g_game->gameState->officerEng->attributes.getVitality() < 70){
		OfficerBtns[4]->SetTextColor(YELLOW2);
	}else{
		OfficerBtns[4]->SetTextColor(WHITE);
	}


    //COMMUNICATIONS TREAT
	if(g_game->gameState->officerCom->attributes.getVitality() <= 0){
		OfficerBtns[5]->SetTextColor(BLACK);
	}else if(g_game->gameState->officerCom->isBeingHealed() == true){
		OfficerBtns[5]->SetTextColor(LTGREEN);
		if(currentDoc->CanSkillCheck() == true){
			currentDoc->FakeSkillCheck();
			currentDoc->attributes.extra_variable++;
			g_game->gameState->officerCom->attributes.augVitality(heal_rate);
		}
		if(g_game->gameState->officerCom->attributes.getVitality() == 100)
		{
			g_game->gameState->officerCom->Recovering(false);
			other = "Comm Officer " + g_game->gameState->officerCom->getLastName();
			g_game->printout(g_game->g_scrollbox, other + " has fully recovered.", BLUE, 1000);
		}
	}else if(g_game->gameState->officerCom->attributes.getVitality() < 30){
		OfficerBtns[5]->SetTextColor(RED2);
	}else if(g_game->gameState->officerCom->attributes.getVitality() < 70){
		OfficerBtns[5]->SetTextColor(YELLOW2);
	}else{
		OfficerBtns[5]->SetTextColor(WHITE);
	}


    //DOCTOR TREAT
	if(g_game->gameState->officerDoc->attributes.getVitality() <= 0)
    {
		OfficerBtns[6]->SetTextColor(BLACK);
	}
    else if(g_game->gameState->officerDoc->isBeingHealed() )
    {
		OfficerBtns[6]->SetTextColor(LTGREEN);
		if(currentDoc->CanSkillCheck() == true)
        {
			currentDoc->FakeSkillCheck();
			currentDoc->attributes.extra_variable++;
			g_game->gameState->officerDoc->attributes.augVitality(heal_rate);
		}
		if(g_game->gameState->officerDoc->attributes.getVitality() == 100)
        {
			g_game->gameState->officerDoc->Recovering(false);
			other = "Doctor " + g_game->gameState->officerDoc->getLastName();
			(currentDoc == g_game->gameState->officerDoc)?
				g_game->printout(g_game->g_scrollbox, med + "I'm feeling much better now.", BLUE, 1000) :
				g_game->printout(g_game->g_scrollbox, other + " has fully recovered.", BLUE, 1000);
		}
	}else if(g_game->gameState->officerDoc->attributes.getVitality() < 30){
		OfficerBtns[6]->SetTextColor(RED2);
	}else if(g_game->gameState->officerDoc->attributes.getVitality() < 70){
		OfficerBtns[6]->SetTextColor(YELLOW2);
	}else{
		OfficerBtns[6]->SetTextColor(WHITE);
	}
}

void ModuleMedical::Draw()
{
    static const string skillnames[] = {
        "science",
        "navigation",
        "engineering",
        "communication",
        "medical",
        "tactical",
        "learning",
        "durability" };

	float percentile = 0.00f;
//	char t_buffer[20];

    //update medical status
	MedicalUpdate();


    //RIGHT WINDOW (crew list)

    int RIGHT_X = right_offset;
    int RIGHT_Y = 85;

	if(right_offset < SCREEN_W)
	{
		//draw crew list viewer
		masked_blit(img_crewlist_viewer, g_game->GetBackBuffer(), 0, 0, RIGHT_X, RIGHT_Y, img_crewlist_viewer->w, img_crewlist_viewer->h);

		//draw crew buttons
		for(int i=0; i < 7; i++)
        {
			OfficerBtns[i]->SetX( RIGHT_X + 34 );
			OfficerBtns[i]->Run(g_game->GetBackBuffer());
		}
	}


    //CREW HEALTH VIEWER

    int LEFT_X = 0;
    int LEFT_Y = 85;
    int inner_window_x = 25;
    int inner_window_y = 34;
    int x1 = LEFT_X + inner_window_x + 10;
    int y1 = LEFT_Y + inner_window_y + 10;

	if(left_offset2 > -LEFT_TARGET_OFFSET)
	{
		masked_blit(img_crewhealth_viewer, g_game->GetBackBuffer(), 0, 0, LEFT_X, LEFT_Y, img_crewhealth_viewer->w, img_crewhealth_viewer->h);

		if(selected_officer != NULL)
		{

			//print crew person's title and name
            string title = selected_officer->GetTitle() + ":";
			g_game->Print22(g_game->GetBackBuffer(), x1, y1, title, WHITE); 

            y1+=20;
            string name = selected_officer->name;
			g_game->Print22(g_game->GetBackBuffer(), x1, y1, name, GOLD); 


			//show health status
			int health_color = 0;
			std::string status = "";
			if(selected_officer->attributes.getVitality() <= 0)
            {
				health_color = BLACK;
				status = "DEAD";
			}
            else if(selected_officer->attributes.getVitality() < 30)
            {
				health_color = RED2;
				status = "CRITICAL";
			}
            else if(selected_officer->attributes.getVitality() < 70)
            {
				health_color = YELLOW2;
				status = "INJURED";
			}
            else
            {
				health_color = GREEN2;
				status = "HEALTHY";
			}
            y1+=40;
			g_game->Print20(g_game->GetBackBuffer(), x1, y1, "STATUS: ", WHITE); 
			g_game->Print20(g_game->GetBackBuffer(), x1 + 80, y1, status, health_color); 


            //draw the health bar
            y1+=40;
			percentile = selected_officer->attributes.getVitality();
			rectfill(g_game->GetBackBuffer(), x1, y1, x1 + (int)(275.0f * percentile / 100.0f), y1 + 50, health_color );
			
			//print health percentage
            string percent = Util::ToString(percentile, 1, 1);
			g_game->Print22(g_game->GetBackBuffer(), x1 + 115, y1 + 20, percent, WHITE);


			//enable treat button based on crew health
			int officer = selected_officer->GetOfficerType() - 1;
			if (selected_officer->attributes.getVitality() < 100.0f)
				HealBtns[officer]->SetEnabled(true); 
			else
				HealBtns[officer]->SetEnabled(false); 


			//display treat button
            y1+=90;
			int buttonx = x1 + 80;
			int buttony = y1;
			for(int i=0; i<7; i++)
			{
				HealBtns[i]->SetX(buttonx);
				HealBtns[i]->SetY(buttony);
				HealBtns[i]->Run(g_game->GetBackBuffer());
			}
		}
    }


    //CREW SKILLS VIEWER

    LEFT_X = 0;
    LEFT_Y = 60;
    inner_window_x = 25;
    inner_window_y = 32;
    x1 = LEFT_X + inner_window_x + 45;
    y1 = LEFT_Y + inner_window_y + 3;

	if(left_offset > -LEFT_TARGET_OFFSET)
    {
        //left sliding viewer showing crew skills
		masked_blit(img_crewskills_viewer, g_game->GetBackBuffer(), 0, 0, LEFT_X, LEFT_Y, img_crewskills_viewer->w, img_crewskills_viewer->h);

        //display officer stat bars
		if(selected_officer != NULL)
		{
            int stepy = 37;
            int skilltextcolor = WHITE;
            string s;

            //vitality bar
			percentile = selected_officer->attributes.getVitality(); 
            //percentile /= 100;
			masked_blit(img_medical_bar, g_game->GetBackBuffer(), 0, 0, x1, y1, img_health_bar->w * percentile/100, img_health_bar->h);
            s = "HEALTH: " + Util::ToString(percentile,1,0) + string(" %");
            g_game->Print22(g_game->GetBackBuffer(), x1+10, y1+5, s, skilltextcolor);

            //science bar
			percentile = selected_officer->attributes.getScience();	
            //percentile /= 250;
            y1+=stepy;
			masked_blit(img_medical_bar, g_game->GetBackBuffer(), 0, 0, x1, y1, img_science_bar->w * percentile/250, img_science_bar->h);
            s = "SCI SKILL: " + Util::ToString(percentile,1,0);
            g_game->Print22(g_game->GetBackBuffer(), x1+10, y1+5, s, skilltextcolor);

            //navigation bar
			percentile = selected_officer->attributes.getNavigation(); 
            //percentile /= 250;
            y1 += stepy;
			masked_blit(img_medical_bar, g_game->GetBackBuffer(), 0, 0, x1, y1, img_nav_bar->w * percentile/250, img_nav_bar->h);
            s = "NAV SKILL: " + Util::ToString(percentile,1,0);
            g_game->Print22(g_game->GetBackBuffer(), x1+10, y1+5, s, skilltextcolor);

            //engineering bar
			percentile = selected_officer->attributes.getEngineering(); 
            //percentile /= 250;
            y1 += stepy;
			masked_blit(img_medical_bar, g_game->GetBackBuffer(), 0, 0, x1, y1, img_engineer_bar->w * percentile/250, img_engineer_bar->h);
            s = "ENG SKILL: " + Util::ToString(percentile,1,0);
            g_game->Print22(g_game->GetBackBuffer(), x1+10, y1+5, s, skilltextcolor);

            //communications bar
			percentile = selected_officer->attributes.getCommunication(); 
            //percentile /= 250;
            y1 += stepy;
			masked_blit(img_medical_bar, g_game->GetBackBuffer(), 0, 0, x1, y1, img_comm_bar->w * percentile/250, img_comm_bar->h);
            s = "COM SKILL: " + Util::ToString(percentile,1,0);
            g_game->Print22(g_game->GetBackBuffer(), x1+10, y1+5, s, skilltextcolor);

            //medical bar
			percentile = selected_officer->attributes.getMedical(); 
            //percentile /= 250;
            y1 += stepy;
			masked_blit(img_medical_bar, g_game->GetBackBuffer(), 0, 0, x1, y1, img_medical_bar->w * percentile/250, img_medical_bar->h);
            s = "MED SKILL: " + Util::ToString(percentile,1,0);
            g_game->Print22(g_game->GetBackBuffer(), x1+10, y1+5, s, skilltextcolor);

            //tactical bar
			percentile = selected_officer->attributes.getTactics(); 
            //percentile /= 250;
            y1 += stepy;
			masked_blit(img_medical_bar, g_game->GetBackBuffer(), 0, 0, x1, y1, img_tac_bar->w * percentile/250, img_tac_bar->h);
            s = "TAC SKILL: " + Util::ToString(percentile,1,0);
            g_game->Print22(g_game->GetBackBuffer(), x1+10, y1+5, s, skilltextcolor);

            //learning rate bar
			percentile = selected_officer->attributes.getLearnRate(); 
            //percentile /= 65;
            y1 += stepy;
			masked_blit(img_medical_bar, g_game->GetBackBuffer(), 0, 0, x1, y1, img_learn_bar->w * percentile/65, img_learn_bar->h);
            s = "LEARNING: " + Util::ToString(percentile,1,0);
            g_game->Print22(g_game->GetBackBuffer(), x1+10, y1+5, s, skilltextcolor);

            //durability
			percentile = selected_officer->attributes.getDurability(); 
            //percentile /= 65;
            y1 += stepy;
			masked_blit(img_medical_bar, g_game->GetBackBuffer(), 0, 0, x1, y1, img_dur_bar->w * percentile/65, img_dur_bar->h);
            s = "DURABILITY: " + Util::ToString(percentile,1,0);
            g_game->Print22(g_game->GetBackBuffer(), x1+10, y1+5, s, skilltextcolor);
		}
	}


    //MOVE THE SLIDING PANELS
    if(viewer_active)
    {
		if(right_offset > RIGHT_TARGET_OFFSET)
        {
			right_offset -= VIEWER_MOVE_RATE;
		}
		if(b_examine == true)
        {
            //view crew stats
			if(left_offset < 0)
            {
				left_offset += VIEWER_MOVE_RATE;
			}
			if(left_offset2 > -LEFT_TARGET_OFFSET)
            {
				left_offset2 -= VIEWER_MOVE_RATE;
			}
		}else{
            //treat crew injuries
			if(left_offset2 < 0)
            {
				left_offset2 += VIEWER_MOVE_RATE;
			}
			if(left_offset > -LEFT_TARGET_OFFSET)
            {
				left_offset -= VIEWER_MOVE_RATE;
			}
		}
	}else{
		if(right_offset < SCREEN_W)
        {
			right_offset += VIEWER_MOVE_RATE;
		}
		if(left_offset > -LEFT_TARGET_OFFSET)
        {
			left_offset -= VIEWER_MOVE_RATE;
		}
		if(left_offset2 > -LEFT_TARGET_OFFSET)
        {
			left_offset2 -= VIEWER_MOVE_RATE;
		}
		disable_others(-1);
		selected_officer = NULL;
		b_examine = false;
	}

}

void ModuleMedical::OnKeyPressed(int keyCode){}
void ModuleMedical::OnKeyPress( int keyCode ){}
void ModuleMedical::OnKeyReleased(int keyCode){}

void ModuleMedical::OnMouseMove(int x, int y)
{
	if (g_game->gameState->getCurrentSelectedOfficer() != OFFICER_MEDICAL)
		return;
	
	for (int i=0; i < 7; i++){
		OfficerBtns[i]->OnMouseMove(x,y);
		HealBtns[i]->OnMouseMove(x,y);
	}
}

void ModuleMedical::OnMouseClick(int button, int x, int y){}
void ModuleMedical::OnMousePressed(int button, int x, int y){}
void ModuleMedical::OnMouseReleased(int button, int x, int y)
{
	if (g_game->gameState->getCurrentSelectedOfficer() != OFFICER_MEDICAL)
		return;

	for (int i=0; i < 7; i++){
		OfficerBtns[i]->OnMouseReleased(button, x,y);
		HealBtns[i]->OnMouseReleased(button, x,y);
	}
}

void ModuleMedical::OnMouseWheelUp(int x, int y){}
void ModuleMedical::OnMouseWheelDown(int x, int y){}
