/*
 * Test Card - A test pattern generator for computer displays
 *
 * Copyright (C) 2009-2015 Väinö Helminen <vah78@yahoo.com>
 *
 * The main purpose of this program is to see you get an unscaled
 * perfect image on your computer display (e.g. a TV) and check how
 * much, if any, overscan there is. You will also get see if the
 * colors at least in the right ball park and if the pixels are square
 * or squeezed in some way. As a bonus you can try to estimate your
 * display's gamma value.
 *
 * The provided Makefile should just work on your ordinary Linux
 * desktop when you run "make" but because this is a just a single
 * source file with SDL2 and SDL_ttf as dependencies I guess you can
 * figure out how to compile if it doesn't.
 *
 * This program is licensed under the GPL2 and the full license text
 * should have been included with the source code (see file COPYING).
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>

static inline SDL_Surface* setVideoMode(int fullscreen, int width, int height, int d)
{
  static int mode = 0;
  mode -= d;
  Uint32 flags = SDL_SWSURFACE;
  if(fullscreen) flags |= SDL_FULLSCREEN;
  if(width < 0) {
    SDL_Rect **modes = SDL_ListModes(NULL, flags|(SDL_FULLSCREEN|SDL_ANYFORMAT));
    if(!modes || modes == (SDL_Rect**)-1) {
      fprintf(stderr, "No suitable video mode\n");
      exit(EXIT_FAILURE);
    }
    int count = 0;
    for(; modes[count]; ++count) ;
    if(mode < 0) mode += count;
    mode %= count;
    width = modes[mode]->w;
    height = modes[mode]->h;
  }

  return SDL_SetVideoMode(width, height, 32, flags);
}

static inline void fillRect(SDL_Surface *surface, int x, int y, int w, int h, Uint32 color)
{
  SDL_Rect r = {x, y, w, h};
  SDL_FillRect(surface, &r, color);
}

static void rasterRect(SDL_Surface *surface, int x, int y, int w, int h, Uint32 color1, Uint32 color2)
{
  for(int j = y; j < y+h; ++j) {
    for(int i = x; i < x+w; ++i) {
      fillRect(surface, i, j, 1, 1, (i^j)&1 ? color1 : color2);
    }
  }
}

static void hLineRect(SDL_Surface *surface, int l, int x, int y, int w, int h, Uint32 color1, Uint32 color2)
{
  int c = 0;
  h -= l;
  for(int i = y; i <= y + h; i += l) {
    fillRect(surface, x, i, w, l, (c=!c) ? color1 : color2);
  }
}

static void vLineRect(SDL_Surface *surface, int l, int x, int y, int w, int h, Uint32 color1, Uint32 color2)
{
  int c = 0;
  w -= l;
  for(int i = x; i <= x + w; i += l) {
    fillRect(surface, i, y, l, h, (c=!c) ? color1 : color2);
  }
}

static void colorFade(SDL_Surface *surface, int x, int y, int w, int h, int startr, int startg, int startb, int endr, int endg, int endb)
{
  if(w > h) {
    for(int i = 0; i < w; ++i) {
      int r = startr + (i * (endr-startr))/(w-1);
      int g = startg + (i * (endg-startg))/(w-1);
      int b = startb + (i * (endb-startb))/(w-1);
      fillRect(surface, x+i, y, 1, h, SDL_MapRGB(surface->format, r, g, b));
    }
  } else {
    for(int i = 0; i < h; ++i) {
      int r = startr + (i * (endr-startr))/(h-1);
      int g = startg + (i * (endg-startg))/(h-1);
      int b = startb + (i * (endb-startb))/(h-1);
      fillRect(surface, x, y+i, w, 1, SDL_MapRGB(surface->format, r, g, b));
    }
  }
}

static inline void borders(SDL_Surface *surface, int s)
{
  int w = surface->w, h = surface->h;
  Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0), white = SDL_MapRGB(surface->format, 255, 255, 255);

  // top, bottom, left and right rasterbars
  rasterRect(surface, 3*s+1, 0, w-6*s-2, 2*s, white, black);
  rasterRect(surface, 3*s+1, h-2*s, w-6*s-2, 2*s, white, black);
  rasterRect(surface, 0, 3*s+1, 2*s, h-6*s-2, white, black);
  rasterRect(surface, w-2*s, 3*s+1, 2*s, h-6*s-2, white, black);

  // top-left corner
  fillRect(surface, 0, 0, 3*s+1, 3*s+1, black);
  fillRect(surface,   1,   0, 3*s-1,     1, white);
  fillRect(surface, 1*s,   1,   2*s,     1, white);
  fillRect(surface, 2*s,   2,   1*s,     1, white);
  fillRect(surface,   0,   1,     1, 3*s-1, white);
  fillRect(surface,   1, 1*s,     1,   2*s, white);
  fillRect(surface,   2, 2*s,     1,   1*s, white);

  // bottom-left corner
  fillRect(surface, 0, h-3*s-1, 3*s+1, 3*s+1, black);
  fillRect(surface,   1,   h-1, 3*s-1,     1, white);
  fillRect(surface, 1*s,   h-2,   2*s,     1, white);
  fillRect(surface, 2*s,   h-3,   1*s,     1, white);
  fillRect(surface,   0, h-3*s,     1, 3*s-1, white);
  fillRect(surface,   1, h-3*s,     1,   2*s, white);
  fillRect(surface,   2, h-3*s,     1,   1*s, white);

  // top-right corner
  fillRect(surface, w-3*s-1, 0, 3*s+1, 3*s+1, black);
  fillRect(surface, w-3*s,   0, 3*s-1,     1, white);
  fillRect(surface, w-3*s,   1,   2*s,     1, white);
  fillRect(surface, w-3*s,   2,   1*s,     1, white);
  fillRect(surface,   w-1,   1,     1, 3*s-1, white);
  fillRect(surface,   w-2, 1*s,     1,   2*s, white);
  fillRect(surface,   w-3, 2*s,     1,   1*s, white);

  // bottom-right corner
  fillRect(surface, w-3*s-1, h-3*s-1, 3*s+1, 3*s+1, black);
  fillRect(surface, w-3*s,   h-1, 3*s-1,     1, white);
  fillRect(surface, w-3*s,   h-2,   2*s,     1, white);
  fillRect(surface, w-3*s,   h-3,   1*s,     1, white);
  fillRect(surface,   w-1, h-3*s,     1, 3*s-1, white);
  fillRect(surface,   w-2, h-3*s,     1,   2*s, white);
  fillRect(surface,   w-3, h-3*s,     1,   1*s, white);
}

static inline void colorBars(SDL_Surface *surface, int x, int y, int w, int h)
{
  int s = (h-4)/4;
  y += 2;
  colorFade(surface, x,     y, w, s, 255,0,0, 0,0,0);
  colorFade(surface, x,   y+s, w, s, 0,255,0, 0,0,0);
  colorFade(surface, x, y+2*s, w, s, 0,0,255, 0,0,0);
  colorFade(surface, x, y+3*s, w, s, 255,255,255, 0,0,0);
}

static inline void gammaTable(SDL_Surface *surface, int x, int y, int w, int h)
{
  int s = (2*h)/9;
  {
    int sx = w/54;
    if(sx < s) s = sx;
  }
  x += (w - 54*s)/2;
  y += (h - (9*s)/2)/2;

  TTF_Font *font = TTF_OpenFont("Vera.ttf", (3*s)/2);
  if(!font) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }

  Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0), white = SDL_MapRGB(surface->format, 255, 255, 255);
  SDL_Color whiteColor = {255, 255, 255, 0};

  hLineRect(surface, 1, x,     y, 2*s, s, white, black);
  rasterRect(surface,   x,   y+s, 2*s, s, white, black);
  vLineRect(surface, 1, x, y+2*s, 2*s, s, white, black);

  for(int i = 0; i < 13; ++i) {
    double gamma = 1. + i/10.;
    int shade = 255. * pow(0.5, 1./gamma);
    Uint32 gray = SDL_MapRGB(surface->format, shade, shade, shade);
    x += 2*s;
    fillRect(surface,  x, y, 2*s, 3*s, gray);

    char buf[10];
    sprintf(buf, "%1.1f", gamma);
    SDL_Surface *text = TTF_RenderText_Blended(font, buf, whiteColor);
    if(!text) {
      fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
    } else {
      SDL_Rect blitrect = {x + (2*s-text->w)/2, y+3*s, 0, 0};
      SDL_BlitSurface(text, NULL, surface, &blitrect);
      SDL_FreeSurface(text);
    }

    x += 2*s;
    hLineRect(surface, 1, x,     y, 2*s, s, white, black);
    rasterRect(surface,   x,   y+s, 2*s, s, white, black);
    vLineRect(surface, 1, x, y+2*s, 2*s, s, white, black);
  }
  TTF_CloseFont(font);
}

static inline void resolution(SDL_Surface *surface, int x, int y, int w, int h)
{
  TTF_Font *font = TTF_OpenFont("Vera.ttf", h/2);
  if(!font) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }
  char buf[16];
  sprintf(buf, "%d×%d", (int)surface->w, (int)surface->h);
  SDL_Color blackColor = {0,0,0,0}, whiteColor = {255, 255, 255, 0};
  SDL_Surface *text = TTF_RenderUTF8_Shaded(font, buf, whiteColor, blackColor);
  if(!text) fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
  TTF_CloseFont(font);
  if(!text) return;

  SDL_Rect blitrect = { x + (w - text->w)/2, y + (h - text->h)/2, 0, 0 };

  Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0), white = SDL_MapRGB(surface->format, 255, 255, 255);
  fillRect(surface, blitrect.x-h/4, y, text->w+h/2, h, white);
  fillRect(surface, blitrect.x-h/8, y+h/8, text->w+h/4, h-h/4, black);

  SDL_BlitSurface(text, NULL, surface, &blitrect);
  SDL_FreeSurface(text);
}

static inline void linesBars(SDL_Surface *surface, int x, int y, int w, int h)
{
  Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0), white = SDL_MapRGB(surface->format, 255, 255, 255);
  int s = w/8;
  x += (w - 8*s)/2;
  for(int l = 1; l <= 4; ++l) {
    vLineRect(surface, l, x, y, s, h, white, black);
    x += s;
  }
  for(int l = 4; l >= 1; --l) {
    hLineRect(surface, l, x, y, s, h, white, black);
    x += s;
  }
}

static inline void colorBoxes(SDL_Surface *surface, int x, int y, int w, int h)
{
  Uint32 colors[8][3] = {
    {250, 250, 250}, // white
    {255, 255, 0}, // yellow
    {0,255, 255}, // cyan
    {0, 255, 0}, // green
    {255, 0, 255}, // magenta
    {255, 0, 0}, // red
    {0, 0, 255}, // blue
    {0, 0, 0}, // black
  };

  int s = w / 8;
  x += (w - 8*s)/2;
  for(int i = 0; i < 8; ++i) {
    Uint32 color = SDL_MapRGB(surface->format, colors[i][0], colors[i][1], colors[i][2]);
    fillRect(surface, x, y, s, h, color);
    x += s;
  }
}

static inline void circlePoints(SDL_Surface *surface, int cx, int cy, int x, int y, int size, Uint32 color)
{
  if(x == 0) {
    fillRect(surface, cx, cy + y, size, size, color);
    fillRect(surface, cx, cy - y, size, size, color);
    fillRect(surface, cx + y, cy, size, size, color);
    fillRect(surface, cx - y, cy, size, size, color);
  } else {
    fillRect(surface, cx + x, cy + y, size, size, color);
    fillRect(surface, cx - x, cy + y, size, size, color);
    fillRect(surface, cx + x, cy - y, size, size, color);
    fillRect(surface, cx - x, cy - y, size, size, color);
    if(x < y) {
      fillRect(surface, cx + y, cy + x, size, size, color);
      fillRect(surface, cx - y, cy + x, size, size, color);
      fillRect(surface, cx + y, cy - x, size, size, color);
      fillRect(surface, cx - y, cy - x, size, size, color);
    }
  }
}

static void drawCircle(SDL_Surface *surface, int cx, int cy, int radius, int size, Uint32 color)
{
  int x = 0, y = radius, p = (5-radius*4)/4;
  circlePoints(surface, cx, cy, x, y, size, color);
  while(x < y) {
    ++x;
    if(p < 0) {
      p += 2*x+1;
    } else {
      --y;
      p += 2*(x-y)+1;
    }
    circlePoints(surface, cx, cy, x, y, size, color);
  }
}

static inline void copyright(SDL_Surface *surface)
{
  TTF_Font *font = TTF_OpenFont("Vera.ttf", 8);
  if(!font) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }
  SDL_Color grayColor = {180,180,180,0}, blueColor = {0,0,255,0};
  SDL_Surface *text = TTF_RenderUTF8_Shaded(font, "Copyright © 2009-2015 Väinö Helminen", blueColor, grayColor);
  if(!text) fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
  TTF_CloseFont(font);
  if(!text) return;
  SDL_Rect blitrect = { (surface->w - text->w)/2, surface->h - text->h, 0, 0};
  SDL_BlitSurface(text, NULL, surface, &blitrect);
  SDL_FreeSurface(text);
}

static inline void bigCircle(SDL_Surface *surface)
{
  int radius;
  if(surface->h < surface->w) radius = surface->h;
  else radius = surface->w;
  radius = (2*radius)/5;
  int cx = surface->w/2-1, cy = surface->h/2-1;
  Uint32 black = SDL_MapRGB(surface->format, 0,0,0),
    gray = SDL_MapRGB(surface->format, 180,180,180),
    white = SDL_MapRGB(surface->format, 255,255,255);
  drawCircle(surface, cx+1, cy+1, radius, 3, black);
  drawCircle(surface, cx-1, cy-1, radius, 3, white);
  drawCircle(surface, cx, cy, radius, 3, gray);
}

static inline void overscan(SDL_Surface *surface)
{
  int w = surface->w, h = surface->h;
  int w5 = (w+10)/20, w10 = (w+5)/10, h5 = (h+10)/20, h10 = (h+5)/10;
  Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0),
    green = SDL_MapRGB(surface->format, 0, 255, 0), yellow = SDL_MapRGB(surface->format, 255, 255, 0);
  SDL_Color greenColor = {0,255,0,0}, yellowColor = {255,255,0,0};

  // top-left 5%
  fillRect(surface, w5-1, h5-1, w5+1, 3, black);
  fillRect(surface, w5-1, h5-1, 3, h5+1, black);
  fillRect(surface, w5, h5, w5, 1, green);
  fillRect(surface, w5, h5, 1, h5, green);
  // top-left 10%
  fillRect(surface, w10-1, h10-1, w5+1, 3, black);
  fillRect(surface, w10-1, h10-1, 3, h5+1, black);
  fillRect(surface, w10, h10, w5, 1, yellow);
  fillRect(surface, w10, h10, 1, h5, yellow);

  // bottom-left 5%
  fillRect(surface, w5-1, h-h5-2, w5+1, 3, black);
  fillRect(surface, w5-1, h-2*h5, 3, h5+1, black);
  fillRect(surface, w5, h-h5-1, w5, 1, green);
  fillRect(surface, w5, h-2*h5, 1, h5, green);
  // bottom-left 10%
  fillRect(surface, w10-1, h-h10-2, w5+1, 3, black);
  fillRect(surface, w10-1, h-h10-h5, 3, h5+1, black);
  fillRect(surface, w10, h-h10-1, w5, 1, yellow);
  fillRect(surface, w10, h-h10-h5, 1, h5, yellow);

  // top-right 5%
  fillRect(surface, w-2*w5, h5-1, w5+1, 3, black);
  fillRect(surface, w-w5-2, h5-1, 3, h5+1, black);
  fillRect(surface, w-2*w5, h5, w5, 1, green);
  fillRect(surface, w-w5-1, h5, 1, h5, green);
  // top-right 10%
  fillRect(surface, w-w10-w5, h10-1, w5+1, 3, black);
  fillRect(surface, w-w10-2, h10-1, 3, h5+1, black);
  fillRect(surface, w-w10-w5, h10, w5, 1, yellow);
  fillRect(surface, w-w10-1, h10, 1, h5, yellow);

  // bottom-right 5%
  fillRect(surface, w-2*w5, h-h5-2, w5+1, 3, black);
  fillRect(surface, w-w5-2, h-2*h5, 3, h5+1, black);
  fillRect(surface, w-2*w5, h-h5-1, w5, 1, green);
  fillRect(surface, w-w5-1, h-2*h5, 1, h5, green);
  // bottom-right 10%
  fillRect(surface, w-w10-w5, h-h10-2, w5+1, 3, black);
  fillRect(surface, w-w10-2, h-h10-h5, 3, h5+1, black);
  fillRect(surface, w-w10-w5, h-h10-1, w5, 1, yellow);
  fillRect(surface, w-w10-1, h-h10-h5, 1, h5, yellow);


  TTF_Font *font = TTF_OpenFont("Vera.ttf", w5/2);
  if(!font) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }
  SDL_Surface *text = TTF_RenderText_Blended(font, "5%", greenColor);
  if(!text) {
    fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
  } else {
    SDL_Rect blitrect = {w-w5-2-text->w, h5+1, 0, 0};
    SDL_BlitSurface(text, NULL, surface, &blitrect);
  }
  text = TTF_RenderText_Blended(font, "10%", yellowColor);
  if(!text) {
    fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
  } else {
    SDL_Rect blitrect = {w-w10-2-text->w, h10+1, 0, 0};
    SDL_BlitSurface(text, NULL, surface, &blitrect);
  }
  TTF_CloseFont(font);
}


static inline void render(SDL_Surface *surface)
{
  SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 32, 32, 32));
  int w = surface->w, h = surface->h;
  int x = w/60;
  borders(surface, x);
  x *= 3; w -= 2*x;
  int sy = (h-2*x)/5;
  colorBoxes(surface, x, x     , w, sy);
  resolution(surface, x, x+2*sy, w, sy);
  linesBars (surface, x, x+4*sy, w, sy);
  bigCircle(surface);
  gammaTable(surface, x, x  +sy, w, sy);
  colorBars (surface, x, x+3*sy, w, sy);
  copyright(surface);
  overscan(surface);
}


int main(int argc, char **argv)
{
  fprintf(stdout, "Test Card - Copyright (C) 2009-2015 Vaino Helminen\n");

  if(SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  atexit(SDL_Quit);

  if(TTF_Init()) {
    fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
    return EXIT_FAILURE;
  }
  atexit(TTF_Quit);


  int fullscreen = 1, width = -1, height = -1, savebmp=0, quit = 0;
  for(int i = 1; i < argc; ++i) {
    if(argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 's':
	savebmp = 1;
	continue;
      case 'w':
	fullscreen = 0;
	continue;
      case 'q':
	quit = 1;
	continue;
      default:
	break;
      }
    } else if(width < 0 && sscanf(argv[i], "%dx%d", &width, &height) == 2 && width > 0 && height > 0) {
      continue;
    }
    fprintf(stderr, "\nInvalid argument: %s\n\nUsage: %s [-q] [-s] [-w] [<width>x<height>]\n\t-q\tQuit immediately (use with -s)\n\t-s\tSave image as <width>x<height>.bmp\n\t-w\tRun in window instead of fullscreen\n\t<width>x<height> Use the given resolution instead of the highest available\n\nKeys:\tUp / +\tSwitch to a higher resolution (loops to lowest)\n\tDown / -\tSwitch to a lower resolution (loops to highest)\n\ts\tSave a screenshot\n\tEsc / q\tQuit\n", argv[i], argv[0]);
    return EXIT_FAILURE;
  }

  SDL_Surface *screen = setVideoMode(fullscreen, width, height, 0);
  if(!screen) {
    fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  if(fullscreen) SDL_ShowCursor(0);
  SDL_WM_SetCaption("Test Card", 0);

  do {
    render(screen);
    SDL_Flip(screen);

    if(savebmp) {
      char buf[80];
      sprintf(buf, "%dx%d.bmp", (int)screen->w, (int)screen->h);
      if(SDL_SaveBMP(screen, buf)) {
	fprintf(stderr, "SDL_SaveBMP: %s\n", SDL_GetError());
      } else {
        fprintf(stdout, "Saved a screenshot to %s\n", buf);
      }
      savebmp = 0;
    }
    if(quit) return EXIT_SUCCESS;

    SDL_WaitEvent(NULL);
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT:
	quit = 1;
        break;

      case SDL_KEYDOWN:
	switch(event.key.keysym.sym) {
	case SDLK_ESCAPE:
	case SDLK_q:
	  quit = 1;
	  break;

	case SDLK_UP:
	case SDLK_PLUS:
	case SDLK_KP_PLUS:
	  screen = setVideoMode(fullscreen, -1, -1, 1);
	  break;

	case SDLK_DOWN:
	case SDLK_MINUS:
	case SDLK_KP_MINUS:
	  screen = setVideoMode(fullscreen, -1, -1, -1);
	  break;

	case SDLK_s:
	  savebmp = 1;
	  break;

	default:
	  break;
	}
        break;

      default:
	break;
      }
    }
  } while(!quit);
  return EXIT_SUCCESS;
}
