/*
	STARFLIGHT - THE LOST COLONY (REMASTERED)
	PlanetaryBody.cpp - Encapsulates planet textures
	Author: J.Harbour
	Date: Feb, 2024
*/



#include <exception>
#include "env.h"
#include "PerlinTL.h"
#include "Game.h"
#include "Util.h"
#include "PlanetaryBody.h"
using namespace std;


PlanetaryBody::PlanetaryBody(void)
{
    planetTexture=NULL;
    starid = planetid = planetRadius = 0;
    lightmapOffsetX = lightmapOffsetY = 0;
    lightmapFilename="";
    planetType=PlanetType::PT_INVALID;
    texSphere=NULL;
    planetTexture=NULL;
    planetTopography=NULL;
    planetScannerMap=NULL;
    randomness=0;
}

PlanetaryBody::~PlanetaryBody(void)
{
    if (this->lightmapOverlay) 
    {
        destroy_bitmap(this->lightmapOverlay);
        this->lightmapOverlay=NULL;
    }
	if (this->planetTopography) 
    {
        destroy_bitmap(this->planetTopography);
        this->planetTopography=NULL;
    }
    if (this->planetScannerMap) 
    {
        destroy_bitmap(this->planetScannerMap);
        this->planetScannerMap=NULL;
    }
    if (this->planetTexture) 
    {
        destroy_bitmap(this->planetTexture);
        this->planetTexture = NULL;
    }

}

bool PlanetaryBody::CreatePlanetTexture()
{
	//these must not change--just noted for reference
    //(these sizes are assumed by planet orbit and planet surface)
    const int TEX_SIZE_ORBIT = 256; 
	const int TEX_SIZE_SURFACE = 500;
	std::string orbitFilename="";
	std::string surfaceFilename="";

	//use starid and planetid for random seed
	randomness = starid * 1000 + planetid;

	//planet textures are stored in data/planetorbit using the starid and planetid as a random seed
	//once created they are re-used
	//two textures are required for each planet as a result of replacing the OpenGL with a software renderer

    //set up planet orbit texture filename
    ostringstream os;
    os << "data/planetorbit/planet_" << randomness << "_256.bmp";
    orbitFilename = os.str();

    //set up planet surface texture filename
	os.str("");
	os << "data/planetorbit/planet_" << randomness << "_500.bmp";
	surfaceFilename = os.str();

    debug << "Planet orbit filename: " << orbitFilename << ", surface filename: " << surfaceFilename << endl;

    //try to find planet texture file PREVIOUSLY saved...
	this->planetTexture=NULL;
    this->planetTexture = (BITMAP*)load_bitmap(orbitFilename.c_str(), NULL);
	if (!this->planetTexture) 
    {
        //TEXTURE FILE NOT FOUND

	    //generate new planet texture for ORBIT 256x256
	    createPlanetSurface(TEX_SIZE_ORBIT, TEX_SIZE_ORBIT, this->randomness, this->planetType, orbitFilename);


	    //generate planet texture for SURFACE 500x500
	    createPlanetSurface(TEX_SIZE_SURFACE, TEX_SIZE_SURFACE, this->randomness, this->planetType, surfaceFilename);


        //load newly generated planet texture
        this->planetTexture = (BITMAP*)load_bitmap(orbitFilename.c_str(), NULL);
	    if (!this->planetTexture) 
        {
		    g_game->message("PlanetOrbit: Error loading planet texture");
		    return false;
	    }
    }


	//get AUX_SCREEN gui values from globals
	static int asw = (int)g_game->getGlobalNumber("AUX_SCREEN_WIDTH");
	static int ash = (int)g_game->getGlobalNumber("AUX_SCREEN_HEIGHT");

    //create planet topography bitmap for minimap
	this->planetTopography = create_bitmap(asw, ash);
	clear_bitmap(this->planetTopography);

	//scale planet texture onto topography, cutting skewed N/S poles (drop 10 pixels from top/bottom)
 	stretch_blit(this->planetTexture, this->planetTopography, 
        0, 10, this->planetTexture->w, this->planetTexture->h-20, 
        1, 1, this->planetTopography->w-2, this->planetTopography->h-2);

	//now create a scratch image as a duplicate of topography used for sensor scans
	this->planetScannerMap = create_bitmap(asw, ash);
    draw_sprite(this->planetScannerMap, this->planetTopography, 0, 0);


    //create texture-mapped sphere of the planet
    this->texSphere = new TexturedSphere(TEX_SIZE_ORBIT);
    if (!this->texSphere->SetTexture(this->planetTexture))
    {
        g_game->fatalerror("PlanearyBody: error loading planet_texture");
        return false;
    }

    return true;
}
