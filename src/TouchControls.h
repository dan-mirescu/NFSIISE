// SPDX-License-Identifier: MIT

#ifndef TOUCHCONTROLS_H
#define TOUCHCONTROLS_H

#include "Wrapper.h"

#include <SDL3/SDL.h>

/* On-screen touch buttons.
 *
 * The buttons do not talk to the game directly: a press pushes a synthetic
 * SDL key event, which the normal User32.c message pump turns into
 * WM_KEYDOWN/WM_KEYUP, exactly like a real keyboard.
 *
 * Geometry is produced as plain triangles so the renderer (Glide2x/OpenGL2.c)
 * can draw them with a trivial shader; no textures and no assets are needed.
 */

/* Configuration (see nfs2se.conf) */
void touchControlsSetEnabled(BOOL enabled);
void touchControlsSetOpacity(int32_t percent);  /* 0 - 100, default 40 */
void touchControlsSetScale(int32_t percent);    /* 50 - 300, default 100 */
BOOL touchControlsSetLayout(const char *spec);  /* "name:cx,cy,r;…" in percent */
BOOL touchControlsAreEnabled(void);

/* Input: coordinates are normalized (0 - 1, y pointing down), as SDL
   reports them for touch events.  Each function returns true when the
   event belongs to a button and must not be used for anything else. */
BOOL touchControlsFingerDown(SDL_FingerID finger, float nx, float ny);
BOOL touchControlsFingerMotion(SDL_FingerID finger, float nx, float ny);
BOOL touchControlsFingerUp(SDL_FingerID finger);
void touchControlsReleaseAll(void);

/* Triangles to draw, 6 floats per vertex: x, y (0 - 1 window space, y down)
   followed by r, g, b, a.  Returns NULL when there is nothing to draw. */
const float *touchControlsGeometry(int32_t winW, int32_t winH, int32_t *vertexCount);

#endif /* TOUCHCONTROLS_H */
