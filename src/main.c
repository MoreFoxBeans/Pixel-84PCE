#include <string.h>
#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

#include "font/mono.h"
#include "gfx/gfx.h"

#define GetPoint(x, y)  ((x) + ((y) << WIDTH_SHIFT))

#define TRANSPARENT     250
#define GRID            251
#define BACKGROUND      252
#define CURSOR          253
#define BLACK           254
#define WHITE           255
#define BAR_WIDTH       20

#define IMAGE_WIDTH     32
#define IMAGE_HEIGHT    32
#define WIDTH_SHIFT     5
#define HEIGHT_SHIFT    5
#define IMAGE_SIZE      IMAGE_WIDTH << HEIGHT_SHIFT

void SetColor(uint8_t offset, uint8_t ir, uint8_t ig, uint8_t ib);

void DrawImage(void);
void DrawImage_NoClip(void);
void DrawImage_Clip(void);

void DrawCursor(void);
void DrawPixel(uint8_t c);

void DrawBar(void);
void DrawColor(void);

void ClearScreen(void);
void Center(void);

void UpdateRegion(void);
void UpdateSize(void);

void Save(void);
void Load(void);

uint8_t *image;
uint8_t curX = 0, curY = 0;
uint8_t color = 0;
int16_t panX = 0, panY = 0;
uint8_t zoom = 4, zoomShift = 2;
uint8_t showGrid = 1;

uint8_t last, arrows, lastarrows;

uint8_t inregion = 0;
uint16_t dispWidth = 0;
uint16_t dispHeight = 0;

uint8_t tempPalette[2] = {
    0x00, 0x00,
};

int main(void) {
    image = (uint8_t *) malloc(IMAGE_SIZE);
    
    memset(image, 22, IMAGE_SIZE);

    Load();
    
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetClipRegion(BAR_WIDTH, 0, LCD_WIDTH, LCD_HEIGHT);
    
    ClearScreen();
    
    gfx_SetColor(WHITE);
    gfx_SetTransparentColor(TRANSPARENT);
    
    gfx_SetFontData(font8x8);
    gfx_SetFontHeight(8);
    gfx_SetMonospaceFont(6);
    gfx_SetTextConfig(gfx_text_noclip);
    
    gfx_SetTextTransparentColor(TRANSPARENT);
    gfx_SetTextBGColor(TRANSPARENT);
    gfx_SetTextFGColor(0);
    
    UpdateSize();
    
    Center();
    UpdateRegion();
    
    DrawImage();
    DrawBar();
    
    gfx_BlitBuffer();
    
    kb_EnableOnLatch();
    kb_ClearOnLatch();
    
    uint8_t any;
    uint8_t numpad, muldiv;
    
    while (!(kb_On || kb_IsDown(kb_KeyMode))) {
        any = kb_AnyKey();
        
        kb_Scan();
        
        if (any > last || kb_Data[7]) {
            if (kb_Data[7]) {
                if (kb_Data[7] > lastarrows) {
                    arrows = 0;
                } else {
                    ++arrows;
                }
                
                if (arrows == 0 || (arrows > 40 && (arrows & 3) == 0)) {
                    DrawPixel(image[GetPoint(curX, curY)]);
                    
                    if (showGrid) {
                        if (zoom > 1 && (curX << zoomShift) + panX >= BAR_WIDTH && (curY << zoomShift) + panY >= 0 && (curX << zoomShift) + panX + zoom < LCD_WIDTH && (curY << zoomShift) + panY + zoom < LCD_HEIGHT) {
                            gfx_SetColor(GRID);
                            gfx_SetPixel((curX << zoomShift) + BAR_WIDTH + panX, (curY << zoomShift) + panY);
                        }
                    }
                    
                    if (kb_IsDown(kb_KeyUp) && curY > 0) --curY;
                    if (kb_IsDown(kb_KeyDown) && curY < IMAGE_HEIGHT - 1) ++curY;
                    if (kb_IsDown(kb_KeyLeft) && curX > 0) --curX;
                    if (kb_IsDown(kb_KeyRight) && curX < IMAGE_WIDTH - 1) ++curX;
                    
                    DrawCursor();
                }
            } else {
                numpad = 0;
                
                if (kb_IsDown(kb_Key7) || kb_IsDown(kb_Key8) || kb_IsDown(kb_Key9)) {
                    panY += 20;
                    numpad = 1;
                }
                
                if (kb_IsDown(kb_Key1) || kb_IsDown(kb_Key2) || kb_IsDown(kb_Key3)) {
                    panY -= 20;
                    numpad = 1;
                }
                
                if (kb_IsDown(kb_Key1) || kb_IsDown(kb_Key4) || kb_IsDown(kb_Key7)) {
                    panX += 20;
                    numpad = 1;
                }
                
                if (kb_IsDown(kb_Key3) || kb_IsDown(kb_Key6) || kb_IsDown(kb_Key9)) {
                    panX -= 20;
                    numpad = 1;
                }
                
                if (kb_IsDown(kb_Key5)) {
                    Center();
                    numpad = 1;
                }
                
                if (numpad) {
                    UpdateRegion();
                    
                    ClearScreen();
                    DrawImage();
                }
                
                muldiv = 0;
                
                if (kb_IsDown(kb_KeyMul) && zoom < 16) {
                    zoom = zoom << 1;
                    ++zoomShift;
                    muldiv = 1;
                }

                if (kb_IsDown(kb_KeyDiv) && zoom > 1) {
                    zoom = zoom >> 1;
                    --zoomShift;
                    muldiv = 1;
                }
                
                if (muldiv == 1) {
                    UpdateSize();
                    UpdateRegion();
                    
                    ClearScreen();
                    DrawImage();
                }

                if (kb_IsDown(kb_KeySto)) {
                    showGrid = 1 - showGrid;
                    
                    DrawImage();
                }
            }

            if (kb_IsDown(kb_Key2nd)) {
                image[GetPoint(curX, curY)] = color;
                
                DrawPixel(color);
                DrawCursor();
            }

            if (kb_IsDown(kb_KeyAlpha)) {
                color = image[GetPoint(curX, curY)];
                
                DrawColor();
            }

            if (kb_IsDown(kb_KeyDel)) {
                image[GetPoint(curX, curY)] = 22;
                
                DrawPixel(255);
                DrawCursor();
            }
            
            if (kb_IsDown(kb_KeyAdd)) {
                if (color < 255) ++color;
                else color = 0;
                
                DrawColor();
            }

            if (kb_IsDown(kb_KeySub)) {
                if (color > 0) --color;
                else color = 255;
                
                DrawColor();
            }
                
            gfx_BlitBuffer();
        }
        
        last = any;
        lastarrows = kb_Data[7];
    }
    
    gfx_End();
    
    kb_ClearOnLatch();
    kb_DisableOnLatch();

    Save();
    
    free(image);
    
    return 0;
}

void SetColor(uint8_t offset, uint8_t ir, uint8_t ig, uint8_t ib) {
    uint16_t r = (ir << 5) - ir;
    r = (r + 1 + (r >> 8)) >> 8;

    uint16_t g = (ig << 6) - ig;
    g = (g + 1 + (g >> 8)) >> 8;

    uint16_t b = (ib << 5) - ib;
    b = (b + 1 + (b >> 8)) >> 8;

    uint16_t o = ((g & 1) << 15) | (r << 10) | ((g >> 1) << 5) | b;

    uint16_t h1 = o & 255;
    uint16_t h2 = o >> 8;

    tempPalette[0] = (uint8_t)h1;
    tempPalette[1] = (uint8_t)h2;

    gfx_SetPalette(tempPalette, 2, offset);
}

inline void DrawImage(void) {
    gfx_SetColor(GRID);
    gfx_Rectangle(panX - 1 + BAR_WIDTH, panY - 1, dispWidth + 2, dispHeight + 2);
    
    if (inregion) {
        DrawImage_NoClip();
    } else {
        DrawImage_Clip();
    }
    
    DrawCursor();
}

void DrawImage_NoClip(void) {
    int16_t tx;
    int16_t ty;
    
    for (uint8_t x = 0; x < IMAGE_WIDTH; ++x) {
        for (uint8_t y = 0; y < IMAGE_HEIGHT; ++y) {
            tx = (x << zoomShift) + BAR_WIDTH + panX;
            ty = (y << zoomShift) + panY;
            
            gfx_SetColor(image[GetPoint(x, y)]);
            gfx_FillRectangle_NoClip(tx, ty, zoom, zoom);
            
            if (showGrid) {
                if (zoom > 2) {
                    gfx_SetColor(GRID);
                    gfx_SetPixel(tx, ty);
                }
            }
        }
    }
}

void DrawImage_Clip(void) {
    gfx_SetColor(255);
    gfx_FillRectangle(panX + BAR_WIDTH, panY, dispWidth, dispHeight);
    
    int16_t tx;
    int16_t ty;
    uint8_t in;
    
    for (uint8_t x = 0; x < IMAGE_WIDTH; ++x) {
        for (uint8_t y = 0; y < IMAGE_HEIGHT; ++y) {
            tx = (x << zoomShift) + BAR_WIDTH + panX;
            ty = (y << zoomShift) + panY;
            
            if (ty < 0) continue;
            if (tx >= LCD_WIDTH) return;
            if (tx < BAR_WIDTH || ty >= LCD_HEIGHT) break;
            
            gfx_SetColor(image[GetPoint(x, y)]);
            gfx_FillRectangle(tx, ty, zoom, zoom);
            
            if (showGrid) {
                in = tx >= BAR_WIDTH && tx < LCD_WIDTH && ty >= 0 && ty < LCD_HEIGHT;
                
                if (in && zoom > 2) {
                    gfx_SetColor(GRID);
                    gfx_SetPixel(tx, ty);
                }
            }
        }
    }
}

inline void DrawCursor(void) {
    gfx_SetColor(CURSOR);
    gfx_Rectangle((curX << zoomShift) + BAR_WIDTH + panX, (curY << zoomShift) + panY, zoom, zoom);
}

inline void DrawPixel(uint8_t c) {
    gfx_SetColor(c);
    gfx_FillRectangle((curX << zoomShift) + BAR_WIDTH + panX, (curY << zoomShift) + panY, zoom, zoom);
}

void DrawBar(void) {
    gfx_SetColor(WHITE);
    gfx_FillRectangle_NoClip(0, 0, BAR_WIDTH - 1, LCD_HEIGHT);

    gfx_SetColor(GRID);
    gfx_VertLine_NoClip(BAR_WIDTH - 1, 0, LCD_HEIGHT);
    
    DrawColor();
}

void DrawColor(void) {
    gfx_SetColor(color);
    gfx_FillRectangle_NoClip(0, LCD_HEIGHT - BAR_WIDTH + 1, BAR_WIDTH - 1, BAR_WIDTH - 1);
    
    gfx_SetColor(WHITE);
    gfx_FillRectangle_NoClip(0, LCD_HEIGHT - BAR_WIDTH - 7, BAR_WIDTH - 1, 7);

    gfx_SetTextFGColor(BLACK);
    gfx_SetTextXY(1, LCD_HEIGHT - BAR_WIDTH - 7);
    gfx_PrintUInt(color, 3);
}

inline void ClearScreen(void) {
    gfx_SetColor(BACKGROUND);
    gfx_FillRectangle_NoClip(BAR_WIDTH, 0, LCD_WIDTH - BAR_WIDTH, LCD_HEIGHT);
}

inline void Center(void) {
    panX = (LCD_WIDTH - BAR_WIDTH - dispWidth) >> 1;
    panY = (LCD_HEIGHT - dispHeight) >> 1;
}

void UpdateRegion(void) {
    if (panX >= 0 && panY >= 0 && BAR_WIDTH + panX + dispWidth < LCD_WIDTH && panY + dispHeight < LCD_HEIGHT) {
        inregion = 1;
    } else {
        inregion = 0;
    }
}

inline void UpdateSize(void) {
    dispWidth = IMAGE_WIDTH << zoomShift;
    dispHeight = IMAGE_HEIGHT << zoomShift;
}

void Save(void) {
    ti_var_t var = ti_Open("Drawing", "w");

    if (var == 0) return;

    if (ti_Write(image, 1, IMAGE_SIZE, var) != 1) return;

    ti_Close(var);

    var = 0;
}

void Load(void) {
    ti_var_t var = ti_Open("Drawing", "r");

    if (var == 0) return;

    if (ti_Read(image, 1, IMAGE_SIZE, var) != 1) return;

    ti_Close(var);

    var = 0;
}
