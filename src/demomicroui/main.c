#include "lgl/demo.h"
#include "lglui/demoui.h"
#include "demomicroui/microui.h"

#define ICON_CIRCLED_CROSS 0x2716

static Demo* d = NULL;
static Demoui* dui = NULL;
static mu_Context* muiContext = NULL;
static int fontIcons;

static char* cpToUTF8(int cp, char* str)
{
	int n = 0;
	if (cp < 0x80) n = 1;
	else if (cp < 0x800) n = 2;
	else if (cp < 0x10000) n = 3;
	else if (cp < 0x200000) n = 4;
	else if (cp < 0x4000000) n = 5;
	else if (cp <= 0x7fffffff) n = 6;
	str[n] = '\0';
	switch (n) {
	case 6: str[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
	case 5: str[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
	case 4: str[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
	case 3: str[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
	case 2: str[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
	case 1: str[0] = cp;
	}
	return str;
}


int uiTextWidth(mu_Font font, const char *str, int len)
{
    float bounds[4];
    nvgTextBounds(dui->vg, 0.0f, 0.0f, str, NULL, bounds);
    return (int)(bounds[2] - bounds[0]);
}

int uiTextHeight(mu_Font font)
{
    return 16;
}

void uiDrawText(const char* text, float x, float y, mu_Color color)
{
    nvgFontFace(dui->vg, "sans");
    nvgFontSize(dui->vg, 16);
	nvgTextAlign(dui->vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
    //nvgTextLetterSpacing(dui->vg, spacing);
	nvgFillColor(dui->vg, nvgRGBA(color.r,color.g,color.b,color.a));
	nvgText(dui->vg, x, y, text, NULL);
}

void uiDrawRect(mu_Rect rect, mu_Color color)
{
    nvgBeginPath(dui->vg);
	nvgRect(dui->vg, rect.x, rect.y, rect.w, rect.h);
	nvgFillColor(dui->vg, nvgRGBA(color.r,color.g,color.b,color.a));
	nvgFill(dui->vg);
}

void uiClip(mu_Rect rect)
{
    nvgScissor(dui->vg, rect.x, rect.y, rect.w, rect.h);
}

void uiDrawIcon(int id, mu_Rect rect, mu_Color color)
{
    char icon[8];
    nvgFontSize(dui->vg, rect.h*1.3f);
	nvgFontFace(dui->vg, "icons");
	nvgFillColor(dui->vg, nvgRGBA(255,255,255,32));
	nvgTextAlign(dui->vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
	nvgText(dui->vg, rect.x+rect.w-rect.h*0.55f, rect.y+rect.h*0.55f, cpToUTF8(id,icon), NULL);
}

void uiDraw()
{
    mu_Command *cmd = NULL;
    while (mu_next_command(muiContext, &cmd))
    {
      switch (cmd->type) 
      {
        case MU_COMMAND_TEXT: uiDrawText(cmd->text.str, cmd->text.pos.x, cmd->text.pos.y, cmd->text.color); break;
        case MU_COMMAND_RECT: uiDrawRect(cmd->rect.rect, cmd->rect.color); break;
        case MU_COMMAND_ICON: uiDrawIcon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
        case MU_COMMAND_CLIP: uiClip(cmd->clip.rect); break;
        default: break;
      }
    }
}

void uiProcess()
{
    if (mu_begin_window(muiContext, "My Window", mu_rect(10, 10, 140, 86)))
    {
        mu_layout_row(muiContext, 2, (int[]) { 60, -1 }, 0);

        mu_label(muiContext, "First:");
        if (mu_button(muiContext, "Button1"))
        {
            printf("Button1 pressed\n");
        }

        mu_label(muiContext, "Second:");
        if (mu_button(muiContext, "Button2"))
        {
            mu_open_popup(muiContext, "My Popup");
        }

        if (mu_begin_popup(muiContext, "My Popup"))
        {
            mu_label(muiContext, "Hello world!");
            mu_end_popup(muiContext);
        }

        mu_end_window(muiContext);
    }
}

void init()
{
    printf("Demo microui initialized\n");
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    dui = demouiInit(d);
    muiContext = (mu_Context*)malloc(sizeof(mu_Context));
    mu_init(muiContext);
    muiContext->text_width = uiTextWidth;
    muiContext->text_height = uiTextHeight;

    fontIcons = nvgCreateFont(dui->vg, "icons", "assets/fonts/entypo.ttf");
}

void terminate()
{
    demouiTerminate();
    printf("Demo microui terminated\n");
}

void update()
{
    if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
    if(d->keys[GLFW_KEY_F3].pressed)
        demouiToggleShowGraphs();
    
    if(d->keys[GLFW_KEY_F4].pressed)
        demouiToggleGraph();
    
    mu_input_mousemove(muiContext, (int)d->mouse.position.x, (int)d->mouse.position.y);
    mu_input_scroll(muiContext, 0, (int)d->mouse.wheelDelta);

    if(d->mouse.buttons[GLFW_MOUSE_BUTTON_LEFT].down){
        mu_input_mousedown(muiContext, d->mouse.position.x, d->mouse.position.y, MU_MOUSE_LEFT);
    }else{
        mu_input_mouseup(muiContext, d->mouse.position.x, d->mouse.position.y, MU_MOUSE_LEFT);
    }
    
    if(d->mouse.buttons[GLFW_MOUSE_BUTTON_RIGHT].down){
        mu_input_mousedown(muiContext, d->mouse.position.x, d->mouse.position.y, MU_MOUSE_RIGHT);
    }else{
        mu_input_mouseup(muiContext, d->mouse.position.x, d->mouse.position.y, MU_MOUSE_RIGHT);
    }
    
    mu_begin(muiContext);
    uiProcess();
    mu_end(muiContext);
}

void render()
{
    switch (d->renderPass)
    {
    case PASS_3D:
        demouiStartGPUTimer();
        break;
    case PASS_2D:
        break;
    case PASS_UI:
        {
            demouiBeginRender(d->winSize.x, d->winSize.y, d->winSize.x / d->fbSize.x);

            uiDraw();
        }
        break;
    case PASS_FLUSH:
        {
            demouiEndRender(d->winSize.x - 200 - 5, 5);
            demouiUpdateGraphs(d->cpuTime, d->frameDelta);
        }
        break;
    default:
        break;
    }
}

int main(void)
{
    d = demoCreate(init, terminate, update, render);
    demoRun();
    demoDestroy();

    return 0;
}
