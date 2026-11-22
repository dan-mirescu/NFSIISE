// SPDX-License-Identifier: MIT

#include "TouchControls.h"

#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_BUTTONS   12
#define MAX_VERTICES  12000
#define FLOATS_PER_V  6

#define DISC_SEGMENTS 18
#define LABEL_CHARS   3

/* 3x5 pixel font, rows top to bottom, 3 bits per row (MSB = leftmost).
   Index 0-9 = digits, 10-35 = A-Z. */
#define FONT_ROW(a, b, c, d, e) \
	(uint16_t)(((a) << 12) | ((b) << 9) | ((c) << 6) | ((d) << 3) | (e))
static const uint16_t g_font3x5[36] = {
	FONT_ROW(7, 5, 5, 5, 7), /* 0 */
	FONT_ROW(2, 6, 2, 2, 7), /* 1 */
	FONT_ROW(7, 1, 7, 4, 7), /* 2 */
	FONT_ROW(7, 1, 7, 1, 7), /* 3 */
	FONT_ROW(5, 5, 7, 1, 1), /* 4 */
	FONT_ROW(7, 4, 7, 1, 7), /* 5 */
	FONT_ROW(7, 4, 7, 5, 7), /* 6 */
	FONT_ROW(7, 1, 1, 1, 1), /* 7 */
	FONT_ROW(7, 5, 7, 5, 7), /* 8 */
	FONT_ROW(7, 5, 7, 1, 7), /* 9 */
	FONT_ROW(7, 5, 7, 5, 5), /* A */
	FONT_ROW(6, 5, 6, 5, 6), /* B */
	FONT_ROW(7, 4, 4, 4, 7), /* C */
	FONT_ROW(6, 5, 5, 5, 6), /* D */
	FONT_ROW(7, 4, 7, 4, 7), /* E */
	FONT_ROW(7, 4, 7, 4, 4), /* F */
	FONT_ROW(7, 4, 5, 5, 7), /* G */
	FONT_ROW(5, 5, 7, 5, 5), /* H */
	FONT_ROW(7, 2, 2, 2, 7), /* I */
	FONT_ROW(1, 1, 1, 5, 7), /* J */
	FONT_ROW(5, 5, 6, 5, 5), /* K */
	FONT_ROW(4, 4, 4, 4, 7), /* L */
	FONT_ROW(5, 7, 7, 5, 5), /* M */
	FONT_ROW(6, 5, 5, 5, 5), /* N */
	FONT_ROW(7, 5, 5, 5, 7), /* O */
	FONT_ROW(7, 5, 7, 4, 4), /* P */
	FONT_ROW(7, 5, 5, 7, 3), /* Q */
	FONT_ROW(7, 5, 6, 5, 5), /* R */
	FONT_ROW(7, 4, 7, 1, 7), /* S */
	FONT_ROW(7, 2, 2, 2, 2), /* T */
	FONT_ROW(5, 5, 5, 5, 7), /* U */
	FONT_ROW(5, 5, 5, 5, 2), /* V */
	FONT_ROW(5, 5, 7, 7, 5), /* W */
	FONT_ROW(5, 5, 2, 5, 5), /* X */
	FONT_ROW(5, 5, 2, 2, 2), /* Y */
	FONT_ROW(7, 1, 2, 4, 7), /* Z */
};

typedef enum
{
	GLYPH_LABEL = 0,
	GLYPH_ARROW_LEFT,
	GLYPH_ARROW_RIGHT,
	GLYPH_ARROW_UP,
	GLYPH_ARROW_DOWN,
} Glyph;

typedef struct
{
	const char   *name;
	SDL_Keycode   key;
	SDL_Scancode  scancode;
	Glyph         glyph;
	const char   *label;
} KeyName;

static const KeyName g_keyNames[] = {
	{"left",   SDLK_LEFT,     SDL_SCANCODE_LEFT,     GLYPH_ARROW_LEFT,  ""},
	{"right",  SDLK_RIGHT,    SDL_SCANCODE_RIGHT,    GLYPH_ARROW_RIGHT, ""},
	{"up",     SDLK_UP,       SDL_SCANCODE_UP,       GLYPH_ARROW_UP,    ""},
	{"down",   SDLK_DOWN,     SDL_SCANCODE_DOWN,     GLYPH_ARROW_DOWN,  ""},
	{"esc",    SDLK_ESCAPE,   SDL_SCANCODE_ESCAPE,   GLYPH_LABEL,       "ESC"},
	{"enter",  SDLK_RETURN,   SDL_SCANCODE_RETURN,   GLYPH_LABEL,       "OK"},
	{"space",  SDLK_SPACE,    SDL_SCANCODE_SPACE,    GLYPH_LABEL,       "SPC"},
	{"tab",    SDLK_TAB,      SDL_SCANCODE_TAB,      GLYPH_LABEL,       "TAB"},
	{"shift",  SDLK_LSHIFT,   SDL_SCANCODE_LSHIFT,   GLYPH_LABEL,       "SHF"},
	{"ctrl",   SDLK_LCTRL,    SDL_SCANCODE_LCTRL,    GLYPH_LABEL,       "CTL"},
	{"f1",     SDLK_F1,       SDL_SCANCODE_F1,       GLYPH_LABEL,       "F1"},
	{"f2",     SDLK_F2,       SDL_SCANCODE_F2,       GLYPH_LABEL,       "F2"},
	{"f3",     SDLK_F3,       SDL_SCANCODE_F3,       GLYPH_LABEL,       "F3"},
	{"f4",     SDLK_F4,       SDL_SCANCODE_F4,       GLYPH_LABEL,       "F4"},
	{"f5",     SDLK_F5,       SDL_SCANCODE_F5,       GLYPH_LABEL,       "F5"},
	{"f6",     SDLK_F6,       SDL_SCANCODE_F6,       GLYPH_LABEL,       "F6"},
	{"f7",     SDLK_F7,       SDL_SCANCODE_F7,       GLYPH_LABEL,       "F7"},
	{"f8",     SDLK_F8,       SDL_SCANCODE_F8,       GLYPH_LABEL,       "F8"},
	{"f9",     SDLK_F9,       SDL_SCANCODE_F9,       GLYPH_LABEL,       "F9"},
	{"f10",    SDLK_F10,      SDL_SCANCODE_F10,      GLYPH_LABEL,       "F10"},
	{"f11",    SDLK_F11,      SDL_SCANCODE_F11,      GLYPH_LABEL,       "F11"},
	{"f12",    SDLK_F12,      SDL_SCANCODE_F12,      GLYPH_LABEL,       "F12"},
};

typedef struct
{
	SDL_Keycode  key;
	SDL_Scancode scancode;
	Glyph        glyph;
	char         label[LABEL_CHARS + 1];
	float        cx, cy, r;   /* cx: fraction of width, cy and r: of height */
	SDL_FingerID finger;
	BOOL         held;
} TouchButton;

/* Default layout for a landscape screen: steering on the left thumb,
   accelerate/brake on the right one, Esc, camera ("c") and car reset in the
   corners. */
static const char g_defaultLayout[] =
	"left:9,76,10;right:27,76,10;down:73,78,8.5;up:91,76,11"
	";esc:5,11,5.5;c:87,11,4.5;f11:95,11,5.5";

#ifdef __ANDROID__
static BOOL s_enabled = true;
#else
static BOOL s_enabled = false;
#endif
static int32_t s_opacity = 40;
static int32_t s_scale   = 100;

static TouchButton s_buttons[MAX_BUTTONS];
static int32_t     s_nButtons = 0;
static BOOL        s_layoutParsed = false;

static int32_t s_winW = 640, s_winH = 480;

static float  s_vertices[MAX_VERTICES * FLOATS_PER_V];
static int32_t s_nVertices = 0;

/* ------------------------------------------------------------------ */
/*  Layout                                                             */
/* ------------------------------------------------------------------ */

static const KeyName *findKeyName(const char *name, size_t len)
{
	for (size_t i = 0; i < sizeof g_keyNames / sizeof g_keyNames[0]; ++i)
	{
		const char *candidate = g_keyNames[i].name;
		if (strlen(candidate) == len && !strncasecmp(candidate, name, len))
			return &g_keyNames[i];
	}
	return NULL;
}

static BOOL addButton(const char *name, size_t nameLen, float cx, float cy, float r)
{
	if (s_nButtons >= MAX_BUTTONS || r <= 0.0f)
		return false;

	TouchButton *b = &s_buttons[s_nButtons];
	memset(b, 0, sizeof *b);

	const KeyName *kn = findKeyName(name, nameLen);
	if (kn)
	{
		b->key      = kn->key;
		b->scancode = kn->scancode;
		b->glyph    = kn->glyph;
		snprintf(b->label, sizeof b->label, "%s", kn->label);
	}
	else if (nameLen == 1 && (isalnum((unsigned char)name[0])))
	{
		/* A single letter or digit is the key itself */
		char c = (char)tolower((unsigned char)name[0]);
		b->key      = (SDL_Keycode)c;
		b->scancode = SDL_GetScancodeFromKey(b->key, NULL);
		b->glyph    = GLYPH_LABEL;
		b->label[0] = (char)toupper((unsigned char)c);
		b->label[1] = '\0';
	}
	else
	{
		return false;
	}

	b->cx = cx / 100.0f;
	b->cy = cy / 100.0f;
	b->r  = r  / 100.0f;

	++s_nButtons;
	return true;
}

static BOOL parseLayout(const char *spec)
{
	s_nButtons = 0;

	while (*spec)
	{
		while (*spec == ' ' || *spec == ';')
			++spec;
		if (!*spec)
			break;

		const char *colon = strchr(spec, ':');
		if (!colon)
			return false;

		float cx = 0.0f, cy = 0.0f, r = 0.0f;
		if (sscanf(colon + 1, "%f,%f,%f", &cx, &cy, &r) != 3)
			return false;
		if (!addButton(spec, (size_t)(colon - spec), cx, cy, r))
			return false;

		const char *semicolon = strchr(colon, ';');
		if (!semicolon)
			break;
		spec = semicolon + 1;
	}

	return s_nButtons > 0;
}

static void ensureLayout(void)
{
	if (s_layoutParsed)
		return;
	s_layoutParsed = true;
	if (!parseLayout(g_defaultLayout))
		s_nButtons = 0;
}

void touchControlsSetEnabled(BOOL enabled)
{
	s_enabled = enabled;
}

void touchControlsSetOpacity(int32_t percent)
{
	if (percent < 0)
		percent = 0;
	else if (percent > 100)
		percent = 100;
	s_opacity = percent;
}

void touchControlsSetScale(int32_t percent)
{
	if (percent < 50)
		percent = 50;
	else if (percent > 300)
		percent = 300;
	s_scale = percent;
}

BOOL touchControlsSetLayout(const char *spec)
{
	s_layoutParsed = true;
	if (parseLayout(spec))
		return true;
	/* Bad configuration - fall back to the built-in layout */
	parseLayout(g_defaultLayout);
	return false;
}

BOOL touchControlsAreEnabled(void)
{
	return s_enabled;
}

/* ------------------------------------------------------------------ */
/*  Input                                                              */
/* ------------------------------------------------------------------ */

static void pushKeyEvent(const TouchButton *b, BOOL down)
{
	SDL_Event event;
	memset(&event, 0, sizeof event);
	event.type          = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
	event.key.timestamp = SDL_GetTicksNS();
	event.key.key       = b->key;
	event.key.scancode  = b->scancode;
	event.key.down      = down;
	SDL_PushEvent(&event);
}

static void releaseButton(TouchButton *b)
{
	if (!b->held)
		return;
	b->held   = false;
	b->finger = 0;
	pushKeyEvent(b, false);
}

static void pressButton(TouchButton *b, SDL_FingerID finger)
{
	if (b->held)
		return;
	b->held   = true;
	b->finger = finger;
	pushKeyEvent(b, true);
}

static float buttonRadiusPixels(const TouchButton *b)
{
	return b->r * (s_scale / 100.0f) * (float)s_winH;
}

static TouchButton *buttonAt(float nx, float ny)
{
	for (int32_t i = 0; i < s_nButtons; ++i)
	{
		TouchButton *b = &s_buttons[i];
		const float dx = (nx - b->cx) * (float)s_winW;
		const float dy = (ny - b->cy) * (float)s_winH;
		/* A bit of slack around the visible circle, fingers are not precise */
		const float r  = buttonRadiusPixels(b) * 1.15f;
		if (dx * dx + dy * dy <= r * r)
			return b;
	}
	return NULL;
}

static TouchButton *buttonHeldBy(SDL_FingerID finger)
{
	for (int32_t i = 0; i < s_nButtons; ++i)
	{
		if (s_buttons[i].held && s_buttons[i].finger == finger)
			return &s_buttons[i];
	}
	return NULL;
}

BOOL touchControlsFingerDown(SDL_FingerID finger, float nx, float ny)
{
	if (!s_enabled)
		return false;
	ensureLayout();

	TouchButton *b = buttonAt(nx, ny);
	if (!b || b->held)
		return b != NULL;

	pressButton(b, finger);
	return true;
}

BOOL touchControlsFingerMotion(SDL_FingerID finger, float nx, float ny)
{
	if (!s_enabled)
		return false;
	ensureLayout();

	TouchButton *held = buttonHeldBy(finger);
	TouchButton *over = buttonAt(nx, ny);

	if (held == over)
		return held != NULL;

	/* The finger slid off its button (or onto another one) */
	if (held)
		releaseButton(held);
	if (over && !over->held)
	{
		pressButton(over, finger);
		return true;
	}
	return over != NULL || held != NULL;
}

BOOL touchControlsFingerUp(SDL_FingerID finger)
{
	if (!s_enabled)
		return false;

	TouchButton *held = buttonHeldBy(finger);
	if (!held)
		return false;

	releaseButton(held);
	return true;
}

void touchControlsReleaseAll(void)
{
	for (int32_t i = 0; i < s_nButtons; ++i)
		releaseButton(&s_buttons[i]);
}

/* ------------------------------------------------------------------ */
/*  Geometry                                                           */
/* ------------------------------------------------------------------ */

typedef struct
{
	float r, g, b, a;
} Color;

static void emitVertex(float x, float y, Color c)
{
	if (s_nVertices >= MAX_VERTICES)
		return;
	float *v = &s_vertices[s_nVertices * FLOATS_PER_V];
	v[0] = x;
	v[1] = y;
	v[2] = c.r;
	v[3] = c.g;
	v[4] = c.b;
	v[5] = c.a;
	++s_nVertices;
}

static void emitTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color c)
{
	emitVertex(x0, y0, c);
	emitVertex(x1, y1, c);
	emitVertex(x2, y2, c);
}

static void emitQuad(float x, float y, float w, float h, Color c)
{
	emitTriangle(x, y, x + w, y, x + w, y + h, c);
	emitTriangle(x, y, x + w, y + h, x, y + h, c);
}

/* rx/ry are the radii in normalized window space (they differ because the
   window is not square and the buttons have to stay round). */
static void emitDisc(float cx, float cy, float rx, float ry, Color c)
{
	float px = cx + rx, py = cy;
	for (int32_t i = 1; i <= DISC_SEGMENTS; ++i)
	{
		const float a = (float)(2.0 * M_PI * i / DISC_SEGMENTS);
		const float x = cx + rx * cosf(a);
		const float y = cy + ry * sinf(a);
		emitTriangle(cx, cy, px, py, x, y, c);
		px = x;
		py = y;
	}
}

static void emitRing(float cx, float cy, float rx, float ry, float thickness, Color c)
{
	const float irx = rx * (1.0f - thickness), iry = ry * (1.0f - thickness);
	float pox = cx + rx, poy = cy, pix = cx + irx, piy = cy;
	for (int32_t i = 1; i <= DISC_SEGMENTS; ++i)
	{
		const float a = (float)(2.0 * M_PI * i / DISC_SEGMENTS);
		const float ca = cosf(a), sa = sinf(a);
		const float ox = cx + rx  * ca, oy = cy + ry  * sa;
		const float ix = cx + irx * ca, iy = cy + iry * sa;
		emitTriangle(pox, poy, ox, oy, ix, iy, c);
		emitTriangle(pox, poy, ix, iy, pix, piy, c);
		pox = ox; poy = oy;
		pix = ix; piy = iy;
	}
}

static void emitArrow(float cx, float cy, float rx, float ry, Glyph glyph, Color c)
{
	const float sx = rx * 0.55f, sy = ry * 0.55f;
	switch (glyph)
	{
		case GLYPH_ARROW_LEFT:
			emitTriangle(cx - sx, cy, cx + sx * 0.7f, cy - sy, cx + sx * 0.7f, cy + sy, c);
			break;
		case GLYPH_ARROW_RIGHT:
			emitTriangle(cx + sx, cy, cx - sx * 0.7f, cy - sy, cx - sx * 0.7f, cy + sy, c);
			break;
		case GLYPH_ARROW_UP:
			emitTriangle(cx, cy - sy, cx - sx, cy + sy * 0.7f, cx + sx, cy + sy * 0.7f, c);
			break;
		case GLYPH_ARROW_DOWN:
			emitTriangle(cx, cy + sy, cx - sx, cy - sy * 0.7f, cx + sx, cy - sy * 0.7f, c);
			break;
		default:
			break;
	}
}

static void emitLabel(float cx, float cy, float rx, float ry, const char *label, Color c)
{
	const int32_t len = (int32_t)strlen(label);
	if (len <= 0)
		return;

	/* Fit the whole label inside the button: the glyph box is limited both by
	   the available width (len * 3 columns plus a gap between characters) and
	   by the five rows of the font, otherwise a single character would be
	   drawn far larger than the button. */
	const float box = 1.3f;
	float cellH = (box * ry) / 5.0f;
	const float widthLimit = (box * ry) / (float)(len * 4 - 1);
	if (widthLimit < cellH)
		cellH = widthLimit;
	const float cellW = cellH * (rx / ry);
	const float totalW = cellW * (float)(len * 4 - 1);
	const float totalH = cellH * 5.0f;
	const float x0 = cx - totalW / 2.0f;
	const float y0 = cy - totalH / 2.0f;

	for (int32_t i = 0; i < len; ++i)
	{
		const char ch = (char)toupper((unsigned char)label[i]);
		int32_t index = -1;
		if (ch >= '0' && ch <= '9')
			index = ch - '0';
		else if (ch >= 'A' && ch <= 'Z')
			index = 10 + ch - 'A';
		if (index < 0)
			continue;

		const uint16_t bits = g_font3x5[index];
		for (int32_t row = 0; row < 5; ++row)
		{
			const int32_t rowBits = (bits >> (12 - row * 3)) & 0x7;
			for (int32_t col = 0; col < 3; ++col)
			{
				if (!(rowBits & (1 << (2 - col))))
					continue;
				emitQuad(x0 + (float)(i * 4 + col) * cellW,
				         y0 + (float)row * cellH,
				         cellW, cellH, c);
			}
		}
	}
}

const float *touchControlsGeometry(int32_t winW, int32_t winH, int32_t *vertexCount)
{
	*vertexCount = 0;

	if (!s_enabled || winW <= 0 || winH <= 0)
		return NULL;

	s_winW = winW;
	s_winH = winH;
	ensureLayout();

	if (s_nButtons <= 0 || s_opacity <= 0)
		return NULL;

	const float alpha = s_opacity / 100.0f;
	const float aspect = (float)winH / (float)winW;

	s_nVertices = 0;
	for (int32_t i = 0; i < s_nButtons; ++i)
	{
		const TouchButton *b = &s_buttons[i];
		const float ry = b->r * (s_scale / 100.0f);
		const float rx = ry * aspect;

		Color fill  = {1.0f, 1.0f, 1.0f, alpha * (b->held ? 0.55f : 0.18f)};
		Color edge  = {1.0f, 1.0f, 1.0f, alpha * (b->held ? 1.00f : 0.65f)};
		Color glyph = {1.0f, 1.0f, 1.0f, alpha * (b->held ? 1.00f : 0.80f)};
		if (b->held)
		{
			/* Warm tint while pressed */
			fill.b  = 0.45f;
			edge.b  = 0.45f;
			glyph.b = 0.45f;
		}

		emitDisc(b->cx, b->cy, rx, ry, fill);
		emitRing(b->cx, b->cy, rx, ry, 0.14f, edge);
		if (b->glyph == GLYPH_LABEL)
			emitLabel(b->cx, b->cy, rx, ry, b->label, glyph);
		else
			emitArrow(b->cx, b->cy, rx, ry, b->glyph, glyph);
	}

	*vertexCount = s_nVertices;
	return s_nVertices > 0 ? s_vertices : NULL;
}
