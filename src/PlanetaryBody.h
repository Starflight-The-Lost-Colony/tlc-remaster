/*
	STARFLIGHT - THE LOST COLONY (REMASTERED)
	PlanetaryBody.h - Encapsulates planet textures
	Author: J.Harbour
	Date: Feb, 2024

    fyi, the lazy publics are not bad coding, just for convenience.

    There was a single Create... method previously. It was split into one per planet type
    because of the 4 different gas giant colors used which often resulted in one color 
    for planet orbit, a different color on the surface.

*/

#pragma once

#include "Game.h"
#include "env.h"
#include <allegro.h>
#include "Module.h"
#include "DataMgr.h"
#include "TexturedSphere.h"
#include <noise/noise.h>
#include "noiseutils.h"


class PlanetaryBody
{
private:
    int randomness;

    void CreateTexture_Oceanic(int width, int height, int randomness, std::string filename);
    void CreateTexture_Frozen(int width, int height, int randomness, std::string filename);
    void CreateTexture_Molten(int width, int height, int randomness, std::string filename);
    void CreateTexture_Acidic(int width, int height, int randomness, std::string filename);
    void CreateTexture_Rocky(int width, int height, int randomness, std::string filename);
    void CreateTexture_Asteroid(int width, int height, int randomness, std::string filename);
    void CreateTexture_GasGiant_Red(int width, int height, int randomness, std::string filename);
    void CreateTexture_GasGiant_Purple(int width, int height, int randomness, std::string filename);
    void CreateTexture_GasGiant_Green(int width, int height, int randomness, std::string filename);
    void CreateTexture_GasGiant_Blue(int width, int height, int randomness, std::string filename);

public:
    int starid;
    int planetid;
    int planetRadius;
    PlanetType planetType;

    double perlinFrequency;
    int perlinOctaves;
    double perlinPersistence;
    double perlinLightContrast;
    double perlinLightBrightness;
    utils::Color perlinLightColor;


    int lightmapOffsetX;
    int lightmapOffsetY;
    std::string lightmapFilename;
    BITMAP *lightmapOverlay;

    TexturedSphere *planetRenderObj;

    BITMAP *planetTexture;    //HD
    BITMAP *planetTexture256; //orbit
    BITMAP *planetTexture500; //surface
    BITMAP *planetTopography;
    BITMAP *planetScannerMap;

    PlanetaryBody();
    ~PlanetaryBody();

    bool CreatePlanetTextures(int starid, int planetid);
    void CreateTexture(PlanetType planetType, int width, int height, int randomness, std::string filename);



};

