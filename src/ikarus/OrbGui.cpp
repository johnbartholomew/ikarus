#include "Global.h"
#include "OrbGui.h"
#include "OrbInput.h"
#include "Font.h"
#include "GfxUtil.h"

// ===== Helper Functions ====================================================

void renderText(OrbGui &gui, const vec3f &col, const vec2i &pos, const std::string &text, float depth = 0.0f)
{
	glPushMatrix();
	glTranslatef((float)pos.x, (float)pos.y, depth);
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
//   LAYOUTS                                                                //
//////////////////////////////////////////////////////////////////////////////

// ===== StretchLayout =======================================================

StretchLayout::StretchLayout(OrbLayout &lyt, int padLeft, int padTop, int padRight, int padBottom)
{
	mBounds = lyt.place();
	mBounds.topLeft.x += padLeft;
	mBounds.topLeft.y += padTop;
	mBounds.size.x -= (padLeft + padRight);
	mBounds.size.y -= (padTop + padBottom);
}

recti StretchLayout::place()
{
	return mBounds;
}

recti StretchLayout::place(const vec2i &size)
{
	return mBounds;
}

// ===== ColumnLayout ========================================================

ColumnLayout::ColumnLayout(OrbLayout &lyt, int padLeft, int padTop, int padRight, int padBottom, int spacing)
:	mSpacing(spacing)
{
	mBounds = lyt.place();
	mBounds.topLeft.x += padLeft;
	mBounds.topLeft.y += padTop;
	mBounds.size.x -= (padLeft + padRight);
	mBounds.size.y -= (padTop + padBottom);
	
	mNextTop = mBounds.topLeft.y;
}

recti ColumnLayout::place()
{
	recti r(mBounds.topLeft.x, mNextTop, mBounds.size.x, mBounds.size.y - mNextTop);
	mNextTop += r.size.y + mSpacing;
	return r;
}

recti ColumnLayout::place(const vec2i &size)
{
	recti r(mBounds.topLeft.x, mNextTop, mBounds.size.x, size.y);
	mNextTop += r.size.y + mSpacing;
	return r;
}

//////////////////////////////////////////////////////////////////////////////
//   WIDGETS                                                                //
//////////////////////////////////////////////////////////////////////////////

// ===== Spacer ==============================================================

void Spacer::run(OrbGui &gui, OrbLayout &lyt)
{
	lyt.place(mSize);
}

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
		if (bounds.contains(gui.input->getMousePos()))
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
		if (bounds.contains(gui.input->getMousePos()))
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
		bool inControl = bounds.contains(gui.input->getMousePos());
		bool inGrabber = inControl ? grabBox.contains(gui.input->getMousePos()) : false;

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

				if (recti(grabBox.topLeft - vec2i(0, 2), grabBox.size + vec2i(0, 4)).contains(clickPos))
					gui.setActive(wid);
				else if (bounds.contains(clickPos))
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

void ComboBox::add(const std::string &item)
{
	add(WidgetID(item), item);
}

void ComboBox::add(const WidgetID &itemID, const std::string &item)
{
	mEntries.push_back(Entry(itemID, item));
}

WidgetID ComboBox::run(OrbGui &gui, OrbLayout &lyt)
{
	const Entry *curItem = 0;
	int curItemIdx = 0;
	const Entry *e = findEntry(mSelected, &curItemIdx);
	curItem = e;

	vec2f szf;
	if (e == 0)
		szf = vec2f(0.0f, gui.font->getLineHeight());
	else
		szf = gui.textOut->measureText(gui.font, e->text);
	vec2i sz((int)szf.x, (int)szf.y);
	sz += vec2i(10, 4);
	sz.x += sz.y; // square button
	recti bounds = lyt.place(sz);

	vec3f bgCol, buttonCol, selCol, textCol;

	bool isHot = false, isActive = false;
	std::string itemListText;
	recti listBounds;

	if (mEnabled)
	{
		const vec2i mousePos = gui.input->getMousePos();
		bool isInsideBox = bounds.contains(mousePos);
		
		isActive = (gui.getActive() == wid);

		if (!isActive)
		{
			if (isInsideBox)
				gui.requestHot(wid);
			else
				gui.releaseHot(wid);
		}

		isHot = (gui.getHot() == wid);

		if (isHot)
		{
			if (isActive && isInsideBox)
				buttonCol = vec3f(0.3f, 0.7f, 0.3f);
			else
				buttonCol = vec3f(0.3f, 0.3f, 0.7f);
		}
		else
			buttonCol = vec3f(0.3f, 0.3f, 0.3f);

		if (isActive)
		{
			buildItemListText(itemListText);
			
			const vec2f listSzf(gui.textOut->measureText(gui.font, itemListText));
			vec2i listSz((int)listSzf.x, (int)listSzf.y);
			listSz += vec2i(10, 4);

			listBounds.topLeft = vec2i(bounds.topLeft.x, bounds.topLeft.y + bounds.size.y);
			listBounds.size.x = std::max(bounds.size.x, listSz.x);
			listBounds.size.y = listSz.y;

			if (listBounds.contains(mousePos))
			{
				curItemIdx = (mousePos.y - listBounds.topLeft.y - 2) / (int)gui.font->getLineHeight();
				if (curItemIdx < 0) curItemIdx = 0;
				if (curItemIdx >= (int)mEntries.size()) curItemIdx = (int)mEntries.size() - 1;
				curItem = &mEntries[curItemIdx];
				selCol = vec3f(0.3f, 0.7f, 0.3f);
			}
			else
				selCol = vec3f(0.3f, 0.3f, 0.7f);

			if (gui.input->wasMouseReleased(MouseButton::Left))
			{
				if (!isInsideBox)
					gui.clearActive();
				if (curItem != 0)
					mSelected = curItem->entryID;
			}
		}
		else
		{
			if (gui.input->wasMousePressed(MouseButton::Left) && isHot)
				gui.setActive(wid);
		}

		bgCol = vec3f(0.3f, 0.3f, 0.3f);
		textCol = vec3f(1.0f, 1.0f, 1.0f);
	}
	else
	{
		buttonCol = vec3f(0.3f, 0.3f, 0.3f);
		bgCol = vec3f(0.3f, 0.3f, 0.3f);
		textCol = vec3f(0.7f, 0.7f, 0.7f);
	}
	
	renderComboBox(bgCol, buttonCol, textCol, bounds, 3, isActive);
	if (e == 0)
		renderText(gui, textCol, bounds.topLeft + vec2i(5, 2), "");
	else
		renderText(gui, textCol, bounds.topLeft + vec2i(5, 2), e->text);

	if (isActive)
	{
		glDisable(GL_TEXTURE_2D);

		renderItemListBox(bgCol, textCol, listBounds, 3);

		int itemHeight = (int)gui.font->getLineHeight();
		vec2i selA(listBounds.topLeft.x, listBounds.topLeft.y + 2 + itemHeight*curItemIdx);
		vec2i selB(listBounds.topLeft.x + listBounds.size.x, selA.y + itemHeight);

		glColor3fv(selCol);
		glBegin(GL_QUADS);
		glVertex3i(selA.x+1, selA.y, -2);
		glVertex3i(selB.x-1, selA.y, -2);
		glVertex3i(selB.x-1, selB.y, -2);
		glVertex3i(selA.x+1, selB.y, -2);
		glEnd();

		renderText(gui, textCol, listBounds.topLeft + vec2i(5, 2), itemListText, -3.0f);
	}

	return mSelected;
}

void ComboBox::renderComboBox(const vec3f &bgCol, const vec3f &buttonCol, const vec3f &borderCol, const recti &bounds, int cornerRadius, bool opened) const
{
	const vec2i &a(bounds.topLeft);
	const vec2i &b(bounds.topLeft + bounds.size);
	const int splitX = b.x - bounds.size.y;
	
	glDisable(GL_TEXTURE_2D);

	glColor3fv(bgCol);
	glBegin(GL_POLYGON);
	boxPointsLeft(a, b, splitX, cornerRadius, opened);
	glEnd();

	glColor3fv(buttonCol);
	glBegin(GL_POLYGON);
	boxPointsRight(a, b, splitX, cornerRadius, opened);
	glEnd();

	glColor3fv(borderCol);
	glBegin(GL_LINE_STRIP);
	boxPointsLeft(a, b, splitX, cornerRadius, opened);
	boxPointsRight(a, b, splitX, cornerRadius, opened);
	glEnd();

	// down arrow
	const int tx = splitX + bounds.size.y/2;
	const int ty = a.y + (bounds.size.y - 5)/2;
	glBegin(GL_POLYGON);
	glVertex2i(tx - 3, ty);
	glVertex2i(tx + 3, ty);
	glVertex2i(  tx  , ty + 6);
	glEnd();
}

void ComboBox::boxPointsLeft(const vec2i &a, const vec2i &b, int splitX, int cornerRadius, bool opened) const
{
	glVertex2i(splitX, a.y);
	glVertex2i(a.x + cornerRadius, a.y);
	glVertex2i(a.x, a.y + cornerRadius);
	if (opened)
		glVertex2i(a.x, b.y);
	else
	{
		glVertex2i(a.x, b.y - cornerRadius);
		glVertex2i(a.x + cornerRadius, b.y);
	}
	glVertex2i(splitX, b.y);
}

void ComboBox::boxPointsRight(const vec2i &a, const vec2i &b, int splitX, int cornerRadius, bool opened) const
{
	glVertex2i(splitX, a.y);
	glVertex2i(b.x - cornerRadius, a.y);
	glVertex2i(b.x, a.y + cornerRadius);
	if (opened)
		glVertex2i(b.x, b.y);
	else
	{
		glVertex2i(b.x, b.y - cornerRadius);
		glVertex2i(b.x - cornerRadius, b.y);
	}
	glVertex2i(splitX, b.y);
}

void ComboBox::renderItemListBox(const vec3f &bgCol, const vec3f &textCol, const recti &bounds, int cornerRadius) const
{
	const vec2i a(bounds.topLeft);
	const vec2i b(bounds.topLeft + bounds.size);

	glColor3fv(bgCol);
	glBegin(GL_POLYGON);
	{
		glVertex3i(a.x, a.y, -1);
		glVertex3i(b.x, a.y, -1);
		glVertex3i(b.x, b.y - cornerRadius, -1);
		glVertex3i(b.x - cornerRadius, b.y, -1);
		glVertex3i(a.x + cornerRadius, b.y, -1);
		glVertex3i(a.x, b.y - cornerRadius, -1);
	}
	glEnd();
	
	glColor3fv(textCol);
	glBegin(GL_LINE_LOOP);
	{
		glVertex3i(a.x, a.y, -1);
		glVertex3i(b.x, a.y, -1);
		glVertex3i(b.x, b.y - cornerRadius, -1);
		glVertex3i(b.x - cornerRadius, b.y, -1);
		glVertex3i(a.x + cornerRadius, b.y, -1);
		glVertex3i(a.x, b.y - cornerRadius, -1);
	}
	glEnd();
}

void ComboBox::buildItemListText(std::string &text) const
{
	std::ostringstream ss;
	std::vector<Entry>::const_iterator it = mEntries.begin();
	while (it != mEntries.end())
	{
		ss << it->text;
		++it;
		if (it != mEntries.end())
			ss << "\n";
	}
	text = ss.str();
}

const ComboBox::Entry *ComboBox::findEntry(const WidgetID &id, int *idx) const
{
	int i = 0;
	std::vector<Entry>::const_iterator it = mEntries.begin();
	while (it != mEntries.end())
	{
		if (it->entryID == id)
		{
			if (idx != 0)
				*idx = i;
			return &(*it);
		}
		++it;
		++i;
	}
	return 0;
}
