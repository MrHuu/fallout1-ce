#include <malloc.h>

#include "ctr_input.h"
#include "ctr_gfx.h"
#include "ctr_rectmap.h"

#include "vshader_shbin.h"

namespace fallout {

DVLB_s *vshader_dvlb;
shaderProgram_s program;
int8_t uLoc_projection;

C3D_Mtx topProjection;
C3D_Mtx bottomProjection;

C3D_RenderTarget *topRenderTarget;
C3D_RenderTarget *bottomRenderTarget;

uint8_t *renderTextureData;
uint16_t renderTextureWidth = 1024;
uint16_t renderTextureHeight = 512;

static C3D_Tex spritesheet_tex;
int activeTexFilter;

ctr_rectMap_t ctr_rectMap;
bool isN3DS;

void setTextureFilter(int linear)
{
    int newTexFilter;

    if (linear != -1)
        newTexFilter = linear;
    else
        newTexFilter = isN3DS ? GPU_LINEAR : GPU_NEAREST;

    if (newTexFilter != activeTexFilter) {
        C3D_TexSetFilter(&spritesheet_tex, GPU_LINEAR, (GPU_TEXTURE_FILTER_PARAM) newTexFilter);
        activeTexFilter = newTexFilter;
    }
}

inline void drawRect(const float subTexX, const float subTexY, const float subTexW, const float subTexH,
        const float posX, const float posY, const float newWidth, const float newHeight)
{
    C3D_ImmDrawBegin(GPU_TRIANGLE_STRIP);
    {
        const float texW = renderTextureWidth;
        const float texH = renderTextureHeight;

        const float vertZ = 0.5f;
        const float vertW = 1.f;

        // Bottom left corner
        C3D_ImmSendAttrib(posX, posY, vertZ, vertW);                                   // v0 = position xyzw
        C3D_ImmSendAttrib(subTexX / texW, 1.f - (subTexY + subTexH) / texH, 0.f, 0.f); // v1 = texcoord uv

        // Bottom right corner
        C3D_ImmSendAttrib(newWidth + posX, posY, vertZ, vertW);
        C3D_ImmSendAttrib((subTexX + subTexW) / texW, 1.f - (subTexY + subTexH) / texH, 0.f, 0.f);

        // Top left corner
        C3D_ImmSendAttrib(posX, newHeight + posY, vertZ, vertW);
        C3D_ImmSendAttrib(subTexX / texW, 1.f - subTexY / texH, 0.f, 0.f);

        // Top right corner
        C3D_ImmSendAttrib(newWidth + posX, newHeight + posY, vertZ, vertW);
        C3D_ImmSendAttrib((subTexX + subTexW) / texW, 1.f - subTexY / texH, 0.f, 0.f);
    }
    C3D_ImmDrawEnd();
}

void drawRects()
{
    beginRender(false);

    drawTopRenderTarget(0x000000ff);
    switch (ctr_rectMap.active)
    {
        case DISPLAY_SPLASH:
            break;

        case DISPLAY_MOVIE:
            drawRect(rectMaps[ctr_rectMap.active][0]->src_x, rectMaps[ctr_rectMap.active][0]->src_y,
                    rectMaps[ctr_rectMap.active][0]->src_w, rectMaps[ctr_rectMap.active][0]->src_h,
                    rectMaps[ctr_rectMap.active][0]->dst_x, rectMaps[ctr_rectMap.active][0]->dst_y,
                    rectMaps[ctr_rectMap.active][0]->dst_w, rectMaps[ctr_rectMap.active][0]->dst_h
            );
            break;

        case DISPLAY_DIALOG:
        case DISPLAY_INVENTORY_TRADE:
            for (int i = 0; i < numRectsInMap[DISPLAY_DIALOG_TOP]; i++) {
                drawRect(rectMaps[DISPLAY_DIALOG_TOP][i]->src_x, rectMaps[DISPLAY_DIALOG_TOP][i]->src_y,
                        rectMaps[DISPLAY_DIALOG_TOP][i]->src_w, rectMaps[DISPLAY_DIALOG_TOP][i]->src_h,
                        rectMaps[DISPLAY_DIALOG_TOP][i]->dst_x, rectMaps[DISPLAY_DIALOG_TOP][i]->dst_y,
                        rectMaps[DISPLAY_DIALOG_TOP][i]->dst_w, rectMaps[DISPLAY_DIALOG_TOP][i]->dst_h
                );
            }
            break;

        case DISPLAY_LOADSAVE:
            for (int i = 0; i < numRectsInMap[DISPLAY_LOADSAVE_TOP]; i++) {
                drawRect(rectMaps[DISPLAY_LOADSAVE_TOP][i]->src_x, rectMaps[DISPLAY_LOADSAVE_TOP][i]->src_y,
                        rectMaps[DISPLAY_LOADSAVE_TOP][i]->src_w, rectMaps[DISPLAY_LOADSAVE_TOP][i]->src_h,
                        rectMaps[DISPLAY_LOADSAVE_TOP][i]->dst_x, rectMaps[DISPLAY_LOADSAVE_TOP][i]->dst_y,
                        rectMaps[DISPLAY_LOADSAVE_TOP][i]->dst_w, rectMaps[DISPLAY_LOADSAVE_TOP][i]->dst_h
                );
            }
            break;

        case DISPLAY_FIELD:
            for (int i = 0; i < numRectsInMap[DISPLAY_GUI]; i++) {
                drawRect(rectMaps[DISPLAY_GUI][i]->src_x, rectMaps[DISPLAY_GUI][i]->src_y,
                        rectMaps[DISPLAY_GUI][i]->src_w, rectMaps[DISPLAY_GUI][i]->src_h,
                        40+rectMaps[DISPLAY_GUI][i]->dst_x, rectMaps[DISPLAY_GUI][i]->dst_y,
                        rectMaps[DISPLAY_GUI][i]->dst_w, rectMaps[DISPLAY_GUI][i]->dst_h
                );
            }
            for (int i = 0; i < getIndicatorSlotNum(); i++) {
                drawRect(rectMaps[DISPLAY_GUI_INDICATOR][i]->src_x, rectMaps[DISPLAY_GUI_INDICATOR][i]->src_y,
                        rectMaps[DISPLAY_GUI_INDICATOR][i]->src_w, rectMaps[DISPLAY_GUI_INDICATOR][i]->src_h,
                        40+rectMaps[DISPLAY_GUI_INDICATOR][i]->dst_x, rectMaps[DISPLAY_GUI_INDICATOR][i]->dst_y,
                        rectMaps[DISPLAY_GUI_INDICATOR][i]->dst_w, rectMaps[DISPLAY_GUI_INDICATOR][i]->dst_h
                );
            }
            break;

        case DISPLAY_GUI:
            drawRect((offsetX_field > 140)?140:offsetX_field, offsetY_field, 480, 300, 0,   0, 400, 240);
            break;

        case DISPLAY_AUTOMAP:
        case DISPLAY_WORLDMAP:
        case DISPLAY_PIPBOY:
        case DISPLAY_CHARACTER:
        case DISPLAY_VATS:
        case DISPLAY_FULL:
            drawRect(offsetX, (240-offsetY), 400, 240, 0,   0, 400, 240);
            break;

        default:
            drawRect(rectMaps[DISPLAY_FULL][0]->src_x, rectMaps[DISPLAY_FULL][0]->src_y,
                    rectMaps[DISPLAY_FULL][0]->src_w, rectMaps[DISPLAY_FULL][0]->src_h,
                    40+rectMaps[DISPLAY_FULL][0]->dst_x, rectMaps[DISPLAY_FULL][0]->dst_y,
                    rectMaps[DISPLAY_FULL][0]->dst_w, rectMaps[DISPLAY_FULL][0]->dst_h
            );
            break;
    }

    drawBottomRenderTarget(0x000000ff);
    switch (ctr_rectMap.active)
    {
        case DISPLAY_MOVIE:
             break;

        case DISPLAY_SPLASH:
            setTextureFilter(1);
            drawRect(rectMaps[DISPLAY_FULL][0]->src_x, rectMaps[DISPLAY_FULL][0]->src_y,
                    rectMaps[DISPLAY_FULL][0]->src_w, rectMaps[DISPLAY_FULL][0]->src_h,
                    rectMaps[DISPLAY_FULL][0]->dst_x, rectMaps[DISPLAY_FULL][0]->dst_y,
                    rectMaps[DISPLAY_FULL][0]->dst_w, rectMaps[DISPLAY_FULL][0]->dst_h
            );
            setTextureFilter(-1);
            break;

        case DISPLAY_DIALOG:
            for (int i = 0; i < numRectsInMap[ctr_rectMap.active]; i++) {
                drawRect(rectMaps[ctr_rectMap.active][i]->src_x, rectMaps[ctr_rectMap.active][i]->src_y,
                        rectMaps[ctr_rectMap.active][i]->src_w, rectMaps[ctr_rectMap.active][i]->src_h,
                        rectMaps[ctr_rectMap.active][i]->dst_x, rectMaps[ctr_rectMap.active][i]->dst_y,
                        rectMaps[ctr_rectMap.active][i]->dst_w, rectMaps[ctr_rectMap.active][i]->dst_h
                );
            }
            drawRect(rectMaps[DISPLAY_DIALOG_BACK][0]->src_x, rectMaps[DISPLAY_DIALOG_BACK][0]->src_y,
                    rectMaps[DISPLAY_DIALOG_BACK][0]->src_w, rectMaps[DISPLAY_DIALOG_BACK][0]->src_h,
                    rectMaps[DISPLAY_DIALOG_BACK][0]->dst_x, rectMaps[DISPLAY_DIALOG_BACK][0]->dst_y,
                    rectMaps[DISPLAY_DIALOG_BACK][0]->dst_w, rectMaps[DISPLAY_DIALOG_BACK][0]->dst_h
            );
            break;

        case DISPLAY_LOADSAVE:
            drawRect(rectMaps[DISPLAY_LOADSAVE_SLOT][0]->src_x,
			        rectMaps[DISPLAY_LOADSAVE_SLOT][0]->src_y + getSaveSlotOffset(),
                    rectMaps[DISPLAY_LOADSAVE_SLOT][0]->src_w, rectMaps[DISPLAY_LOADSAVE_SLOT][0]->src_h,
                    rectMaps[DISPLAY_LOADSAVE_SLOT][0]->dst_x, rectMaps[DISPLAY_LOADSAVE_SLOT][0]->dst_y,
                    rectMaps[DISPLAY_LOADSAVE_SLOT][0]->dst_w, rectMaps[DISPLAY_LOADSAVE_SLOT][0]->dst_h
            );
            for (int i = 0; i < numRectsInMap[ctr_rectMap.active]; i++) {
                drawRect(rectMaps[ctr_rectMap.active][i]->src_x, rectMaps[ctr_rectMap.active][i]->src_y,
                        rectMaps[ctr_rectMap.active][i]->src_w, rectMaps[ctr_rectMap.active][i]->src_h,
                        rectMaps[ctr_rectMap.active][i]->dst_x, rectMaps[ctr_rectMap.active][i]->dst_y,
                        rectMaps[ctr_rectMap.active][i]->dst_w, rectMaps[ctr_rectMap.active][i]->dst_h
                );
            }
            for (int i = 0; i < numRectsInMap[DISPLAY_LOADSAVE_BACK]; i++) {
                drawRect(rectMaps[DISPLAY_LOADSAVE_BACK][i]->src_x, rectMaps[DISPLAY_LOADSAVE_BACK][i]->src_y,
                        rectMaps[DISPLAY_LOADSAVE_BACK][i]->src_w, rectMaps[DISPLAY_LOADSAVE_BACK][i]->src_h,
                        rectMaps[DISPLAY_LOADSAVE_BACK][i]->dst_x, rectMaps[DISPLAY_LOADSAVE_BACK][i]->dst_y,
                        rectMaps[DISPLAY_LOADSAVE_BACK][i]->dst_w, rectMaps[DISPLAY_LOADSAVE_BACK][i]->dst_h
                );
            }
            break;

        case DISPLAY_FIELD:
            drawRect((offsetX_field > 240)?240:offsetX_field, offsetY_field, 400, 300, 0,   0, 320, 240);
            break;

        case DISPLAY_GUI:
            for (int i = 0; i < getIndicatorSlotNum(); i++) {
                drawRect(rectMaps[DISPLAY_GUI_INDICATOR][i]->src_x, rectMaps[DISPLAY_GUI_INDICATOR][i]->src_y,
                        rectMaps[DISPLAY_GUI_INDICATOR][i]->src_w, rectMaps[DISPLAY_GUI_INDICATOR][i]->src_h,
                        rectMaps[DISPLAY_GUI_INDICATOR][i]->dst_x, rectMaps[DISPLAY_GUI_INDICATOR][i]->dst_y,
                        rectMaps[DISPLAY_GUI_INDICATOR][i]->dst_w, rectMaps[DISPLAY_GUI_INDICATOR][i]->dst_h
                );
            }

        default:
            for (int i = 0; i < numRectsInMap[ctr_rectMap.active]; i++) {
                drawRect(rectMaps[ctr_rectMap.active][i]->src_x, rectMaps[ctr_rectMap.active][i]->src_y,
                        rectMaps[ctr_rectMap.active][i]->src_w, rectMaps[ctr_rectMap.active][i]->src_h,
                        rectMaps[ctr_rectMap.active][i]->dst_x, rectMaps[ctr_rectMap.active][i]->dst_y,
                        rectMaps[ctr_rectMap.active][i]->dst_w, rectMaps[ctr_rectMap.active][i]->dst_h
                );
            }
            break;
    }
    finishRender();
}

void ctr_gfx_draw(SDL_Surface* gSdlSurface)
{
    for (int y = 0; y < gSdlSurface->h; y++) {
        for (int x = 0; x < gSdlSurface->w; x++) {

            int index = y * gSdlSurface->w + x;

            Uint8 colorIndex = ((Uint8*)gSdlSurface->pixels)[index];

            SDL_Color pixelColor = gSdlSurface->format->palette->colors[colorIndex];

            u32 color = (pixelColor.r << 24) | (pixelColor.g << 16) | (pixelColor.b << 8) | 0xFF;

            int bufferIndex = (y * renderTextureWidth + x) * 4;

            ((u32*)renderTextureData)[bufferIndex / sizeof(u32)] = color;
        }
    }

    GSPGPU_FlushDataCache(renderTextureData, gSdlSurface->w*gSdlSurface->h*4);

    C3D_SyncDisplayTransfer((u32*)renderTextureData, GX_BUFFER_DIM(renderTextureWidth, renderTextureHeight),
            (u32*)spritesheet_tex.data, GX_BUFFER_DIM(renderTextureWidth, renderTextureHeight), TEXTURE_TRANSFER_FLAGS);

    GSPGPU_FlushDataCache(spritesheet_tex.data, gSdlSurface->w*gSdlSurface->h*4);

    C3D_TexBind(0, &spritesheet_tex);

    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

    drawRects();
}

void beginRender(bool vSync)
{
    C3D_FrameBegin(vSync ? C3D_FRAME_SYNCDRAW : 0);
}

void drawTopRenderTarget(uint32_t clearColor)
{
    C3D_RenderTargetClear(topRenderTarget, C3D_CLEAR_ALL, clearColor, 0);
    C3D_FrameDrawOn(topRenderTarget);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &topProjection);
}

void drawBottomRenderTarget(uint32_t clearColor)
{
    C3D_RenderTargetClear(bottomRenderTarget, C3D_CLEAR_ALL, clearColor, 0);
    C3D_FrameDrawOn(bottomRenderTarget);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &bottomProjection);
}

void finishRender()
{
    C3D_FrameEnd(0);
}

void initTransferTexture()
{
    uint32_t renderTextureByteCount = renderTextureWidth*renderTextureHeight*4;
    renderTextureData = (uint8_t *)linearAlloc(renderTextureByteCount);

    memset(renderTextureData, 0, renderTextureByteCount);

    C3D_TexInit(&spritesheet_tex, renderTextureWidth, renderTextureHeight,  GPU_RGBA8);

    setTextureFilter(-1);
}

void ctr_gfx_init()
{
    if(gspHasGpuRight())
        gfxExit();

    APT_CheckNew3DS(&isN3DS);

    gfxInitDefault();

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    topRenderTarget = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP, GPU_RB_RGBA8, GPU_RB_DEPTH16);
    C3D_RenderTargetSetOutput(topRenderTarget, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    bottomRenderTarget = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_BOTTOM, GPU_RB_RGBA8, GPU_RB_DEPTH16);
    C3D_RenderTargetSetOutput(bottomRenderTarget, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

    vshader_dvlb = DVLB_ParseFile((uint32_t *)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    C3D_BindProgram(&program);

    uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");

    C3D_AttrInfo *attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord

    Mtx_OrthoTilt(&topProjection, 0, GSP_SCREEN_HEIGHT_TOP, 0, GSP_SCREEN_WIDTH, 0.1f, 1.0f, true);
    Mtx_OrthoTilt(&bottomProjection, 0, GSP_SCREEN_HEIGHT_BOTTOM, 0, GSP_SCREEN_WIDTH, 0.1f, 1.0f, true);

    initTransferTexture();
}

void ctr_gfx_exit()
{
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
    C3D_Fini();
}

} // namespace fallout
