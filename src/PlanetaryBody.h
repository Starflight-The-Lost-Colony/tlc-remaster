/*
	STARFLIGHT - THE LOST COLONY (REMASTERED)
	PlanetaryBody.h - Encapsulates planet textures
	Author: J.Harbour
	Date: Feb, 2024
*/

#pragma once

#include "Game.h"
#include "env.h"
#include <allegro.h>
#include "Module.h"
#include "DataMgr.h"
#include "TexturedSphere.h"


class PlanetaryBody
{
public:
    PlanetaryBody();
    ~PlanetaryBody();

    bool CreatePlanetTexture();

    void setStarID(int v) { this->starid = v; }
    int getStarID() { return this->starid; }
    void setPlanetID(int v) { this->planetid = v; }
    int getPlanetID() { return this->planetid; }
    void setPlanetRadius(int r) { this->planetRadius = r; }
    int getPlanetRadius() { return this->planetRadius; }
    PlanetType getPlanetType() { return this->planetType; }
    void setPlanetType(PlanetType pt) { this->planetType = pt; }

    int randomness;
    int starid, planetid, planetRadius;

    int lightmapOffsetX, lightmapOffsetY;
    std::string lightmapFilename;
    BITMAP *lightmapOverlay;

    PlanetType planetType;

    TexturedSphere *texSphere;

    BITMAP *planetTexture;
    BITMAP *planetTopography;
    BITMAP *planetScannerMap;

private:

};

