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

void boxPoints(const vec2i &a, const vec2i &b, int cornerRadius, bool line)
{
	if (line)
		glBegin(GL_LINE_LOOP);
	else
		glBegin(GL_POLYGON);

	if (cornerRadius == 0)
	{
		glVertex2i(a.x, a.y);
		glVertex2i(b.x, a.y);
		glVertex2i(b.x, b.y);
		glVertex2i(a.x, b.y);
		
	}
	else
	{
		glVertex2i(a.x+cornerRadius, a.y);
		glVertex2i(b.x-cornerRadius, a.y);
		glVertex2i(b.x             , a.y+cornerRadius);
		glVertex2i(b.x             , b.y-cornerRadius);
		glVertex2i(b.x-cornerRadius, b.y);
		glVertex2i(a.x+cornerRadius, b.y);
		glVertex2i(a.x             , b.y-cornerRadius);
		glVertex2i(a.x             , a.y+cornerRadius);
	}
	
	glEnd();
}

void renderBox(const vec3f &bgCol, const vec3f &borderCol, const recti &rect, int cornerRadius)
{
	vec2i a(rect.topLeft);
	vec2i b(rect.topLeft + rect.size);

	glDisable(GL_TEXTURE_2D);
	glColor3fv(bgCol);
	boxPoints(a, b, cornerRadius, false);
	glColor3fv(borderCol);
	boxPoints(a, b, cornerRadius, true);
}

void renderText(OrbGui &gui, const vec3f &col, const vec2i &pos, const std::string &text)
{
	glPushMatrix();
	glTranslatef((float)pos.x, (float)pos.y, 0.0f);
	glColor3fv(col);
	gui.textOut->drawText(gui.font, text);
	glPopMatrix();
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

void OrbGui::requestHot(const WidgetID &wid)
{
	if (mActive.isNull() || (mActive == wid))
		mHot = wid;
}

void OrbGui::releaseHot(const WidgetID &wid)
{
	if (mHot == wid)
		mHot = WidgetID::NullWID;
}

void OrbGui::setActive(const WidgetID &wid)
{
	mActive = wid;
	if (mHot != wid)
		mHot = WidgetID::NullWID;
}

void OrbGui::clearActive()
{
	if (mHot == mActive)
		mHot = WidgetID::NullWID;
	mActive = WidgetID::NullWID;
}

//////////////////////////////////////////////////////////////////////////////
//   WIDGETS                                                                //
//////////////////////////////////////////////////////////////////////////////

// ===== Label ===============================================================

void Label::run(OrbGui &gui, OrbLayout &lyt)
{
	vec2f szf(gui.textOut->measureText(gui.font, mText));
	vec2i sz((int)szf.x, (int)szf.y);
	sz += vec2i(6, 0);
	recti bounds = lyt.place(sz);

	renderText(
		gui,
		mEnabled ? vec3f(1.0f, 1.0f, 1.0f) : vec3f(0.7f, 0.7f, 0.7f),
		bounds.topLeft,
		mText
	);
}

// ===== Button ==============================================================

bool Button::run(OrbGui &gui, OrbLayout &lyt)
{
	bool result = false;

	vec2f szf(gui.textOut->measureText(gui.font, mText));
	vec2i sz((int)szf.x, (int)szf.y);
	sz += vec2i(10, 4);
	recti bounds = lyt.place(sz);

	vec3f bgCol, borderCol, textCol;

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
			if (gui.input->wasMouseReleased(MouseButton::Left))
			{
				if (isHot) result = true;
				gui.clearActive();
			}
		}
		else
		{
			if (gui.input->wasMousePressed(MouseButton::Left) && isHot)
				gui.setActive(wid);
		}

		borderCol = vec3f(1.0f, 1.0f, 1.0f);
		textCol = vec3f(1.0f, 1.0f, 1.0f);
	}
	else
	{
		bgCol = vec3f(0.3f, 0.3f, 0.3f);
		borderCol = vec3f(0.7f, 0.7f, 0.7f);
		textCol = vec3f(0.7f, 0.7f, 0.7f);
	}
	
	renderBox(bgCol, borderCol, bounds, 3);
	renderText(gui, textCol, bounds.topLeft + vec2i(5, 2), mText);

	return result;
}

// ===== CheckBox ============================================================

bool CheckBox::run(OrbGui &gui, OrbLayout &lyt)
{
	vec2f szf(gui.textOut->measureText(gui.font, mText));
	vec2i sz((int)szf.x, (int)szf.y);
	int chkTop = (int)((szf.y - 10.0f) / 2.0f);
	sz += vec2i(18, 4);
	recti bounds = lyt.place(sz);

	vec3f bgCol, textCol;

	if (mEnabled)
	{
		if (inRect(gui.input->getMousePos(), bounds))
			gui.requestHot(wid);
		else
			gui.releaseHot(wid);

		bool isHot = (gui.getHot() == wid);
		bool isActive = (gui.getActive() == wid);

		if (isActive)
		{
			if (gui.input->wasMouseReleased(MouseButton::Left))
			{
				if (isHot) mChecked = !mChecked;
				gui.clearActive();
			}
		}
		else
		{
			if (gui.input->wasMousePressed(MouseButton::Left) && isHot)
				gui.setActive(wid);
		}

		if (isHot)
		{
			if (isActive)
				bgCol = vec3f(0.3f, 0.7f, 0.3f);
			else
				bgCol = vec3f(0.3f, 0.3f, 0.7f);
		}
		else
			bgCol = vec3f(0.3f, 0.3f, 0.3f);

		textCol = vec3f(1.0f, 1.0f, 1.0f);
	}
	else
	{
		bgCol = vec3f(0.3f, 0.3f, 0.3f);
		textCol = vec3f(0.7f, 0.7f, 0.7f);
	}

	renderBox(
		bgCol, textCol,
		recti(bounds.topLeft.x, bounds.topLeft.y + chkTop, 10, 10),
		2
	);

	if (mChecked)
	{
		vec2i a(bounds.topLeft.x, bounds.topLeft.y + chkTop), b(bounds.topLeft.x + 10, bounds.topLeft.y + chkTop + 10);
		glLineWidth(1.0f);
		glBegin(GL_LINES);
		glVertex2i(a.x+1, a.y+1); glVertex2i(b.x-1, b.y-1);
		glVertex2i(b.x-1, a.y+1); glVertex2i(a.x+1, b.y-1);
		glEnd();
		glLineWidth(0.75f);
	}

	renderText(
		gui,
		textCol,
		bounds.topLeft + vec2i(15, 0),
		mText
	);

	return mChecked;
}

// ===== Slider ==============================================================

double Slider::run(OrbGui &gui, OrbLayout &lyt)
{
	recti bounds = lyt.place(vec2i((int)(mMax - mMin), (int)gui.font->getLineHeight()));

	// the centre of the grabber
	vec2i grabPos;
	recti grabBox;
	calcGrabPos(mValue, bounds, grabPos, grabBox);

	vec3f bgCol, lineCol;

	if (mEnabled)
	{
		bool inControl = inRect(gui.input->getMousePos(), bounds);
		bool inGrabber = inControl ? inRect(gui.input->getMousePos(), grabBox) : false;

		if (inControl)
			gui.requestHot(wid);
		else
			gui.releaseHot(wid);
		
		bool isHot = (gui.getHot() == wid);
		bool isActive = (gui.getActive() == wid);

		if (isActive)
		{
			int mouseX = gui.input->getMousePos().x;
			mouseX -= bounds.topLeft.x + 2;

			// update the internal current value
			double v = ((double)mouseX / (double)(bounds.size.x - 4));
			if (v < 0.0) v = 0.0;
			if (v > 1.0) v = 1.0;
			v = mMin + v*(mMax - mMin);

			// update the calculated position for the grabber
			calcGrabPos(v, bounds, grabPos, grabBox);

			if (mContinuousUpdate)
				mValue = v;

			if (gui.input->wasMouseReleased(MouseButton::Left))
			{
				gui.clearActive();
				mValue = v;
			}
		}
		else
		{
			if (gui.input->wasMousePressed(MouseButton::Left) && isHot)
			{
				vec2i clickPos = gui.input->getMouseClickPos(MouseButton::Left);

				if (inRect(clickPos, recti(grabBox.topLeft - vec2i(0, 2), grabBox.size + vec2i(0, 4))))
					gui.setActive(wid);
				else if (inRect(clickPos, bounds))
				{
					if (clickPos.x >= grabPos.x)
						mValue += mStep;
					else if (clickPos.x < grabPos.x)
						mValue -= mStep;
					if (mValue > mMax) mValue = mMax;
					if (mValue < mMin) mValue = mMin;

					calcGrabPos(mValue, bounds, grabPos, grabBox);
				}
			}
		}

		if (isActive)
			bgCol = vec3f(0.3f, 0.7f, 0.3f);
		else
		{
			if (isHot)
				bgCol = vec3f(0.3f, 0.3f, 0.7f);
			else
				bgCol = vec3f(0.3f, 0.3f, 0.3f);
		}

		lineCol = vec3f(1.0f, 1.0f, 1.0f);
	}
	else
	{
		bgCol = vec3f(0.3f, 0.3f, 0.3f);
		lineCol = vec3f(0.7f, 0.7f, 0.7f);
	}

	glDisable(GL_TEXTURE_2D);

	glColor3fv(lineCol);
	glBegin(GL_LINES);
	glVertex2i(bounds.topLeft.x + 2, grabPos.y);
	glVertex2i(bounds.topLeft.x + bounds.size.x - 2, grabPos.y);
	glEnd();

	renderBox(bgCol, lineCol, grabBox, 2);

	return mValue;
}

void Slider::calcGrabPos(double v, const recti &bounds, vec2i &grabPos, recti &grabBox) const
{
	double a = (v - mMin) / (mMax - mMin);
	grabPos = vec2i(
		bounds.topLeft.x + 2 + (int)(a * (bounds.size.x - 4)),
		bounds.topLeft.y + (bounds.size.y / 2)
	);
	grabBox = recti(grabPos.x - 2, bounds.topLeft.y + 2, 4, bounds.size.y - 4);
}

// ===== ComboBox ============================================================
