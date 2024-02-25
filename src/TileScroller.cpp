
#include "env.h"
#include <allegro.h>
#include "TileScroller.h"

TileScroller::TileScroller()
{
   this->scrollbuffer = NULL;
   this->tiles = NULL;
   this->tilewidth = 0;
   this->tileheight = 0;
   this->tilesacross = 0;
   this->tilesdown = 0;
   this->scrollx = 0.0f;
   this->scrolly = 0.0f;
   this->windowwidth = 0;
   this->windowheight = 0;
   this->columns = 1;
   this->rows = 1;
   this->ResetTiles();
   this->bLoaded = false;
}

TileScroller::~TileScroller()
{
	this->Destroy();
}

void TileScroller::Destroy()
{
	if (this->scrollbuffer) 
	{
		destroy_bitmap(this->scrollbuffer);
		this->scrollbuffer = NULL;
	}

	if (this->bLoaded && this->tiles) 
	{
		destroy_bitmap(this->tiles);
		this->tiles = NULL;
	}
}

void TileScroller::ResetTiles() 
{
   for (int one=0; one<MAX_SCROLL_SIZE; one++)
	  for (int two=0; two<MAX_SCROLL_SIZE; two++)
		 this->tiledata[one][two] = 0;
}

void TileScroller::SetTile(int col, int row, short value)
{
   this->tiledata[col][row] = value;
}

short TileScroller::GetTile(int col, int row)
{
   return this->tiledata[col][row];
}

short TileScroller::GetTilebyCoords(int x, int y)
{
   return this->tiledata[ x / this->tilewidth ][ y / this->tileheight ];
}

void TileScroller::SetTileImage(BITMAP *image) 
{ 
	if (!image) return;
	
	//if tile image was previously loaded, free it's memory
	if (this->bLoaded && this->tiles) { destroy_bitmap(tiles); }
	
	if (image) 
    {
		this->tiles = image; 
		//image is now a pointer, not loaded
		bLoaded = false;
	}
}

bool TileScroller::LoadTileImage(char *filename)
{
	this->tiles = (BITMAP*)load_bitmap(filename, NULL);
	if (this->tiles) 
		this->bLoaded = true;
	else 
		this->bLoaded = false;

	return this->bLoaded;
}

bool TileScroller::CreateScrollBuffer(int width,int height)
{
    this->windowwidth = width;
    this->windowheight = height;
    this->scrollbuffer = (BITMAP*)create_bitmap(width + this->tilewidth * 2, height + this->tileheight * 2);
    return (this->scrollbuffer != NULL);
}

void TileScroller::SetScrollPosition(float x,float y)
{
   this->scrollx = x;
   this->scrolly = y;
}

void TileScroller::SetScrollPosition(Point2D p) 
{
   this->scrollx = p.x; 
   this->scrolly = p.y;
}

void TileScroller::SetRegionSize(int w,int h)
{
   if (w >= 0 && w <= MAX_SCROLL_SIZE) this->tilesacross = w; 
   if (h >= 0 && h <= MAX_SCROLL_SIZE) this->tilesdown = h;
}

/**
 * Fills the scroll buffer with tiles based on current scrollx,scrolly
**/
void TileScroller::UpdateScrollBuffer()
{
    short tilenum=0,left=0,top=0;

    //prevent a crash
    if ( (!this->scrollbuffer) || (!this->tiles) ) return;
    if (this->tilewidth < 1 || this->tileheight < 1) return;

    //calculate starting tile position
    int tilex = (int)this->scrollx / this->tilewidth;
    int tiley = (int)this->scrolly / this->tileheight;

    //calculate the number of columns and rows
    int cols = this->windowwidth / this->tilewidth;
    int rows = this->windowheight / this->tileheight;

    //draw tiles onto the scroll buffer surface
    int tx,ty;
    for (int y=0; y<=rows; y++) 
    {
	    for (int x=0; x<=cols; x++)	
        {
            tx = tilex + x;
            if (tx < 0) tx = 0;
            ty = tiley + y;
            if (ty < 0) ty = 0;
            tilenum = tiledata[tx][ty];

            left = (tilenum % this->columns) * this->tilewidth;
            top = (tilenum / this->columns) * this->tileheight;

            blit( this->tiles, this->scrollbuffer, left, top, x * this->tilewidth, y * this->tileheight, this->tilewidth, this->tileheight);
		}
	}
}

/**
 * Draws the portion of the scroll buffer based on the current partial tile scroll position.
**/ 
void TileScroller::DrawScrollWindow(BITMAP *dest, int x, int y, int width, int height)
{
	//prevent a crash
	if (this->tilewidth < 1 || this->tileheight < 1) return;
	
	//calculate the partial sub-tile lines to draw using modulus
	int partialx = (int)this->scrollx % this->tilewidth;
	int partialy = (int)this->scrolly % this->tileheight;
	
	//draw the scroll buffer to the destination bitmap
	if (this->scrollbuffer)
		blit( this->scrollbuffer, dest, partialx, partialy, x, y, width, height );
}
