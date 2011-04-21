/*****************************************************************
(c)2002 Eric Lasota/Orbiter Productions

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to

The Free Foundation, Inc.
59 Temple Place, Suite 330
Boston, MA  02111-1307  USA
*****************************************************************/

#ifndef __UIX_H__
#define __UIX_H__

/*
  UIX is a package I created for C++ interaction with the Quake 2 draw code.
  It is mostly string based, but can be modified to do other stuff.
*/

typedef struct
{
	void (*DrawString) (const int x, const int y, const char *str, ...);
	void (*DrawAltString) (const int x, const int y, const char *str, ...);
	void (*DrawPic) (int x, int y, char *name);
	void (*DrawStretchPic) (int x, int y, int w, int h, char *name);
	void (*Exec) (char *str, ...);

	float (*CvarValue) (const char *cvarname);
	const char *(*CvarString) (const char *cvarname);

	void (*SetCvarValue) (const char *cvarname, const float value);
	void (*SetCvarString) (const char *cvarname, const char *value);

	void (*ExitDesktop) ();
	void (*ExitToConsole) ();

	void (*MoveMouse) (const int x, const int y);

	void (*PlaySound) (const char *str);

	int (*StringToKeynum) (char *str);
	char *(*KeynumToString) (int i);

	const char *(*GetBind) (const int keynum);
	void (*SetBind) (int keynum, char *command);
} uix_import_t;

typedef struct
{
	void (*DrawObjects) (const int mousex, const int mousey, const int screensizex, const int screensizey);
	void (*KeyEvent) (const int key);
	void (*Init) ();

	void (*MouseDown) ();
	void (*MouseUp) ();
} uix_export_t;

#endif