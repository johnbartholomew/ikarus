#include "Global.h"
#include "OrbGui.h"
#include "OrbInput.h"
#include "Font.h"

// ===== Helper Functions ====================================================

bool inRect(const vec2i &v, const vec2i &topLeft, const vec2i &sz)
{
	vec2i v2(v - topLeft);
	return
		(v2.x >= 0) && (v2.y >= 0) &&
		(v2.x < sz.x) && (v2.y < sz.y);
}

// ===== WidgetID ============================================================

const WidgetID WidgetID::NullWID;

void WidgetID::makeHash()
{
	unsigned int h = 0;
	h = MurmurHash2(static_cast<const void*>(name.c_str()), name.size(), h);
	h = MurmurHash2(static_cast<const void*>(&data), sizeof(data), h);
	h = MurmurHash2(static_cast<const void*>(&idx), sizeof(idx), h);
	hash = h;
}

// ===== OrbGui ==============================================================

OrbGui::OrbGui(const OrbInput *input, const Font *font, TextRenderer *textOut)
:	input(input),
	font(font),
	textOut(textOut)
{
}

OrbGui::~OrbGui()
{
}

void OrbGui::releaseHot(const WidgetID &wid)
{
	if (mHot == wid)
		mHot = WidgetID::NullWID;
}

void OrbGui::requestHot(const WidgetID &wid)
{
	if (mActive.isNull() || (mActive == wid))
		mHot = wid;
}

void OrbGui::setActive(const WidgetID &wid)
{
	mActive = wid;
	if (mHot != wid)
		mHot = WidgetID::NullWID;
}

//////////////////////////////////////////////////////////////////////////////
//   WIDGETS                                                                //
//////////////////////////////////////////////////////////////////////////////

// ===== Label ===============================================================

void Label::run(OrbGui &gui, const vec2i &pos)
{
	glPushMatrix();
	glTranslatef((float)pos.x, (float)pos.y, 0.0f);
	gui.textOut->drawText(gui.font, mText);
	glPopMatrix();
}

// ===== Button ==============================================================

bool Button::run(OrbGui &gui, const vec2i &pos)
{
	bool result = false;

	vec2f szf(gui.textOut->measureText(gui.font, mText));
	vec2i sz((int)szf.x, (int)szf.y);
	
	sz += vec2i(4, 4);

	glPushMatrix();
	glTranslatef((float)pos.x, (float)pos.y, 0.0f);

	if (mEnabled)
	{
		if (inRect(gui.input->getMousePos(), pos, sz))
			gui.requestHot(wid);
		else
			gui.releaseHot(wid);

		bool isHot = (gui.getHot() == wid);
		bool isActive = (gui.getActive() == wid);

		if (isHot)
		{
			if (isActive)
				glColor3f(0.3f, 0.7f, 0.3f);
			else
				glColor3f(0.3f, 0.3f, 0.7f);
		}
		else
			glColor3f(0.3f, 0.3f, 0.3f);

		if (isActive)
		{
			if (gui.input->wasMouseButtonReleased(MouseButton::Left))
			{
				if (isHot) result = true;
				gui.setActive(WidgetID::NullWID);
			}
		}
		else
		{
			if (gui.input->wasMouseButtonPressed(MouseButton::Left) && isHot)
				gui.setActive(wid);
		}
	}
	else
		glColor3f(0.3f, 0.3f, 0.3f);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(sz.x, 0);
	glVertex2i(sz.x, sz.y);
	glVertex2i(0, sz.y);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	if (mEnabled)
		glColor3f(1.0f, 1.0f, 1.0f);
	else
		glColor3f(0.7f, 0.7f, 0.7f);
	glTranslatef(2.0f, 2.0f, 0.0f);
	gui.textOut->drawText(gui.font, mText);

	glPopMatrix();
	return result;
}

// ===== CheckBox ============================================================

bool CheckBox::run(OrbGui &gui, const vec2i &pos)
{
	glPushMatrix();
	glTranslatef((float)pos.x, (float)pos.y, 0.0f);

	glBegin(GL_QUADS);
	glVertex3f(0.0f, 3.0f, 0.0f);
	glVertex3f(6.0f, 3.0f, 0.0f);
	glVertex3f(6.0f, 9.0f, 0.0f);
	glVertex3f(0.0f, 9.0f, 0.0f);
	glEnd();
	if (mChecked)
	{
		glBegin(GL_LINES);
		glVertex3f(0.0f, 3.0f, 0.0f); glVertex3f(6.0f, 9.0f, 0.0f);
		glVertex3f(6.0f, 3.0f, 0.0f); glVertex3f(0.0f, 9.0f, 0.0f);
		glEnd();
	}
	glTranslatef(8.0f, 0.0f, 0.0f);
	gui.textOut->drawText(gui.font, mText);

	glPopMatrix();
	return false;
}

// ===== Slider ==============================================================
// ===== ComboBox ============================================================
