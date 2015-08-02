/*
Copyright (C) 2007 q2xp team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef BOTSTRUCT
#define BOTSTRUCT

//Zigock client info
#define ALEAT_MAX	10

typedef struct zgcl_s {
	int zclass;					// class no.

	int botindex;				// botlist's index NO.

	// qtrue clientp zoom
	int aiming;					// 0-not 1-aiming 2-firing zoomingflag
	float distance;				// zoom
	float olddistance;			// zooming FOV
	qboolean autozoom;			// autozoom
	qboolean lockon;			// lockon flag qfalse-not qtrue-locking

	// botp
	int zcstate;				// status
	int zccmbstt;				// combat status

	// duck
	float n_duckedtime;			// non ducked time

	// targets
	edict_t *first_target;		// enemy uses LockOntarget(for client)
	float targetlock;			// target locking time
	short firstinterval;		// enemy search count
	edict_t *second_target;		// kindof items
	short secondinterval;		// item pickup call count

	// waiting
	vec3_t movtarget_pt;		// moving target waiting point
	edict_t *waitin_obj;		// for waiting sequence complete

	// basical moving
	float moveyaw;				// qtrue moving yaw

	// combat
	int total_bomb;				// total put bomb
	float gren_time;			// grenade time

	// contents
	//  int         front_contents;
	int ground_contents;
	float ground_slope;

	// count (inc only)
	int tmpcount;

	// moving hist
	float nextcheck;			// checking time
	vec3_t pold_origin;			// old origin
	vec3_t pold_angles;			// old angles

	// target object shot
	qboolean objshot;


	edict_t *sighten;			// sighting enemy to me info from entity
	// sight
	edict_t *locked;			// locking enemy to me info from lockon
	// missile

	// waterstate
	int waterstate;

	// route
	qboolean route_trace;
	int routeindex;				// routing index
	float rt_locktime;
	float rt_releasetime;
	qboolean havetarget;		// target on/off
	int targetindex;

	// battle
	edict_t *last_target;		// old enemy
	vec3_t last_pos;			// old origin
	int battlemode;				// mode
	int battlecount;			// temporary count
	int battlesubcnt;			// subcount
	int battleduckcnt;			// duck
	float fbattlecount;			// float temoporary count
	vec3_t vtemp;				// temporary vec
	int foundedenemy;			// foundedenemy
	char secwep_selected;		// secondweapon selected

	vec3_t aimedpos;			// shottenpoint
	qboolean trapped;			// trapflag

	// team
	short tmplstate;			// teamplay state
	short ctfstate;				// ctf state
	edict_t *followmate;		// follow
	float matelock;				// team mate locking time
} zgcl_t;

#endif
