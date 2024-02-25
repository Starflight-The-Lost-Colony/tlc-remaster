#ifndef _TILESCROLLER_H
#define _TILESCROLLER_H 1

#include "env.h"
#include <allegro.h>
#include <stdlib.h>
#include <string.h>
#include "Point2D.h"

#define MAX_SCROLL_SIZE 2500

class TileScroller 
{
private:
   BITMAP *scrollbuffer;
   BITMAP *tiles;

   short tiledata[MAX_SCROLL_SIZE][MAX_SCROLL_SIZE];

   int tilewidth, tileheight, columns, rows;
   int tilesacross, tilesdown;
   float scrollx, scrolly;
   int windowwidth,windowheight;
   
   bool bLoaded;


public:
    ~TileScroller();
    TileScroller();
    void Destroy();
    bool CreateScrollBuffer(int width,int height);
    bool LoadTileImage(char *filename);
    void ResetTiles();

    void SetTile(int col, int row, short value);
    short GetTile(int col, int row);
    short GetTilebyCoords(int x, int y);

    int GetTileImageColumns() { return this->columns; }
    void SetTileImageColumns(int c) { this->columns = c; }

    int GetTileImageRows() { return this->rows; }
    void SetTileImageRows(int r) { this->rows = r; }

    void SetTileSize(int w,int h) { this->tilewidth = w; this->tileheight = h; }
    int GetTileWidth() { return tilewidth;  }
    void SetTileWidth(int width) { this->tilewidth = width; }
    int GetTileHeight() {  return this->tileheight;   }
    void SetTileHeight(int height) { this->tileheight = height; }

    void SetRegionSize(int w,int h);
    int GetTilesAcross() { return this->tilesacross; }
    int GetTilesDown() { return this->tilesdown; }

    void SetWindowSize(int w,int h) { this->windowwidth = w; this->windowheight = h; }
    int GetWindowWidth() { return this->windowwidth; }
    int GetWindowHeight() { return this->windowheight; }

    BITMAP *GetTileImage() { return this->tiles; }
    void SetTileImage(BITMAP *image);

    void SetScrollPosition(float x,float y);
    void SetScrollPosition(Point2D p);
    float GetScrollX() { return this->scrollx; }
    void SetScrollX(int x) { this->scrollx = x; }
    float GetScrollY() { return this->scrolly; }
    void SetScrollY(int y) { this->scrolly = y; }

    void UpdateScrollBuffer();
    void DrawScrollWindow(BITMAP *dest, int x, int y, int width, int height);
};


#endif

