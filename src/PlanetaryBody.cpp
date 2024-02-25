/*
	STARFLIGHT - THE LOST COLONY (REMASTERED)
	PlanetaryBody.cpp - Encapsulates planet textures
	Author: J.Harbour
	Date: Feb, 2024
*/



#include <exception>
#include <iostream>
#include "env.h"
#include "Game.h"
#include "Util.h"
#include "PlanetaryBody.h"
using namespace std;


PlanetaryBody::PlanetaryBody(void)
{
    this->starid = 0;
    this->planetid = 0;
    this->planetRadius = 0;
    this->lightmapOffsetX = 0;
    this->lightmapOffsetY = 0;
    this->lightmapFilename="";
    this->planetType=PT_INVALID;
    this->planetRenderObj=NULL;
    this->planetTexture=NULL;
    this->planetTexture256=NULL;
    this->planetTexture500=NULL;
    this->planetTopography=NULL;
    this->planetScannerMap=NULL;
    this->randomness=0;
    this->perlinFrequency = 0.0;
    this->perlinOctaves = 0;
    this->perlinPersistence = 0.0;
    this->perlinLightContrast = 0.0;
    this->perlinLightBrightness = 0.0;
    this->perlinLightColor = utils::Color(255,255,255,255);

}

PlanetaryBody::~PlanetaryBody(void)
{
    if (this->planetRenderObj)
    {
        delete this->planetRenderObj;
        this->planetRenderObj=NULL;
    }
    if (this->planetTexture256)
    {
        destroy_bitmap(this->planetTexture256);
        this->planetTexture256=NULL;
    }
    if (this->planetTexture500)
    {
        destroy_bitmap(this->planetTexture500);
        this->planetTexture500=NULL;
    }
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


bool PlanetaryBody::CreatePlanetTextures(int starid, int planetid)
{
    const int TEX_SIZE_RENDER = 1024;
    std::string renderFilename="";

    this->starid = starid;
    this->planetid = planetid;

	//use starid and planetid for random seed
	this->randomness = this->starid * 1000 + this->planetid;

	//planet textures are stored in data/planetorbit for reuse
    ostringstream os;
    os << "data/planetorbit/planet_" << this->randomness << "_" << TEX_SIZE_RENDER << ".bmp";
    renderFilename = os.str();
    debug << "Planet texture filename: " << renderFilename << endl;

    //shouldn't happen, but just in case of a repeat call...
    if (this->planetTexture!=NULL)
    {
        delete this->planetTexture;
    }
    this->planetTexture=NULL;

    //try to find planet texture file PREVIOUSLY saved...
    this->planetTexture = (BITMAP*)load_bitmap(renderFilename.c_str(), NULL);

    //TEXTURE FILE NOT FOUND--generate a new one
	if (!this->planetTexture) 
    {
        //use PERLIN to render the HD texture 
	    this->CreateTexture(this->planetType, TEX_SIZE_RENDER, TEX_SIZE_RENDER, this->randomness, renderFilename);

        //load newly generated planet texture
        this->planetTexture = (BITMAP*)load_bitmap(renderFilename.c_str(), NULL);
	    if (!this->planetTexture) 
        {
		    g_game->message("PlanetOrbit: Error loading planet texture: " + renderFilename);
		    return false;
	    }
    }

    //
    //use the generated texture to create the planet orbit image
    //texture must be reduced to 256x256 (cannot be changed!)
    //

    if (this->planetTexture256!=NULL)
    {
        delete this->planetTexture256;
        this->planetTexture256=NULL;
    }
    this->planetTexture256 = (BITMAP*)create_bitmap(256,256);

    //copy source HD image onto downscaled orbit texture 
    stretch_blit(this->planetTexture, this->planetTexture256, 
        0, 0, TEX_SIZE_RENDER, TEX_SIZE_RENDER,
        0, 0, 256, 256);

    //create the planet orbit renderer object
    this->planetRenderObj = new TexturedSphere(256);
    if (!this->planetRenderObj->SetTexture(this->planetTexture))
    {
        g_game->fatalerror("PlanearyBody: error loading planet_texture");
        return false;
    }

    //
    //use the HD image to create the planet surface tilemap source image
    //image must be reduced to 500x500 (tilemap size cannot be changed!)
    //

    if (this->planetTexture500!=NULL)
    {
        delete this->planetTexture500;
        this->planetTexture500=NULL;
    }
    this->planetTexture500 = (BITMAP*)create_bitmap(500,500);

    //copy source HD image onto downscaled surface image
    stretch_blit(this->planetTexture, this->planetTexture500, 
        0, 0, TEX_SIZE_RENDER, TEX_SIZE_RENDER,
        0, 0, 500, 500);

    //
    // use the generated texture to create the minimap for sensor animation
    //

	//get AUX_SCREEN gui dimensions from globals
	static int asw = (int)g_game->getGlobalNumber("AUX_SCREEN_WIDTH");
	static int ash = (int)g_game->getGlobalNumber("AUX_SCREEN_HEIGHT");

    //create planet topography bitmap for minimap
	this->planetTopography = create_bitmap(asw, ash);
	clear_bitmap(this->planetTopography);

	//scale planet texture onto topography, cutting skewed N/S poles (drop 10 pixels from top/bottom)
 	stretch_blit(this->planetTexture, this->planetTopography, 
        0, 10, this->planetTexture->w, this->planetTexture->h-20, 
        1, 1, this->planetTopography->w-2, this->planetTopography->h-2);

    //just in case of a duplicate call...
    if (this->planetScannerMap!=NULL)
    {
        delete this->planetScannerMap;
        this->planetScannerMap=NULL;
    }
	//now create a scratch image as a duplicate of topography used for sensor scans
    this->planetScannerMap = create_bitmap(asw, ash);
    draw_sprite(this->planetScannerMap, this->planetTopography, 0, 0);


    return true;
}


void PlanetaryBody::CreateTexture(PlanetType planetType, int width, int height, int randomness, std::string filename)
{
    switch (planetType)
    {
    case PT_OCEANIC:
        this->perlinFrequency = 1.0;
        this->perlinOctaves = 6;
        this->perlinPersistence = 0.5;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_Oceanic(width, height, randomness, filename); 
        break;
    case PT_ACIDIC:
        this->perlinFrequency = 1.0;
        this->perlinOctaves = 6;
        this->perlinPersistence = 0.5;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_Acidic(width, height, randomness, filename); 
        break;
    case PT_FROZEN:
        this->perlinFrequency = 1.0;
        this->perlinOctaves = 5;
        this->perlinPersistence = 0.4;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_Frozen(width, height, randomness, filename); 
        break;
    case PT_ROCKY:
        this->perlinFrequency = 1.0;
        this->perlinOctaves = 6;
        this->perlinPersistence = 0.5;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_Rocky(width, height, randomness, filename); 
        break;
    case PT_MOLTEN:
        this->perlinFrequency = 1.4;
        this->perlinOctaves = 6;
        this->perlinPersistence = 0.5;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_Molten(width, height, randomness, filename); 
        break;
    case PT_ASTEROID:
        this->perlinFrequency = 1.0;
        this->perlinOctaves = 6;
        this->perlinPersistence = 0.5;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_Asteroid(width, height, randomness, filename); 
        break;
    case PT_GASGIANT_BLUE:
        this->perlinFrequency = 1.0;
        this->perlinOctaves = 3;
        this->perlinPersistence = 0.3;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_GasGiant_Blue(width, height, randomness, filename); 
        break;
    case PT_GASGIANT_GREEN:
        this->perlinFrequency = 1.0;
        this->perlinOctaves = 3;
        this->perlinPersistence = 0.3;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_GasGiant_Green(width, height, randomness, filename); 
        break;
    case PT_GASGIANT_RED:
        this->perlinFrequency = 1.0;
        this->perlinOctaves = 3;
        this->perlinPersistence = 0.3;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_GasGiant_Red(width, height, randomness, filename); 
        break;
    case PT_GASGIANT_PURPLE:
        this->perlinFrequency = 1.0;
        this->perlinOctaves = 3;
        this->perlinPersistence = 0.3;
        this->perlinLightContrast = 3.0;
        this->perlinLightBrightness = 2.0;
        this->perlinLightColor = utils::Color(255,255,255,255);
        this->CreateTexture_GasGiant_Purple(width, height, randomness, filename); 
        break;
    }
}


void PlanetaryBody::CreateTexture_Oceanic(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //OCEANIC values
    renderer.AddGradientPoint (-1.0000, utils::Color (0, 0, 120, 255)); // deeps
    renderer.AddGradientPoint (-0.2500, utils::Color (0, 0, 255, 255)); // shallow
    renderer.AddGradientPoint ( 0.0000, utils::Color (0, 120, 250, 255)); // shore
    renderer.AddGradientPoint ( 0.0625, utils::Color (180, 180, 60, 255)); // sand
    renderer.AddGradientPoint ( 0.1250, utils::Color (50, 160, 0, 255)); // grass
    renderer.AddGradientPoint ( 0.3750, utils::Color (180, 180, 0, 255)); // dirt
    renderer.AddGradientPoint ( 0.7500, utils::Color (150, 150, 150, 255)); // rock
    renderer.AddGradientPoint ( 1.0000, utils::Color (255, 255, 255, 255)); // snow

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}

void PlanetaryBody::CreateTexture_Frozen(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //FROZEN values
	renderer.AddGradientPoint (-1.0000, utils::Color (65, 65, 150, 255)); // deeps
    renderer.AddGradientPoint (-0.2500, utils::Color (100, 100, 150, 255)); // shallow
    renderer.AddGradientPoint ( 0.0000, utils::Color (150, 150, 150, 255)); // shore
    renderer.AddGradientPoint ( 0.0625, utils::Color (160, 160, 160, 255)); // sand
    renderer.AddGradientPoint ( 0.1250, utils::Color (170, 170, 170, 255)); // grass
    renderer.AddGradientPoint ( 0.3750, utils::Color (200, 200, 200, 255)); // dirt
    renderer.AddGradientPoint ( 0.7500, utils::Color (230, 230, 230, 255)); // rock
    renderer.AddGradientPoint ( 1.0000, utils::Color (255, 255, 255, 255)); // snow

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}

void PlanetaryBody::CreateTexture_Molten(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //MOLTEN values
	renderer.AddGradientPoint (-1.0000, utils::Color (200, 30, 30, 255)); 
	renderer.AddGradientPoint (-0.6000, utils::Color (235, 40, 40, 255)); 
	renderer.AddGradientPoint (-0.3000, utils::Color (255, 50, 50, 255)); 
	renderer.AddGradientPoint ( 0.0000, utils::Color (80, 70, 70, 255));
	renderer.AddGradientPoint ( 0.1250, utils::Color (100, 100, 100, 255)); 
	renderer.AddGradientPoint ( 0.5000, utils::Color (150, 120, 100, 255)); 
	renderer.AddGradientPoint ( 1.0000, utils::Color (130, 140, 140, 255)); 

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}

void PlanetaryBody::CreateTexture_Acidic(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //ACIDIC values
	renderer.AddGradientPoint (-1.0000, utils::Color (0, 115, 27, 255)); // acid
	renderer.AddGradientPoint (-0.2500, utils::Color (0, 255, 0, 255)); // shallow
	renderer.AddGradientPoint ( 0.0000, utils::Color (60, 240, 135, 255)); // shore
    renderer.AddGradientPoint ( 0.1250, utils::Color (155, 50, 80, 255)); // grass
    renderer.AddGradientPoint ( 0.7500, utils::Color (30, 30, 100, 255)); // rock
    renderer.AddGradientPoint ( 1.0000, utils::Color (60, 50, 115, 255)); // snow

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}

void PlanetaryBody::CreateTexture_Rocky(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //ROCKY values
    renderer.AddGradientPoint (-1.0000, utils::Color (120, 100, 100, 255)); // deeps
    renderer.AddGradientPoint (-0.2500, utils::Color (120, 120, 120, 255)); // shallow
    renderer.AddGradientPoint ( 0.0000, utils::Color (160, 150, 160, 255)); // shore
    renderer.AddGradientPoint ( 0.0625, utils::Color (120, 120, 100, 255)); // sand
    renderer.AddGradientPoint ( 0.1250, utils::Color (120, 120, 120, 255)); // grass
    renderer.AddGradientPoint ( 0.3750, utils::Color (150, 160, 170, 255)); // dirt
    renderer.AddGradientPoint ( 0.7500, utils::Color (150, 150, 150, 255)); // rock
    renderer.AddGradientPoint ( 1.0000, utils::Color (160, 150, 160, 255)); // snow

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}

void PlanetaryBody::CreateTexture_Asteroid(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //ASTEROID values
	renderer.AddGradientPoint (-1.0000, utils::Color (0, 0, 0, 255)); 
	renderer.AddGradientPoint (-0.6000, utils::Color (20, 20, 20, 255)); 
	renderer.AddGradientPoint (-0.2000, utils::Color (30, 30, 30, 255)); 
	renderer.AddGradientPoint ( 0.0000, utils::Color (40, 40, 40, 255)); 
	renderer.AddGradientPoint ( 0.1000, utils::Color (50, 50, 50, 255));
	renderer.AddGradientPoint ( 0.3000, utils::Color (60, 60, 60, 255)); 
	renderer.AddGradientPoint ( 0.6000, utils::Color (70, 70, 70, 255)); 
	renderer.AddGradientPoint ( 1.0000, utils::Color (90, 90, 90, 255)); 

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}

void PlanetaryBody::CreateTexture_GasGiant_Red(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //RED GASGIANT values
	renderer.AddGradientPoint (-1.0000, utils::Color (145, 95, 50, 255)); 
	renderer.AddGradientPoint (-0.5000, utils::Color (70, 0, 0, 255)); 
	renderer.AddGradientPoint ( 0.0000, utils::Color (180, 100, 100, 255)); 
	renderer.AddGradientPoint ( 0.5000, utils::Color (150, 0, 0, 255)); 
	renderer.AddGradientPoint ( 1.0000, utils::Color (255, 145, 0, 255));

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}

void PlanetaryBody::CreateTexture_GasGiant_Purple(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //PURPLE GASGIANT values
	renderer.AddGradientPoint (-1.0000, utils::Color (80, 0, 80, 255)); 
	renderer.AddGradientPoint (-0.5000, utils::Color (160, 0, 160, 255)); 
	renderer.AddGradientPoint ( 0.0000, utils::Color (175, 150, 175, 255)); 
	renderer.AddGradientPoint ( 0.5000, utils::Color (182, 99, 182, 255)); 
	renderer.AddGradientPoint ( 1.0000, utils::Color (160, 140, 160, 255)); 

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}

void PlanetaryBody::CreateTexture_GasGiant_Green(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //GREEN GASGIANT values
	renderer.AddGradientPoint (-1.0000, utils::Color (0, 100, 0, 255)); 
	renderer.AddGradientPoint (-0.5000, utils::Color (0, 140, 0, 255)); 
	renderer.AddGradientPoint ( 0.0000, utils::Color (100, 180, 100, 255)); 
	renderer.AddGradientPoint ( 0.5000, utils::Color (0, 150, 0, 255)); 
	renderer.AddGradientPoint ( 1.0000, utils::Color (0, 180, 0, 255)); 

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}

void PlanetaryBody::CreateTexture_GasGiant_Blue(int width, int height, int randomness, std::string filename)
{
    module::Perlin perlin;
    this->randomness = randomness;
    perlin.SetSeed(randomness);
    perlin.SetFrequency(this->perlinFrequency);
    perlin.SetOctaveCount(this->perlinOctaves);
    perlin.SetPersistence(this->perlinPersistence);

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    srand(randomness);

    //BLUE GASGIANT values
	renderer.AddGradientPoint (-1.0000, utils::Color (0, 45, 110, 255)); 
	renderer.AddGradientPoint (-0.5000, utils::Color (0, 0, 140, 255)); 
	renderer.AddGradientPoint ( 0.0000, utils::Color (100, 100, 180, 255)); 
	renderer.AddGradientPoint ( 0.5000, utils::Color (0, 100, 180, 255)); 
	renderer.AddGradientPoint ( 1.0000, utils::Color (100, 190, 210, 255));

    renderer.EnableLight();
    renderer.SetLightContrast(this->perlinLightContrast);
    renderer.SetLightBrightness(this->perlinLightBrightness);
    renderer.SetLightColor(this->perlinLightColor);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(filename);
    writer.WriteDestFile();
}


