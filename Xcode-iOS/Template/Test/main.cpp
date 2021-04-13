/*
 *  rectangles.c
 *  written by Holmes Futrell
 *  use however you want
 */

#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLUtil.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkColor.h"
#include "include/core/SkSurface.h"
#include "include/utils/SkRandom.h"
#include "include/core/SkSwizzle.h"
#include "include/core/SkVertices.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480

SDL_GLContext gl_context;
int contextType;

int drawableWidth, drawableHeight;

sk_sp<const GrGLInterface> interface;
sk_sp<GrDirectContext> grContext;

GrGLint buffer;
GrGLFramebufferInfo info;

SkColorType colorType;

GrBackendRenderTarget target;
SkSurfaceProps props;

sk_sp<SkSurface> surface;
SkCanvas *canvas = NULL;

static const int kStencilBits = 8;     // Skia needs 8 stencil bits
static const int kMsaaSampleCount = 0; //4;

int
randomInt(int min, int max)
{
    return min + rand() % (max - min + 1);
}

void
render(SDL_Renderer *renderer)
{

    SDL_Rect rect;
    Uint8 r, g, b;

    /* Clear the screen */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    /*  Come up with a random rectangle */
    rect.w = randomInt(64, 128);
    rect.h = randomInt(64, 128);
    rect.x = randomInt(0, SCREEN_WIDTH);
    rect.y = randomInt(0, SCREEN_HEIGHT);

    /* Come up with a random color */
    r = randomInt(50, 255);
    g = randomInt(50, 255);
    b = randomInt(50, 255);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

    /*  Fill the rectangle in the color */
    SDL_RenderFillRect(renderer, &rect);

    /* update screen */
    SDL_RenderPresent(renderer);
}

int
main(int argc, char *argv[])
{

    SDL_Window *window;
    SDL_Renderer *renderer;
    int done;
    SDL_Event event;

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
        printf("Could not initialize SDL\n");
        return 1;
    }

    /* seed random number generator */
    srand(time(NULL));
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    uint32_t windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                  SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP |
                  SDL_WINDOW_ALLOW_HIGHDPI;

    /* create window and renderer */
    window =
        SDL_CreateWindow(NULL, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                         windowFlags);
    if (!window) {
        printf("Could not initialize Window\n");
        return 1;
    }

    
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &contextType);

    SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
    printf("Drawable size: %i %i\n", drawableWidth, drawableHeight);
    
    interface = GrGLMakeNativeInterface();
    SkASSERT(interface);
    
    // setup contexts
    grContext = GrDirectContext::MakeGL(interface);
    SkASSERT(grContext);
    
    // Wrap the frame buffer object attached to the screen in a Skia render target so Skia can
    // render to it
    GR_GL_GetIntegerv(interface.get(), GR_GL_FRAMEBUFFER_BINDING, &buffer);

    info.fFBOID = (GrGLuint)buffer;
    
    uint32_t windowFormat = SDL_GetWindowPixelFormat(window);
    if (SDL_PIXELFORMAT_RGBA8888 == windowFormat)
    {
        info.fFormat = GR_GL_RGBA8;
        colorType = kRGBA_8888_SkColorType;
    }
    else
    {
        colorType = kBGRA_8888_SkColorType;
        if (SDL_GL_CONTEXT_PROFILE_ES == contextType)
        {
            info.fFormat = GR_GL_BGRA8;
        }
        else
        {
            // We assume the internal format is RGBA8 on desktop GL
            info.fFormat = GR_GL_RGBA8;
        }
    }

    target = GrBackendRenderTarget(drawableWidth, drawableHeight, kMsaaSampleCount, kStencilBits, info);
    surface = SkSurface::MakeFromBackendRenderTarget(   grContext.get(),
                                                        target,
                                                        kBottomLeft_GrSurfaceOrigin,
                                                        colorType, nullptr, &props);
    canvas = surface->getCanvas();
    /*
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        printf("Could not create renderer\n");
        return 1;
    }
     */

    /* Enter render loop, waiting for user to quit */
    done = 0;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            }
        }
        canvas->clear(SK_ColorWHITE);
        
        
        
        canvas->flush();
        SDL_GL_SwapWindow(window);
        //render(renderer);
        SDL_Delay(1);
    }

    /* shutdown SDL */
    SDL_Quit();

    return 0;
}
