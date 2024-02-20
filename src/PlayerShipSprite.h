/*
	STARFLIGHT - THE LOST COLONY
	PlayerShipSprite.h
	Author: J.Harbour
	Date: 2007
*/

#pragma once

#include "env.h"
#include <allegro.h>
#include <string>
#include "Sprite.h"
#include "Script.h"
#include "Timer.h"

class PlayerShipSprite {
protected:
	Sprite	*ship;
	double maximumVelocity;
	double	forwardThrust;
	double	reverseThrust;
	double	lateralThrust;
	double	turnRate;
	Timer timer, braking_timer;
    double shipScale;

public:
	~PlayerShipSprite();
	PlayerShipSprite();
	void Destroy();
	void ApplyThrust();
	void ReverseThrust();
	void LimitVelocity();
	void Cruise();
	void AllStop();
	void TurnLeft();
	void TurnRight();
	void Starboard();
	void Port();
	void ApplyBraking();

	void Draw(BITMAP *dest);
	double GetX() { return ship->getX(); }
	double GetY() { return ship->getY(); }
	void SetX(double value) { ship->setX(value); }
	void SetX(int value)    { ship->setX(value); }
	void SetY(double value) { ship->setY(value); }
	void SetY(int value)    { ship->setY(value); }
	float GetVelocityX();
	float GetVelocityY();
	void SetVelocityX(float value);
	void SetVelocityY(float value);
	float GetRotationAngle();
	double GetCurrentSpeed();

	void Reset();
	
	double GetMaximumVelocity();
	double GetForwardThrust();
	double GetReverseThrust();
	double GetLateralThrust();
	double GetTurnRate();
	
	Sprite *GetSprite() { return ship; }

};

