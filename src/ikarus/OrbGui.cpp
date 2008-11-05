#include "Global.h"
#include "OrbGui.h"
#include "OrbInput.h"
#include "Font.h"

// ===== Helper Functions ====================================================

bool inRect(const vec2i &v, const recti &bounds)
{
	vec2i v2(v - bounds.topLeft);
	return
		(v2.x >= 0) && (v2.y >= 0) &&
		(v2.x < bounds.size.x) && (v2.y < bounds.size.y);
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

void Label::run(OrbGui &gui, OrbLayout &lyt)
{
	vec2f szf(gui.textOut->measureText(gui.font, mText));
	vec2i sz((int)szf.x, (int)szf.y);
	recti bounds = lyt.place(sz);

	if (mEnabled)
		glColor3f(1.0f, 1.0f, 1.0f);
	else
		glColor3f(0.7f, 0.7f, 0.7f);
	glPushMatrix();
	glTranslatef((float)bounds.topLeft.x, (float)bounds.topLeft.y, 0.0f);
	gui.textOut->drawText(gui.font, mText);
	glPopMatrix();
}

// ===== Button ==============================================================

bool Button::run(OrbGui &gui, OrbLayout &lyt)
{
	bool result = false;

	vec2f szf(gui.textOut->measureText(gui.font, mText));
	vec2i sz((int)szf.x, (int)szf.y);
	sz += vec2i(4, 4);
	recti bounds = lyt.place(sz);

	if (mEnabled)
	{
		if (inRect(gui.input->getMousePos(), bounds))
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
	
	glPushMatrix();
	glTranslatef((float)bounds.topLeft.x, (float)bounds.topLeft.y, 0.0f);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(sz.x, 0);
	glVertex2i(sz.x, sz.y);
	glVertex2i(0, sz.y);
	glEnd();

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

bool CheckBox::run(OrbGui &gui, OrbLayout &lyt)
{
	bool result = mChecked;

	vec2f szf(gui.textOut->measureText(gui.font, mText));
	vec2i sz((int)szf.x, (int)szf.y);
	int chkTop = (int)((szf.y - 10.0f) / 2.0f);
	sz += vec2i(14, 4);
	recti bounds = lyt.place(sz);

	vec3f bgCol;

	if (mEnabled)
	{
		if (inRect(gui.input->getMousePos(), bounds))
			gui.requestHot(wid);
		else
			gui.releaseHot(wid);

		bool isHot = (gui.getHot() == wid);
		bool isActive = (gui.getActive() == wid);

		if (isHot)
		{
			if (isActive)
				bgCol = vec3f(0.3f, 0.7f, 0.3f);
			else
				bgCol = vec3f(0.3f, 0.3f, 0.7f);
		}
		else
			bgCol = vec3f(0.3f, 0.3f, 0.3f);

		if (isActive)
		{
			if (gui.input->wasMouseButtonReleased(MouseButton::Left))
			{
				if (isHot)
				{
					result = !result;
					mChecked = result;
				}
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
		bgCol = vec3f(0.3f, 0.3f, 0.3f);

	glPushMatrix();
	glTranslatef((float)bounds.topLeft.x, (float)bounds.topLeft.y, 0.0f);

	glDisable(GL_TEXTURE_2D);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2i( 0, chkTop);
	glVertex2i(10, chkTop);
	glVertex2i(10, chkTop+10);
	glVertex2i( 0, chkTop+10);
	glEnd();

	glColor3fv(bgCol);
	glBegin(GL_QUADS);
	glVertex2i( 0, chkTop);
	glVertex2i(10, chkTop);
	glVertex2i(10, chkTop+10);
	glVertex2i( 0, chkTop+10);
	glEnd();

	if (mEnabled)
		glColor3f(1.0f, 1.0f, 1.0f);
	else
		glColor3f(0.7f, 0.7f, 0.7f);

	if (mChecked)
	{
		glBegin(GL_LINES);
		glVertex2i( 0, chkTop); glVertex2i(10, chkTop+10);
		glVertex2i(10, chkTop); glVertex2i( 0, chkTop+10);
		glEnd();
	}

	glTranslatef(12.0f, 0.0f, 0.0f);
	gui.textOut->drawText(gui.font, mText);

	glPopMatrix();
	return result;
}

// ===== Slider ==============================================================
// ===== ComboBox ============================================================
