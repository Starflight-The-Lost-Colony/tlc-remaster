/*
	STARFLIGHT - THE LOST COLONY
    PLAYERSHIPSPRITE CLASS
*/

#include "env.h"
#include "Game.h"
#include "GameState.h"
#include "PlayerShipSprite.h"
#include "Timer.h"
#include "Util.h"

using namespace std;

//slow down ship movements to 60 fps
const int TIMER_RATE = 16;

PlayerShipSprite::PlayerShipSprite()
{
	//initialize and load the ship sprite
	this->ship = new Sprite();
	switch(g_game->gameState->getProfession())
	{
		case PROFESSION_FREELANCE:
			this->ship->load("data/spacetravel/freelance_128.bmp");
			break;
		case PROFESSION_MILITARY:
			this->ship->load("data/spacetravel/military_128.bmp");
			break;
		case PROFESSION_SCIENTIFIC:
            this->ship->load("data/spacetravel/science_128.bmp");
            break;
		default:
            g_game->fatalerror("FATAL ERROR: player's profession is invalid!");
            break;
	}
	if (!ship) {
		g_game->message("Error loading player ship image");
		return;
	}


    //SHIP SCALING
    //these values result in faster rendering vs using a %
    double scale=0.0;
    string s = g_game->getGlobalString("SHIP_SPRITE_SCALE");
    debug << "SHIP_SPRITE_SCALE = " << s << endl;
    if (s=="1") s="1.00";
    if (s.length() < 4) s="0.50"; //default to 64
    s = Util::Left(s,4);
    if (s=="0.25") scale = 0.25; else
    if (s=="0.50") scale = 0.5; else
    if (s=="0.75") scale = 0.75; else
    if (s=="1.00") scale = 1.0;
    else scale=0.5;
    this->shipScale = scale;
    
   
    this->ship->setFrameWidth( this->ship->getWidth() );
    this->ship->setFrameHeight( this->ship->getHeight() );
	this->ship->setAnimColumns( 1 );
	this->ship->setTotalFrames( 1 );
	this->ship->setCurrFrame( 0 );
	this->ship->setX( SCREEN_WIDTH/2 - this->ship->getFrameWidth()/2 );
	this->ship->setY( SCREEN_HEIGHT/2 - this->ship->getFrameHeight()/2 );


    // keekping the rotation angle consistent - JJH
	if (g_game->CrossModuleAngle == 0) 
    {
		this->ship->setFaceAngle( Util::Random(1,359) );
	}
	else {
		this->ship->setFaceAngle( g_game->CrossModuleAngle );	
	}


	//ship movement variables
	this->maximumVelocity = this->GetMaximumVelocity();
	this->forwardThrust = this->GetForwardThrust();
	this->reverseThrust = this->GetReverseThrust();
	this->lateralThrust = this->GetLateralThrust();
	this->turnRate = this->GetTurnRate();
	//reset ship control timer
	this->timer.reset(); this->braking_timer.reset();
	
}

PlayerShipSprite::~PlayerShipSprite()  
{
	this->Destroy();
	this->ship = NULL;
	//lua_close(L);
}

void PlayerShipSprite::Destroy() 
{
	delete this->ship;
}


void PlayerShipSprite::Draw(BITMAP *dest) 
{
    if (this->shipScale == 1.0)
    {
    	this->ship->DrawRotated( dest, (int)this->ship->getFaceAngle() ); 
        return;
    }

    this->ship->DrawScaledRotated( dest, this->shipScale, (int)this->ship->getFaceAngle() );

}




#pragma region MOVEMENT

void PlayerShipSprite::TurnLeft()
{
	if (!timer.stopwatch(TIMER_RATE)) return;

    double angle = this->ship->getFaceAngle() - this->turnRate;
	this->ship->setFaceAngle(angle);
	if (this->ship->getFaceAngle() < 0) 
        this->ship->setFaceAngle(360 + this->ship->getFaceAngle());

    ship->setCurrFrame(4);
}

void PlayerShipSprite::TurnRight()
{
	if (!this->timer.stopwatch(TIMER_RATE)) return;

    this->ship->setFaceAngle( this->ship->getFaceAngle() + this->turnRate );
	if (this->ship->getFaceAngle() > 359) ship->setFaceAngle(360 - this->ship->getFaceAngle());
}

// Retrieve maximum_velocity variable 
double PlayerShipSprite::GetMaximumVelocity()
{
	double topspeed = 1.0;

	int engine = g_game->gameState->getShip().getEngineClass();
	if (engine < 1 || engine > 6) 
    {
		engine = 1;
		debug << "*** Error in PlayerShipSprite::getMaximumVelocity: Engine class is invalid" << endl;
	}

	switch(engine) 
    {
		case 1: topspeed = g_game->getGlobalNumber("ENGINE1_TOPSPEED"); break;
		case 2: topspeed = g_game->getGlobalNumber("ENGINE2_TOPSPEED"); break;
		case 3: topspeed = g_game->getGlobalNumber("ENGINE3_TOPSPEED"); break;
		case 4: topspeed = g_game->getGlobalNumber("ENGINE4_TOPSPEED"); break;
		case 5: topspeed = g_game->getGlobalNumber("ENGINE5_TOPSPEED"); break;
		case 6: topspeed = g_game->getGlobalNumber("ENGINE6_TOPSPEED"); break;
	}

	return topspeed;
}

// Read forward thrust from script
double PlayerShipSprite::GetForwardThrust()
{
	double accel = 1.0;

	int engine = g_game->gameState->getShip().getEngineClass();
	if (engine < 1 || engine > 6) 
    {
		engine = 1;
		debug << "*** Error in PlayerShipSprite::getForwardThrust: Engine class is invalid" << endl;
	}

	switch(engine) {
		case 1: accel = g_game->getGlobalNumber("ENGINE1_ACCEL"); break;
		case 2: accel = g_game->getGlobalNumber("ENGINE2_ACCEL"); break;
		case 3: accel = g_game->getGlobalNumber("ENGINE3_ACCEL"); break;
		case 4: accel = g_game->getGlobalNumber("ENGINE4_ACCEL"); break;
		case 5: accel = g_game->getGlobalNumber("ENGINE5_ACCEL"); break;
		case 6: accel = g_game->getGlobalNumber("ENGINE6_ACCEL"); break;
	}

	return accel;
}

// Read reverse thrust from script
double PlayerShipSprite::GetReverseThrust()
{
	return GetForwardThrust() * 0.75;
}

// Read lateral thrust from script
double PlayerShipSprite::GetLateralThrust()
{
	return GetForwardThrust() * 0.75;
}

// Read turning rate from script
double PlayerShipSprite::GetTurnRate()
{
	double turnrate = 1.0;

	int engine = g_game->gameState->getShip().getEngineClass();
	if (engine < 1 || engine > 6) 
    {
		engine = 1;
		debug << "*** Error in PlayerShipSprite::getTurnRate: Engine class is invalid" << endl;
	}

	switch(engine) 
    {
		case 1: turnrate = g_game->getGlobalNumber("ENGINE1_TURNRATE"); break;
		case 2: turnrate = g_game->getGlobalNumber("ENGINE2_TURNRATE"); break;
		case 3: turnrate = g_game->getGlobalNumber("ENGINE3_TURNRATE"); break;
		case 4: turnrate = g_game->getGlobalNumber("ENGINE4_TURNRATE"); break;
		case 5: turnrate = g_game->getGlobalNumber("ENGINE5_TURNRATE"); break;
		case 6: turnrate = g_game->getGlobalNumber("ENGINE6_TURNRATE"); break;
	}

	return turnrate;
}

void PlayerShipSprite::LimitVelocity()
{
	double vx = 0;
	double vy = 0;

	//get current velocity	
	vx = this->ship->getVelX();
	vy = this->ship->getVelY();

    double speed = sqrt(vx*vx + vy*vy);
    double dir = atan2(vy,vx);
        
    if (speed > this->maximumVelocity){
	    speed = this->maximumVelocity;
    }else if(speed < -this->maximumVelocity){
	    speed = -this->maximumVelocity;
    }

    vx = speed * cos(dir);
    vy = speed * sin(dir);

	//set new velocity	
	this->ship->setVelX(vx);
	this->ship->setVelY(vy);
}


/*
 * Both side thrusters fire to move ship toward the starboard (right)
 */
void PlayerShipSprite::Starboard()
{
	if (!this->timer.stopwatch(TIMER_RATE)) return

    this->ship->setMoveAngle( this->ship->getFaceAngle() );
	this->ship->setVelX( this->ship->getVelX() + this->ship->calcAngleMoveX( this->ship->getMoveAngle() ) * lateralThrust);
	this->ship->setVelY( this->ship->getVelY() + this->ship->calcAngleMoveY( this->ship->getMoveAngle() ) * lateralThrust);
	LimitVelocity();
}

/*
 * Both side thrusters fire together to move ship toward the port (left)
 */
void PlayerShipSprite::Port()
{
	if (!this->timer.stopwatch(TIMER_RATE)) return;
	//move angle should be 180 degrees clockwise from face angle
	this->ship->setMoveAngle( this->ship->getFaceAngle() - 180 );
	this->ship->setVelX( this->ship->getVelX() + this->ship->calcAngleMoveX( this->ship->getMoveAngle() ) * lateralThrust );
	this->ship->setVelY( this->ship->getVelY() + this->ship->calcAngleMoveY( this->ship->getMoveAngle() ) * lateralThrust );
	LimitVelocity();
}

void PlayerShipSprite::ApplyThrust()
{
	if (!this->timer.stopwatch(TIMER_RATE)) return;
	this->ship->setMoveAngle( this->ship->getFaceAngle() - 90 ); 
	if ( this->ship->getMoveAngle() < 0) this->ship->setMoveAngle( 359 + this->ship->getMoveAngle() );
	this->ship->setVelX( this->ship->getVelX() + this->ship->calcAngleMoveX( this->ship->getMoveAngle() )  * forwardThrust );
	this->ship->setVelY( this->ship->getVelY() + this->ship->calcAngleMoveY( this->ship->getMoveAngle() ) * forwardThrust );
	LimitVelocity();
}

void PlayerShipSprite::ReverseThrust()
{
	if (!this->timer.stopwatch(TIMER_RATE)) return;
	this->ship->setMoveAngle( this->ship->getFaceAngle() - 270 );
	if (ship->getMoveAngle() < 0) ship->setMoveAngle(359 + ship->getMoveAngle() );
	this->ship->setVelX( this->ship->getVelX() + this->ship->calcAngleMoveX( this->ship->getMoveAngle() ) * reverseThrust );
	this->ship->setVelY( this->ship->getVelY() + this->ship->calcAngleMoveY( this->ship->getMoveAngle() ) * reverseThrust );
	LimitVelocity();
}

void PlayerShipSprite::Cruise() 
{ 
	this->ship->setCurrFrame(0);
}

void PlayerShipSprite::AllStop()
{
	this->ship->setVelX(0.0);
	this->ship->setVelY(0.0);
	this->ship->setCurrFrame(0);
}

void PlayerShipSprite::ApplyBraking()
{
	if (!this->braking_timer.stopwatch(TIMER_RATE)) return;

	//get current velocity
	double velx = this->ship->getVelX();
	double vely = this->ship->getVelY();

	//if done braking, then exit
	if ((velx == 0) && (vely == 0)) 
    {
		this->ship->setCurrFrame(0);
		return;
	}

    //this minimum speed will stop the ship if it's "drifting" slowly
    double driftThreshold = 0.05;
    double brakeValue = 0.04;
    double speed =  sqrt(velx*velx + vely*vely);
    if (fabs(speed) < driftThreshold)
		speed = 0.0;
    else {
        if (speed > 0.0)
            speed -= brakeValue;
        else
            speed += brakeValue;
    }

    //get x,y velocity based on ship's direction
    double dir = atan2(vely,velx);
    velx = speed * cos(dir);
    vely = speed * sin(dir);
	
	//set new velocity
	ship->setVelX(velx);
	ship->setVelY(vely);
	ship->setCurrFrame(0);
}


float PlayerShipSprite::GetVelocityX() 
{ 
	return this->ship->getVelX(); 
}

float PlayerShipSprite::GetVelocityY() 
{ 
	return this->ship->getVelY(); 
}

void PlayerShipSprite::SetVelocityX(float value)
{
	this->ship->setVelX( value );
}

void PlayerShipSprite::SetVelocityY(float value)
{
	this->ship->setVelY( value );
}

float PlayerShipSprite::GetRotationAngle() 
{
	return this->ship->getFaceAngle(); 
}

void PlayerShipSprite::Reset()
{
	this->AllStop();
	this->ship->setCurrFrame(0);
}

double PlayerShipSprite::GetCurrentSpeed()
{
	double vx = this->GetVelocityX();
	double vy = this->GetVelocityY();
	double speed = sqrt(vx*vx + vy*vy);
	return speed;
}

#pragma endregion



