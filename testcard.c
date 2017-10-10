/*
 * Test Card - A test pattern generator for computer displays
 *
 * Copyright (C) 2009-2016 Väinö Helminen
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
 * source file with SDL 1.2 and SDL_ttf as dependencies I guess you
 * can figure out how to compile if it doesn't.
 *
 * This program is licensed under the GPL2 and the full license text
 * should have been included with the source code (see file COPYING).
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>

static char *fontName;

static inline int maxi(int a, int b)
{
  return a < b ? b : a;
}

static inline int mini(int a, int b)
{
  return a < b ? a : b;
}

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

  return SDL_SetVideoMode(width, height, 0, flags);
}

static inline void fillRect(SDL_Surface *surface, int x, int y, int w, int h, Uint32 color)
{
  SDL_Rect r = {x, y, w, h};
  SDL_FillRect(surface, &r, color);
}

static void rasterRect(SDL_Surface *surface, int x, int y, int w, int h, Uint32 color1, Uint32 color2)
{
  fillRect(surface, x, y, w, h, color1);
  w += x;
  h += y;
  for (int j = y; j < h; ++j) {
    for (int i = x + (j & 1); i < w; i += 2) {
      fillRect(surface, i, j, 1, 1, color2);
    }
  }
}

static void hLineRect(SDL_Surface *surface, int l, int x, int y, int w, int h, Uint32 color1, Uint32 color2)
{
  fillRect(surface, x, y, w, l*(h/l), color1);
  y += l;
  h += y - 2*l;
  for(; y <= h; y += 2*l) {
    fillRect(surface, x, y, w, l, color2);
  }
}

static void vLineRect(SDL_Surface *surface, int l, int x, int y, int w, int h, Uint32 color1, Uint32 color2)
{
  fillRect(surface, x, y, l*(w/l), h, color1);
  x += l;
  w += x - 2*l;
  for(; x <= w; x += 2*l) {
    fillRect(surface, x, y, l, h, color2);
  }
}

static void gradientRGB(SDL_Surface *surface, int x, int y, int w, int h, int startr, int startg, int startb, int endr, int endg, int endb)
{
  if(w > h) {
    int s = maxi(1, w / 256);
    for(int i = 0; i < w-s; i += s) {
      int r = startr + (i * (endr-startr))/(w-1);
      int g = startg + (i * (endg-startg))/(w-1);
      int b = startb + (i * (endb-startb))/(w-1);
      fillRect(surface, x+i, y, s, h, SDL_MapRGB(surface->format, r, g, b));
    }
    fillRect(surface, x+w-s, y, s, h, SDL_MapRGB(surface->format, endr, endg, endb));
  } else {
    int s = maxi(1, h / 256);
    for(int i = 0; i < h-s; i += s) {
      int r = startr + (i * (endr-startr))/(h-1);
      int g = startg + (i * (endg-startg))/(h-1);
      int b = startb + (i * (endb-startb))/(h-1);
      fillRect(surface, x, y+i, w, s, SDL_MapRGB(surface->format, r, g, b));
    }
    fillRect(surface, x, y+h-s, w, s, SDL_MapRGB(surface->format, endr, endg, endb));
  }
}

static inline void borders(SDL_Surface *surface, int size)
{
  Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0);
  Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);
  int w = surface->w;
  int h = surface->h;

  // top, bottom, left and right rasterbars
  int border = 2*(size/3);
  rasterRect(surface, size, 0, w-2*size, border, white, black);
  rasterRect(surface, size, h-border, w-2*size, border, white, black);
  rasterRect(surface, 0, size, border, h-2*size, white, black);
  rasterRect(surface, w-border, size, border, h-2*size, white, black);

  // top-left corner
  fillRect(surface, 0, 0, size-1, size-1, white);
  fillRect(surface, 0, 0, 1, 1, black);
  fillRect(surface, 1, 1, size/3-1, size/3-1, black);
  fillRect(surface, 2, 2, 2*(size/3)-2, 2*(size/3)-2, black);
  fillRect(surface, 3, 3, size-4, size-4, black);

  // top-right corner
  fillRect(surface, w-size+1, 0, size-1, size-1, white);
  fillRect(surface, w-1, 0, 1, 1, black);
  fillRect(surface, w-size/3, 1, size/3-1, size/3-1, black);
  fillRect(surface, w-2*(size/3), 2, 2*(size/3)-2, 2*(size/3)-2, black);
  fillRect(surface, w-size+1, 3, size-4, size-4, black);

  // bottom-left corner
  fillRect(surface, 0, h-size+1, size-1, size-1, white);
  fillRect(surface, 0, h-1, 1, 1, black);
  fillRect(surface, 1, h-size/3, size/3-1, size/3-1, black);
  fillRect(surface, 2, h-2*(size/3), 2*(size/3)-2, 2*(size/3)-2, black);
  fillRect(surface, 3, h-size+1, size-4, size-4, black);

  // bottom-right corner
  fillRect(surface, w-size+1, h-size+1, size-1, size-1, white);
  fillRect(surface, w-1, h-1, 1, 1, black);
  fillRect(surface, w-size/3, h-size/3, size/3-1, size/3-1, black);
  fillRect(surface, w-2*(size/3), h-2*(size/3), 2*(size/3)-2, 2*(size/3)-2, black);
  fillRect(surface, w-size+1, h-size+1, size-4, size-4, black);
}

static inline void RGBGradients(SDL_Surface *surface, int x, int y, int w, int h)
{
  int s = h/4;
  gradientRGB(surface, x, y, w, s, 255,0,0, 0,0,0);
  y += s;
  gradientRGB(surface, x, y, w, s, 0,255,0, 0,0,0);
  y += s;
  gradientRGB(surface, x, y, w, s, 0,0,255, 0,0,0);
  y += s;
  gradientRGB(surface, x, y, w, h-3*s, 255,255,255, 0,0,0);
}

static inline void gammaTable(SDL_Surface *surface, int x, int y, int w, int h)
{
  Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0);
  Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);
  SDL_Color blackColor = {0,0,0,0};
  SDL_Color grayColor = {200,200,200,0};

  w = surface->w;

  int wb = w / (2*17+1);
  x = (w - wb * (2*17+1))/2;
  if ((x&1)) x -= 1;

  TTF_Font *font = TTF_OpenFont(fontName, maxi(h/5, 8));
  if (font) {
    h -= TTF_FontLineSkip(font);
  } else {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
  }

  for (int i = 0; ; ++i) {
    rasterRect(surface, x, y, wb, h, white, black);

    if (i > 16) break;

    x += wb;

    double gamma = 1. + i/10.;
    int shade = 255. * pow(0.5, 1./gamma);
    Uint32 gray = SDL_MapRGB(surface->format, shade, shade, shade);

    fillRect(surface, x, y, wb, h, gray);

    if (font) {
      char buf[10];
      sprintf(buf, "%1.1f", gamma);
      {
        TTF_SetFontOutline(font, 1);
        SDL_Surface *text = TTF_RenderText_Blended(font, buf, blackColor);
        if(!text) {
          fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
        } else {
          SDL_Rect blitrect = {x + (wb - text->w)/2, y+h-1, 0, 0};
          SDL_BlitSurface(text, NULL, surface, &blitrect);
          SDL_FreeSurface(text);
        }
      }
      {
        TTF_SetFontOutline(font, 0);
        SDL_Surface *text = TTF_RenderText_Blended(font, buf, grayColor);
        if(!text) {
          fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
        } else {
          SDL_Rect blitrect = {x + (wb - text->w)/2, y+h, 0, 0};
          SDL_BlitSurface(text, NULL, surface, &blitrect);
          SDL_FreeSurface(text);
        }
      }
    }

    x += wb;
  }

  if (font) {
    TTF_CloseFont(font);
  }
}

static inline void imageInfo(SDL_Surface *surface, int x, int y, int w, int h)
{
  TTF_Font *font = TTF_OpenFont(fontName, maxi(h/2, 8));
  if(!font) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }
  char buf[16];
  sprintf(buf, "%d×%d", (int)surface->w, (int)surface->h);
  SDL_Color blackColor = {0,0,0,0};
  SDL_Color whiteColor = {255, 255, 255, 0};
  SDL_Surface *text = TTF_RenderUTF8_Shaded(font, buf, whiteColor, blackColor);
  if(text) {
    SDL_Rect blitrect = { x + (w - text->w)/2, y + (h - text->h)/2, 0, 0 };
    Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0);
    Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);
    fillRect(surface, blitrect.x-h/4, y, text->w+h/2, h, white);
    fillRect(surface, blitrect.x-h/8, y+h/8, text->w+h/4, h-h/4, black);
    SDL_BlitSurface(text, NULL, surface, &blitrect);
    SDL_FreeSurface(text);
  } else {
    fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
  }
  TTF_CloseFont(font);
}

static inline void BWLinesBar(SDL_Surface *surface, int x, int y, int w, int h)
{
  Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0);
  Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);
  int s = w/8;
  x += (w - 8*s)/2;
  for(int l = 1; l <= 4; ++l) {
    vLineRect(surface, l, x, y, s, h/2, white, black);
    vLineRect(surface, l, x+1, y+h/2, s-1, h/2, white, black);
    x += s;
  }
  for(int l = 4; l >= 1; --l) {
    hLineRect(surface, l, x, y, s/2, h, white, black);
    hLineRect(surface, l, x+s/2, y+1, s/2, h-1, white, black);
    x += s;
  }
}

static inline void colorRects(SDL_Surface *surface, int x, int y, int w, int h)
{
  Uint8 rgb[8][3] = {
    {255, 255, 255}, // white
    {255, 255, 0}, // yellow
    {0,255, 255}, // cyan
    {0, 255, 0}, // green
    {255, 0, 255}, // magenta
    {255, 0, 0}, // red
    {0, 0, 255}, // blue
    {0, 0, 0}, // black
  };

  Uint32 colors[8];
  for(int i = 0; i < 8; ++i) {
    colors[i] = SDL_MapRGB(surface->format, rgb[i][0], rgb[i][1], rgb[i][2]);
  }

  fillRect(surface, 0, 0, surface->w, y+h, colors[0]);

  int rw = w/8;
  x += (w - 7*rw)/2;
  for(int i = 1; i < 8; ++i) {
    fillRect(surface, x, y, rw, h, colors[i]);
    x += rw;
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


static void subsampleRect(SDL_Surface *surface, int x, int y, int w, int h, Uint32 color1, Uint32 color2, Uint32 color3)
{
  int w6 = w/6;
  int h6 = h/6;
  rasterRect(surface, x, y, w, h, color1, color2);
  vLineRect(surface, 1, x+2*w6, y+w6, 3*w6, h6, color1, color2);
  hLineRect(surface, 1, x+w6, y+2*w6, w6, 3*h6, color1, color2);
  fillRect(surface, x+3*w6, y+3*h6, 2*w6, 2*h6, color3);
}

static void subsampleVLines(SDL_Surface *surface, int x, int y, int w, int h, Uint32 color0, Uint32 color1, Uint32 color2, Uint32 color3, Uint32 color4)
{
  fillRect(surface, x, y, w, h, color0);
  w += x;
  h /= 4;
  for (++x; x < w; x += 2) {
    fillRect(surface, x, y+1, 1, h-2, color1);
    fillRect(surface, x, y+1+h, 1, h-2, color2);
    fillRect(surface, x, y+1+2*h, 1, h-2, color3);
    fillRect(surface, x, y+1+3*h, 1, h-2, color4);
  }
}

static void subsampleHLines(SDL_Surface *surface, int x, int y, int w, int h, Uint32 color0, Uint32 color1, Uint32 color2, Uint32 color3, Uint32 color4)
{
  fillRect(surface, x, y, w, h, color0);
  h += y;
  w /= 4;
  for (++y; y < h; y += 2) {
    fillRect(surface, x+1, y, w-2, 1, color1);
    fillRect(surface, x+1+w, y, w-2, 1, color2);
    fillRect(surface, x+1+2*w, y, w-2, 1, color3);
    fillRect(surface, x+1+3*w, y, w-2, 1, color4);
  }
}

static void colorSubsampling(SDL_Surface *surface, int x, int y, int w, int h)
{
  int w8 = mini(h, w/12);
  int m = (w - 12*w8)/2;

  // horizontal lines
  subsampleHLines(surface, x+0*w8, y, w8, h,
                  SDL_MapRGB(surface->format, 0,0,0),
                  SDL_MapRGB(surface->format, 255,255,255),
                  SDL_MapRGB(surface->format, 255,0,0),
                  SDL_MapRGB(surface->format, 0,255,0),
                  SDL_MapRGB(surface->format, 0,0,255));

  subsampleHLines(surface, x+1*w8, y, w8, h,
                  SDL_MapRGB(surface->format, 255,0, 0),
                  SDL_MapRGB(surface->format, 255,255,255),
                  SDL_MapRGB(surface->format, 0,255,255),
                  SDL_MapRGB(surface->format, 0,255,0),
                  SDL_MapRGB(surface->format, 0,0,255));

  subsampleHLines(surface, x+2*w8, y, w8, h,
                  SDL_MapRGB(surface->format, 0,255,0),
                  SDL_MapRGB(surface->format, 255,255,255),
                  SDL_MapRGB(surface->format, 255,0,0),
                  SDL_MapRGB(surface->format, 255,0,255),
                  SDL_MapRGB(surface->format, 0,0,255));

  subsampleHLines(surface, x+3*w8, y, w8, h,
                  SDL_MapRGB(surface->format, 0,0,255),
                  SDL_MapRGB(surface->format, 255,255,255),
                  SDL_MapRGB(surface->format, 255,0,0),
                  SDL_MapRGB(surface->format, 0,255,0),
                  SDL_MapRGB(surface->format, 255,255,0));

  x += m;

  // quick indicators
  subsampleRect(surface, x+4*w8, y, w8, h,
                SDL_MapRGB(surface->format, 255,255,255),
                SDL_MapRGB(surface->format, 0,0,0),
                SDL_MapRGB(surface->format, 128,128,128));

  subsampleRect(surface, x+5*w8, y, w8, h,
                SDL_MapRGB(surface->format, 255,0,0),
                SDL_MapRGB(surface->format, 0,0,255),
                SDL_MapRGB(surface->format, 128,0,128));

  subsampleRect(surface, x+6*w8, y, w8, h,
                SDL_MapRGB(surface->format, 0,0,255),
                SDL_MapRGB(surface->format, 0,255,0),
                SDL_MapRGB(surface->format, 0,146,146));

  subsampleRect(surface, x+7*w8, y, w8, h,
                SDL_MapRGB(surface->format, 0,255,0),
                SDL_MapRGB(surface->format, 255,0,0),
                SDL_MapRGB(surface->format, 132,132,0));

  x += m;
  // vertical lines
  subsampleVLines(surface, x+8*w8, y, w8, h,
                  SDL_MapRGB(surface->format, 0,0,255),
                  SDL_MapRGB(surface->format, 255,255,255),
                  SDL_MapRGB(surface->format, 255,0,0),
                  SDL_MapRGB(surface->format, 0,255,0),
                  SDL_MapRGB(surface->format, 255,255,0));

  subsampleVLines(surface, x+9*w8, y, w8, h,
                  SDL_MapRGB(surface->format, 0,255,0),
                  SDL_MapRGB(surface->format, 255,255,255),
                  SDL_MapRGB(surface->format, 255,0,0),
                  SDL_MapRGB(surface->format, 255,0,255),
                  SDL_MapRGB(surface->format, 0,0,255));

  subsampleVLines(surface, x+10*w8, y, w8, h,
                  SDL_MapRGB(surface->format, 255,0, 0),
                  SDL_MapRGB(surface->format, 255,255,255),
                  SDL_MapRGB(surface->format, 0,255,255),
                  SDL_MapRGB(surface->format, 0,255,0),
                  SDL_MapRGB(surface->format, 0,0,255));

  subsampleVLines(surface, x+11*w8, y, w8, h,
                  SDL_MapRGB(surface->format, 0,0,0),
                  SDL_MapRGB(surface->format, 255,255,255),
                  SDL_MapRGB(surface->format, 255,0,0),
                  SDL_MapRGB(surface->format, 0,255,0),
                  SDL_MapRGB(surface->format, 0,0,255));


}

static inline void copyright(SDL_Surface *surface)
{
  TTF_Font *font = TTF_OpenFont(fontName, maxi(8, surface->w/120));
  if (!font) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }
  SDL_Color grayColor = {180,180,180,0};
  SDL_Color blueColor = {0,0,255,0};
  SDL_Surface *text = TTF_RenderUTF8_Shaded(font, " Copyright © 2009-2016 Väinö Helminen ", blueColor, grayColor);
  if (text) {
    SDL_Rect blitrect = { (surface->w - text->w)/2, surface->h - text->h, 0, 0};
    SDL_BlitSurface(text, NULL, surface, &blitrect);
    SDL_FreeSurface(text);

    text = TTF_RenderUTF8_Shaded(font, " http://vah.dy.fi/testcard/ ", blueColor, grayColor);
    if (text) {
      blitrect.x = (surface->w - text->w)/2;
      blitrect.y = 0;
      SDL_BlitSurface(text, NULL, surface, &blitrect);
      SDL_FreeSurface(text);
    }
  }
  if (!text) {
    fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
  }
  TTF_CloseFont(font);

}

static inline void bigCircle(SDL_Surface *surface)
{
  int radius = 2*mini(surface->w, surface->h)/5;
  int cx = surface->w/2-1, cy = surface->h/2-1;
  Uint32 black = SDL_MapRGB(surface->format, 0,0,0);
  Uint32 gray = SDL_MapRGB(surface->format, 180,180,180);
  Uint32 white = SDL_MapRGB(surface->format, 255,255,255);
  drawCircle(surface, cx+1, cy+1, radius, 3, black);
  drawCircle(surface, cx-1, cy-1, radius, 3, white);
  drawCircle(surface, cx, cy, radius, 3, gray);
}

static inline void overscan(SDL_Surface *surface)
{
  int w = surface->w, h = surface->h;
  int w5 = (w+10)/20, w10 = (w+5)/10, h5 = (h+10)/20, h10 = (h+5)/10;
  Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0);
  Uint32 green = SDL_MapRGB(surface->format, 0, 255, 0);
  Uint32 yellow = SDL_MapRGB(surface->format, 255, 255, 0);
  SDL_Color blackColor = { 0,0,0,0 };
  SDL_Color greenColor = {0,255,0,0};
  SDL_Color yellowColor = {255,255,0,0};

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


  TTF_Font *font = TTF_OpenFont(fontName, maxi(8, w/60));
  if(!font) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }
  {
    TTF_SetFontOutline(font, 1);
    SDL_Surface *text = TTF_RenderText_Blended(font, "5%", blackColor);
    if(!text) {
      fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
    } else {
      SDL_Rect blitrect = {w-w5-2-text->w, h5, 0, 0};
      SDL_BlitSurface(text, NULL, surface, &blitrect);
    }
    text = TTF_RenderText_Blended(font, "10%", blackColor);
    if(!text) {
      fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
    } else {
      SDL_Rect blitrect = {w-w10-2-text->w, h10, 0, 0};
      SDL_BlitSurface(text, NULL, surface, &blitrect);
    }
  }
  {
    TTF_SetFontOutline(font, 0);
    SDL_Surface *text = TTF_RenderText_Blended(font, "5%", greenColor);
    if(!text) {
      fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
    } else {
      SDL_Rect blitrect = {w-w5-3-text->w, h5+1, 0, 0};
      SDL_BlitSurface(text, NULL, surface, &blitrect);
    }
    text = TTF_RenderText_Blended(font, "10%", yellowColor);
    if(!text) {
      fprintf(stderr, "TTF_Render: %s\n", TTF_GetError());
    } else {
      SDL_Rect blitrect = {w-w10-3-text->w, h10+1, 0, 0};
      SDL_BlitSurface(text, NULL, surface, &blitrect);
    }
  }

  TTF_CloseFont(font);
}


static inline void render(SDL_Surface *surface)
{
  Uint32 background = SDL_MapRGB(surface->format, 48, 48, 48);
  int x = maxi((surface->w+10)/20, (surface->h+10)/20);
  int w = surface->w - 2*x;
  int m = surface->h / 70;
  int hh = surface->h - 2*x - 4*m;
  int h = hh / 12;
  int y = x + (hh - 12*h)/2;
  SDL_FillRect(surface, NULL, background);
  colorRects  (surface, x, 0, w, y + h);
  borders(surface, x);
  copyright(surface);
  colorSubsampling(surface, x, y + 1*h + 1*m, w, 2*h);
  imageInfo   (surface, x, y + 5*h + 3*m, w, 2*h);
  BWLinesBar  (surface, x, y + 10*h + 5*m, w, 2*h);
  bigCircle(surface);
  gammaTable  (surface, x, y + 3*h + 2*m, w, 2*h);
  RGBGradients(surface, x, y + 8*h + 4*m, w, 2*h);
  overscan(surface);
}

static char *fontName="Vera.ttf";

int main(int argc, char **argv)
{
  fprintf(stdout, "Test Card - Copyright (C) 2009-2016 Vaino Helminen\n");

  bool fullscreen = true;
  bool savebmp = false;
  bool quit = false;
  int width = -1, height = -1;
  bool fail = false;
  for(int i = 1; i < argc; ++i) {
    if(argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 's':
	savebmp = true;
	continue;
      case 'w':
	fullscreen = false;
	continue;
      case 'q':
	quit = true;
	continue;
      case 'f':
        if (++i>=argc) { fail = true ; break; }
        fontName = argv[i];
	continue;
      default:
	break;
      }
    } else if(width < 0 && sscanf(argv[i], "%dx%d", &width, &height) == 2 && width > 0 && height > 0) {
      continue;
    }
    fprintf(stderr, "\nInvalid argument: %s\n\n", argv[i]);
    fail = true;
    break;
  }

  if (fail)
  {
    fprintf(stderr, "\n"
            "Usage: %s [-q] [-s] [-w] [<width>x<height>]\n"
            "\t-q\tQuit immediately (use with -s)\n"
            "\t-s\tSave image as <width>x<height>.bmp\n"
            "\t-w\tRun in window instead of fullscreen\n"
            "\t<width>x<height> Use the given resolution instead of the highest available\n"
            "\n"
            "Keys:\tUp / +\tSwitch to a higher resolution (loops to lowest)\n"
            "\tDown / -\tSwitch to a lower resolution (loops to highest)\n"
            "\ts\tSave a screenshot\n"
            "\tEsc / q\tQuit\n",
            argv[0]);
    return EXIT_FAILURE;
  }


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


  SDL_Surface *screen = setVideoMode(fullscreen, width, height, 0);
  if(!screen) {
    fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  if(fullscreen) SDL_ShowCursor(0);
  SDL_WM_SetCaption("Test Card", 0);

  render(screen);
  SDL_Flip(screen);

  for(;;) {
    if(savebmp) {
      char buf[80];
      sprintf(buf, "%dx%d.bmp", (int)screen->w, (int)screen->h);
      if(SDL_SaveBMP(screen, buf)) {
	fprintf(stderr, "SDL_SaveBMP: %s\n", SDL_GetError());
      } else {
        fprintf(stdout, "Saved a screenshot to %s\n", buf);
      }
      savebmp = false;
    }
    if(quit) return EXIT_SUCCESS;

    SDL_WaitEvent(NULL);
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT:
        return EXIT_SUCCESS;

      case SDL_KEYDOWN:
	switch(event.key.keysym.sym) {
	case SDLK_ESCAPE:
	case SDLK_q:
          return EXIT_SUCCESS;

	case SDLK_UP:
	case SDLK_PLUS:
	case SDLK_KP_PLUS:
          screen = setVideoMode(fullscreen, -1, -1, 1);
          render(screen);
          SDL_Flip(screen);
	  break;

	case SDLK_DOWN:
	case SDLK_MINUS:
	case SDLK_KP_MINUS:
	  screen = setVideoMode(fullscreen, -1, -1, -1);
          render(screen);
          SDL_Flip(screen);
	  break;

	case SDLK_s:
	  savebmp = true;
	  break;

	default:
	  break;
	}
        break;

      default:
	break;
      }
    }
  }
  return EXIT_SUCCESS;
}
