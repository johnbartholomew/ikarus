#ifndef ORB_GUI_H
#define ORB_GUI_H

#include "OrbWidgetID.h"

class OrbInput;
class Font;
class TextRenderer;

class OrbGui;
class OrbWidget;

class OrbGui
{
public:
	explicit OrbGui(const OrbInput *input, const Font *font, TextRenderer *textOut);
	~OrbGui();

	const WidgetID &getActive() const
	{ return mActive; }
	const WidgetID &getHot() const
	{ return mHot; }
	
	void requestHot(const WidgetID &wid);
	void releaseHot(const WidgetID &wid);
	void setActive(const WidgetID &wid);
	void clearActive();

	const Font *font;
	TextRenderer *textOut;
	const OrbInput *input;
private:
	WidgetID mHot;
	WidgetID mActive;
};

class OrbLayout
{
public:
	// place() is used to place layouts and other things for which there is no preferred size
	virtual recti place() = 0;
	// place(<size>) is used to place things which have a preferred size
	// if a coordinate of the size is zero then it is taken to mean the widget can stretch arbitrarily in that direction
	virtual recti place(const vec2i &size) = 0;
};

class FixedLayout : public OrbLayout
{
public:
	explicit FixedLayout(int x, int y)
		: mRect(x, y, 0, 0) {}
	explicit FixedLayout(const vec2i &topLeft)
		: mRect(topLeft, vec2i(0,0)) {}
	explicit FixedLayout(int x, int y, int w, int h)
		: mRect(x, y, w, h) {}
	explicit FixedLayout(const vec2i &topLeft, const vec2i &size)
		: mRect(topLeft, size) {}
	explicit FixedLayout(const recti &rect)
		: mRect(rect) {}

	// a FixedLayout ignores the box being placed and returns a constant

	virtual recti place()
	{ return mRect; }

	virtual recti place(const vec2i &size)
	{
		recti r = mRect;
		if (r.size.x <= 0) r.size.x = size.x;
		if (r.size.y <= 0) r.size.y = size.y;
		return r;
	}
private:
	const recti mRect;
};

class StretchLayout : public OrbLayout
{
public:
	explicit StretchLayout(OrbLayout &lyt, int padLeft = 0, int padTop = 0, int padRight = 0, int padBottom = 0);

	virtual recti place();
	virtual recti place(const vec2i &size);
private:
	recti mBounds;
};

// a ColumnLayout stretches widgets to the full widget
// of the column and just lays them out in non-overlapping rows
class ColumnLayout : public OrbLayout
{
public:
	explicit ColumnLayout(OrbLayout &lyt, int padLeft = 0, int padTop = 0, int padRight = 0, int padBottom = 0, int spacing = 0);

	virtual recti place();
	virtual recti place(const vec2i &size);
private:
	recti mBounds;
	int mNextTop;
	int mSpacing;
};

class SplitLayout : public OrbLayout
{
public:
	explicit SplitLayout(OrbLayout &lyt);

	virtual recti place();
	virtual recti place(const vec2i &size);
};

class OrbWidget
{
public:
	OrbWidget(const WidgetID &id): wid(id) {}

	const WidgetID wid;
};

class Spacer : public OrbWidget
{
public:
	Spacer(const vec2i &size)
		: OrbWidget(WidgetID::NullWID), mSize(size) {}

	void run(OrbGui &gui, OrbLayout &lyt);
private:
	vec2i mSize;
};

class Label : public OrbWidget
{
public:
	Label(const std::string &text, bool enabled = true)
		: OrbWidget(WidgetID::NullWID), mText(text), mEnabled(enabled) {}
	Label(const WidgetID &id, const std::string &text, bool enabled = true)
		: OrbWidget(id), mText(text), mEnabled(enabled) {}

	void run(OrbGui &gui, OrbLayout &lyt);
private:
	std::string mText;
	bool mEnabled;
};

class Button : public OrbWidget
{
public:
	Button(const WidgetID &id, const std::string &text, bool enabled = true)
		: OrbWidget(id), mText(text), mEnabled(enabled) {}

	bool run(OrbGui &gui, OrbLayout &lyt);
private:
	std::string mText;
	bool mEnabled;
};

class CheckBox : public OrbWidget
{
public:
	CheckBox(const WidgetID &id, const std::string &text, bool checked, bool enabled = true)
		: OrbWidget(id), mText(text), mChecked(checked), mEnabled(enabled) {}

	bool run(OrbGui &gui, OrbLayout &lyt);
private:
	std::string mText;
	bool mChecked;
	bool mEnabled;
};

class Slider : public OrbWidget
{
public:
	Slider(const WidgetID &id,
		double minValue, double maxValue, double stepValue,
		double value,
		bool continuousUpdate = false, bool enabled = true
	)
	:	OrbWidget(id),
		mMin(minValue), mMax(maxValue), mStep(stepValue),
		mValue(value),
		mContinuousUpdate(continuousUpdate), mEnabled(enabled)
	{}

	double run(OrbGui &gui, OrbLayout &lyt);
private:
	void calcGrabPos(double v, const recti &bounds, vec2i &grabPos, recti &grabBox) const;
	double mMin;
	double mMax;
	double mStep;
	double mValue;
	double mContinuousUpdate;
	bool mEnabled;
};

class ComboBox : public OrbWidget
{
public:
	ComboBox(const WidgetID &id, const WidgetID &selected, bool enabled = true)
		: OrbWidget(id), mSelected(selected), mEnabled(enabled) {}

	void add(const std::string &item);
	void add(const WidgetID &itemID, const std::string &item);

	WidgetID run(OrbGui &gui, OrbLayout &lyt);
private:
	struct Entry
	{
		explicit Entry(const WidgetID &id, const std::string &text)
			: entryID(id), text(text) {}

		WidgetID entryID;
		std::string text;
	};

	WidgetID mSelected;
	std::vector<Entry> mEntries;
	bool mEnabled;

	void comboBoxPoints();
	void renderComboBox(const vec3f &bgCol, const vec3f &buttonCol, const vec3f &borderCol, const recti &bounds, int cornerRadius, bool opened) const;
	void boxPointsLeft(const vec2i &a, const vec2i &b, int splitX, int cornerRadius, bool opened) const;
	void boxPointsRight(const vec2i &a, const vec2i &b, int splitX, int cornerRadius, bool opened) const;
	void renderItemListBox(const vec3f &bgCol, const vec3f &textCol, const recti &bounds, int cornerRadius) const;
	void buildItemListText(std::string &text) const;
	const Entry *findEntry(const WidgetID &id, int *idx = 0) const;
};

#endif
