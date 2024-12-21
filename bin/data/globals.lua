-- GLOBALS SCRIPT FILE

-- include the user modifiable options
dofile("data/config.lua")

GAME_TITLE = "Starflight-The Lost Colony"
GAME_VERSION = "2.0"
--STARTUPMODULE = "STARTUP"
--STARTUPMODULE = "SETTINGS"
--STARTUPMODULE = "PLANETORBIT"
--STARTUPMODULE = "STARPORT"
STARTUPMODULE = "TITLESCREEN"
ESCAPEMODULE = "TITLESCREEN"

--to run without the 60 fps limiter
UNLIMITED_FRAMERATE = false

--to enable testing mode for modules in development
DEBUG_MODE = true

--to display debugging info on the screen
DEBUG_CORE = true

--to see the system cursor in unscaled position
DEBUG_MOUSE = true


--SHIP SCALE VALUE (1.00 = 128x128)
--MUST BE 4 CHARACTERS
--only recognizes: 0.25, 0.50, 0.75, 1.00
--any other values will result in 0.50
SHIP_SPRITE_SCALE = 0.50

--acceleration increases velocity per FRAME
--this adds up FAST (40x per sec) so don't go too high
ENGINE1_ACCEL = 0.04	-- base
ENGINE2_ACCEL = 0.06	-- + .02
ENGINE3_ACCEL = 0.08	-- + .02
ENGINE4_ACCEL = 0.10	-- + .02
ENGINE5_ACCEL = 0.12	-- + .02
ENGINE6_ACCEL = 0.14	-- + .02
--ENGINE7_ACCEL = 0.0

--this should stay between 2.0 to 3.0
--speeds < 2.0 are too tediously slow to be fun
--speeds > 3.0 are too fast to be realistic or practical
ENGINE1_TOPSPEED = 2.0 -- base
ENGINE2_TOPSPEED = 2.2 -- + .2
ENGINE3_TOPSPEED = 2.4 -- + .2
ENGINE4_TOPSPEED = 2.6 -- + .2
ENGINE5_TOPSPEED = 2.8 -- + .2
ENGINE6_TOPSPEED = 3.0 -- + .2
--ENGINE7_TOPSPEED = 0.0

--affects ship maneuverability, probably the MOST important feature of the ship engines
--this alone will compel player to upgrade their ship so don't make it too quick early on
--testing revealed a fun range between 2.0 and 4.0
ENGINE1_TURNRATE = 1.8	-- base
ENGINE2_TURNRATE = 2.2	-- + .4
ENGINE3_TURNRATE = 2.6	-- + .4
ENGINE4_TURNRATE = 3.0	-- + .4
ENGINE5_TURNRATE = 3.4	-- + .4
ENGINE6_TURNRATE = 3.8	-- + .4
--ENGINE7_TURNRATE = 0.0

--these should be proportional to weapon damage per second (dps)
--a shield should hold up for at least 3 seconds (see class 1 weaps)
SHIELD1_STRENGTH = 140 	-- (at least 3 seconds of missile-1 hits)
SHIELD2_STRENGTH = 240	-- +100	
SHIELD3_STRENGTH = 350 	-- +110
SHIELD4_STRENGTH = 470 	-- +120
SHIELD5_STRENGTH = 600 	-- +130
SHIELD6_STRENGTH = 740  -- +140
--SHIELD7_STRENGTH = 0

--these should be the same as shield strengths
ARMOR1_STRENGTH = 140 	-- (at least 3 seconds of missile-1 hits)
ARMOR2_STRENGTH = 240 	-- +100
ARMOR3_STRENGTH = 350 	-- +110
ARMOR4_STRENGTH = 470 	-- +120
ARMOR5_STRENGTH = 600 	-- +130
ARMOR6_STRENGTH = 740 	-- +140


--weapon speed/range (milliseconds)
LASER_DURATION   = 1000	 -- range in ms
LASER_SPEED      = 30	 -- weapon speed in pixels/frame
MISSILE_DURATION = 8000  -- range in ms
MISSILE_SPEED    = 15 	 -- weapon speed in pixels/frame

-- alien weapon ranges
ALIEN_MISSILE_RANGE = 6000
ALIEN_LASER_RANGE   = 1000
ALIEN_SAFETY_DISTANCE = 400	--not used. "flee" code is commented out. 


--laser fires at same rate always but higher classes do more damage 
--laser 1 can do 20 pts of dps with direct hits
LASER1_FIRERATE = 200 --milliseconds: 200 = 1/5 second
LASER2_FIRERATE = 200 
LASER3_FIRERATE = 200 
LASER4_FIRERATE = 200 
LASER5_FIRERATE = 200 
LASER6_FIRERATE = 200 
LASER7_FIRERATE = 200
LASER8_FIRERATE = 200
LASER9_FIRERATE = 200 
LASER1_DAMAGE = 4  	-- base
LASER2_DAMAGE = 8  	-- +4
LASER3_DAMAGE = 14 	-- +6
LASER4_DAMAGE = 22 	-- +8
LASER5_DAMAGE = 32 	-- +10
LASER6_DAMAGE = 44 	-- +12
LASER7_DAMAGE = 56	-- Thrynn special case upgrade (science)
LASER8_DAMAGE = 72	-- Thrynn special case upgrade (freelance)
LASER9_DAMAGE = 86	-- Thrynn special case upgrade (military)

--missile damage will always be much higher than lasers
MISSILE1_FIRERATE = 900  -- (500 = 1/2 second)
MISSILE2_FIRERATE = 900 
MISSILE3_FIRERATE = 900 
MISSILE4_FIRERATE = 900 
MISSILE5_FIRERATE = 900 
MISSILE6_FIRERATE = 900 
MISSILE7_FIRERATE = 900 
MISSILE8_FIRERATE = 900 
MISSILE9_FIRERATE = 900 
MISSILE1_DAMAGE = 40	-- base
MISSILE2_DAMAGE = 60 	-- +20
MISSILE3_DAMAGE = 90	-- +30
MISSILE4_DAMAGE = 130	-- +40
MISSILE5_DAMAGE = 180	-- +50
MISSILE6_DAMAGE = 240	-- +60
MISSILE7_DAMAGE = 280
MISSILE8_DAMAGE = 330
MISSILE9_DAMAGE = 380

--when game starts, this should be next to Myrrdan star system
PLAYER_HYPERSPACE_START_X = 15553
PLAYER_HYPERSPACE_START_Y = 13244

--when player enters star system (should probably be random)
PLAYER_SYSTEM_START_X = 10370
PLAYER_SYSTEM_START_Y = 8350

--when player enters the starport
PLAYER_STARPORT_START_X = 200
PLAYER_STARPORT_START_Y = 430

-- gui components
GUI_MESSAGE_POS_X = 242
GUI_MESSAGE_POS_Y = 543
GUI_MESSAGE_WIDTH = 575
GUI_MESSAGE_HEIGHT = 224
GUI_AUX_POS_X = 0
GUI_AUX_POS_Y = 477
--GUI_SOCKET_POS_X = 333
--GUI_SOCKET_POS_Y = 491

--window at lower left corner (drawing area, not the whole gui panel)
AUX_SCREEN_X = 8
AUX_SCREEN_Y = 533
AUX_SCREEN_WIDTH = 228
AUX_SCREEN_HEIGHT = 228

GUI_GAUGES_POS_X = 236
GUI_GAUGES_POS_Y = 0
GUI_VIEWER_POS_X = -40      -- quest viewer
GUI_VIEWER_POS_Y = 30
GUI_VIEWER_SPEED = 12
GUI_VIEWER_LEFT = -440
GUI_VIEWER_RIGHT = -80
GUI_RIGHT_VIEWER_POS_X = 600
GUI_RIGHT_VIEWER_POS_Y = 85


--START components should add up to 3 (incl 1 pod minimum)
--MAX components should add up to 26 (pods = 1/4 point)

-- profession defaults--freelance
PROF_FREELANCE_ENGINE = 1
PROF_FREELANCE_ARMOR = 0
PROF_FREELANCE_SHIELD = 0
PROF_FREELANCE_LASER = 1
PROF_FREELANCE_MISSILE = 0
PROF_FREELANCE_PODS = 1
PROF_FREELANCE_ENGINE_MAX = 5
PROF_FREELANCE_ARMOR_MAX = 4
PROF_FREELANCE_SHIELD_MAX = 4
PROF_FREELANCE_LASER_MAX = 6
PROF_FREELANCE_MISSILE_MAX = 3
PROF_FREELANCE_PODS_MAX = 16 --4 points

-- profession defaults--science
PROF_SCIENCE_ENGINE = 1
PROF_SCIENCE_ARMOR = 1
PROF_SCIENCE_SHIELD = 0
PROF_SCIENCE_LASER = 0
PROF_SCIENCE_MISSILE = 0
PROF_SCIENCE_PODS = 1
PROF_SCIENCE_ENGINE_MAX = 6
PROF_SCIENCE_ARMOR_MAX = 4
PROF_SCIENCE_SHIELD_MAX = 6
PROF_SCIENCE_LASER_MAX = 4
PROF_SCIENCE_MISSILE_MAX = 3
PROF_SCIENCE_PODS_MAX = 12 --3 points

-- profession defaults--military
PROF_MILITARY_ENGINE = 1
PROF_MILITARY_ARMOR = 0
PROF_MILITARY_SHIELD = 0
PROF_MILITARY_LASER = 1
PROF_MILITARY_MISSILE = 0
PROF_MILITARY_PODS = 1
PROF_MILITARY_ENGINE_MAX = 4	
PROF_MILITARY_ARMOR_MAX = 4	
PROF_MILITARY_SHIELD_MAX = 4	
PROF_MILITARY_LASER_MAX = 6
PROF_MILITARY_MISSILE_MAX = 6
PROF_MILITARY_PODS_MAX = 8 --2 points

