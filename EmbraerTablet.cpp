// EmbraerTablet.cpp
#include <MSFS/MSFS.h>
#include <MSFS/MSFS_Core.h>
#include <MSFS/MSFS_Render.h>
#include "MSFS\Render\nanovg.h"
#include <MSFS/Legacy/gauges.h>

#include <stdio.h>
#include "EmbraerTablet.h"
#include <GdiPlus.h>
#include <map>
#include "Button.h"
#include "TextEdit.h"

std::map < FsContext, NVGcontext*> g_TabletNVGContext;

// Returns 1 if col.rgba is 0.0f,0.0f,0.0f,0.0f, 0 otherwise
int isBlack(NVGcolor col)
{
	if (col.r == 0.0f && col.g == 0.0f && col.b == 0.0f && col.a == 0.0f)
	{
		return 1;
	}
	return 0;
}

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

void drawButton(NVGcontext* vg, int preicon, const char* text, float x, float y, float w, float h, NVGcolor col)
{
	NVGpaint bg;
	char icon[8];
	float cornerRadius = 4.0f;
	float tw = 0, iw = 0;

	bg = nvgLinearGradient(vg, x, y, x, y + h, nvgRGBA(255, 255, 255, isBlack(col) ? 16 : 32), nvgRGBA(0, 0, 0, isBlack(col) ? 16 : 32));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x + 1, y + 1, w - 2, h - 2, cornerRadius - 1);
	if (!isBlack(col)) {
		nvgFillColor(vg, col);
		nvgFill(vg);
	}
	nvgFillPaint(vg, bg);
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgRoundedRect(vg, x + 0.5f, y + 0.5f, w - 1, h - 1, cornerRadius - 0.5f);
	nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 48));
	nvgStroke(vg);

	nvgFontSize(vg, 17.0f);
	nvgFontFace(vg, "sans-bold");
	tw = nvgTextBounds(vg, 0, 0, text, nullptr, nullptr);
	if (preicon != 0) {
		nvgFontSize(vg, h * 1.3f);
		nvgFontFace(vg, "icons");
		iw = nvgTextBounds(vg, 0, 0, cpToUTF8(preicon, icon), nullptr, nullptr);
		iw += h * 0.15f;
	}

	if (preicon != 0) {
		nvgFontSize(vg, h * 1.3f);
		nvgFontFace(vg, "icons");
		nvgFillColor(vg, nvgRGBA(255, 255, 255, 96));
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
		nvgText(vg, x + w * 0.5f - tw * 0.5f - iw * 0.75f, y + h * 0.5f, cpToUTF8(preicon, icon), nullptr);
	}

	nvgFontSize(vg, 17.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	nvgFillColor(vg, nvgRGBA(0, 0, 0, 160));
	nvgText(vg, x + w * 0.5f - tw * 0.5f + iw * 0.25f, y + h * 0.5f - 1, text, nullptr);
	nvgFillColor(vg, nvgRGBA(255, 255, 255, 160));
	nvgText(vg, x + w * 0.5f - tw * 0.5f + iw * 0.25f, y + h * 0.5f, text, nullptr);
}

void drawLabel(NVGcontext* vg, const char* text, float x, float y, float w, float h)
{
	NVG_NOTUSED(w);

	nvgFontSize(vg, 20.0f);
	nvgFontFace(vg, "sans");
	nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));

	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	nvgText(vg, x, y + h * 0.5f, text, nullptr);
}

void drawEditBoxBase(NVGcontext* vg, float x, float y, float w, float h)
{
	NVGpaint bg;
	// Edit
	bg = nvgBoxGradient(vg, x + 1, y + 1 + 1.5f, w - 2, h - 2, 3, 4, nvgRGBA(255, 255, 255, 32), nvgRGBA(32, 32, 32, 32));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x + 1, y + 1, w - 2, h - 2, 4 - 1);
	nvgFillPaint(vg, bg);
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgRoundedRect(vg, x + 0.5f, y + 0.5f, w - 1, h - 1, 4 - 0.5f);
	nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 48));
	nvgStroke(vg);
}

void drawEditBox(NVGcontext* vg, const char* text, float x, float y, float w, float h)
{

	drawEditBoxBase(vg, x, y, w, h);

	nvgFontSize(vg, 20.0f);
	nvgFontFace(vg, "sans");
	nvgFillColor(vg, nvgRGBA(255, 255, 255, 64));
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	nvgText(vg, x + h * 0.3f, y + h * 0.5f, text, nullptr);
}

const int LEFT_MARGIN = 20;
const int TOP_MARGIN = 20;
const int BOTTOM_MARGIN = 20;

struct Data {
	int fontNormal = 0, fontBold = 0, fontIcons = 0, fontEmoji = 0;
	int images[12] = { 0 };
	int page = 1;
};
Data	TabletData;

ButtonData ButtonPage1;
ButtonData ButtonPage2;
TextEditData TextEditName;
TextEditData TextEditCompany;

int loadData(NVGcontext* vg)
{
	TabletData.fontNormal = nvgCreateFont(vg, "sans", "./data/Roboto-Regular.ttf");
	if (TabletData.fontNormal == -1)
	{
		fprintf(stderr, "Could not add font italic.\n");
		return -1;
	}

	TabletData.fontBold = nvgCreateFont(vg, "sans-bold", "./data/Roboto-Bold.ttf");
	if (TabletData.fontBold == -1) {
		fprintf(stderr, "Could not add font bold.\n");
		return -1;
	}

	nvgAddFallbackFontId(vg, TabletData.fontNormal, TabletData.fontEmoji);

	return 0;
}

void drawPageStatus(NVGcontext* nvgctx, sGaugeDrawData* p_draw_data)
{
	char buffer[50];
	sprintf(buffer, "Current page: %d", TabletData.page);
	const char* result = buffer;
	drawLabel(nvgctx, result, (float)p_draw_data->fbWidth / 2, 0, 280, 30);
}

void drawPage1(NVGcontext* nvgctx, sGaugeDrawData* p_draw_data)
{
	drawPageStatus(nvgctx, p_draw_data);

	drawButton(nvgctx, 0, ButtonPage1.name, ButtonPage1.x, ButtonPage1.y, ButtonPage1.width, ButtonPage1.height, nvgRGBA(0, 0, 200, 100));
}

void drawPage2(NVGcontext* nvgctx, sGaugeDrawData* p_draw_data)
{
	drawPageStatus(nvgctx, p_draw_data);

	drawLabel(nvgctx, "Pilot info", LEFT_MARGIN, TOP_MARGIN, 300, 22);

	drawEditBox(nvgctx, "Name", LEFT_MARGIN, TOP_MARGIN + 30, 300, 30);

	drawEditBox(nvgctx, "Company", LEFT_MARGIN, TOP_MARGIN + 60, 300, 30);

	drawButton(nvgctx, 0, ButtonPage2.name, ButtonPage2.x, ButtonPage2.y, ButtonPage2.width, ButtonPage2.height, nvgRGBA(0, 0, 200, 100));
}

void initButtons(sGaugeDrawData* p_draw_data)
{
	ButtonPage1.width = 100; ButtonPage1.height = 100;
	ButtonPage1.x = LEFT_MARGIN; ButtonPage1.y = TOP_MARGIN;
	ButtonPage1.name = "Click me";

	ButtonPage2.width = 100; ButtonPage2.height = 100;
	ButtonPage2.x = LEFT_MARGIN; ButtonPage2.y = p_draw_data->fbHeight - ButtonPage2.height - BOTTOM_MARGIN;
	ButtonPage2.name = "Go back";
}

bool isButtonHit(const ButtonData& button, int mouseX, int mouseY) {
	return (mouseX >= button.x && mouseX <= button.x + button.width 
		&& mouseY >= button.y && mouseY <= button.y + button.height);
}

extern "C" {
	MSFS_CALLBACK bool EmbraerTablet_gauge_callback(FsContext ctx, int service_id, void* pData)
	{
		switch (service_id)
		{
		case PANEL_SERVICE_PRE_INSTALL:
		{
			sGaugeInstallData* p_install_data = (sGaugeInstallData*)pData;

			return true;
		}
		case PANEL_SERVICE_POST_INSTALL:
		{
			NVGparams params;
			params.userPtr = ctx;
			params.edgeAntiAlias = true;
			g_TabletNVGContext[ctx] = nvgCreateInternal(&params);
			NVGcontext* nvgctx = g_TabletNVGContext[ctx];

			if (loadData(nvgctx) == -1)
			{
				fprintf(stderr, "Failed to load data\n");
				return false;
			}

			return true;
		}
		case PANEL_SERVICE_PRE_DRAW:
		{
			sGaugeDrawData* p_draw_data = (sGaugeDrawData*)pData;

			initButtons(p_draw_data);

			float pxRatio = (float)p_draw_data->fbWidth / (float)p_draw_data->winWidth;
			NVGcontext* nvgctx = g_TabletNVGContext[ctx];
			nvgBeginFrame(nvgctx, p_draw_data->winWidth, p_draw_data->winHeight, pxRatio);
			{
				// Blue background
				nvgFillColor(nvgctx, nvgRGB(0, 0, 50));
				nvgBeginPath(nvgctx);
				nvgRect(nvgctx, 0, 0, p_draw_data->winWidth, p_draw_data->winHeight);
				nvgFill(nvgctx);

				switch (TabletData.page)
				{
				case 1:
				{
					drawPage1(nvgctx, p_draw_data);
					break;
				}
				case 2:
				{
					drawPage2(nvgctx, p_draw_data);
					break;
				}
				default:
					break;
				}
			}
			nvgEndFrame(nvgctx);

			return true;
		}
		case PANEL_SERVICE_PRE_KILL:
		{
			NVGcontext* nvgctx = g_TabletNVGContext[ctx];
			nvgDeleteInternal(nvgctx);
			g_TabletNVGContext.erase(ctx);
			return true;
		}
		}
		return false;
	}

	MSFS_CALLBACK void EmbraerTablet_mouse_callback(float fX, float fY, unsigned int iFlags)
	{
		switch (iFlags)
		{
		case MOUSE_LEFTSINGLE:
		{
			if (TabletData.page == 1)
			{
				if (isButtonHit(ButtonPage1, fX, fY))
				{
					TabletData.page = 2;
				}
			}
			else
			{
				if (isButtonHit(ButtonPage2, fX, fY))
				{
					TabletData.page = 1;
				}
			}
			break;
		}
		default:
			break;
		}
	}
}
