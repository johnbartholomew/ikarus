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
	
	void releaseHot(const WidgetID &wid);
	void requestHot(const WidgetID &wid);
	void setActive(const WidgetID &wid);

	const Font *font;
	TextRenderer *textOut;
	const OrbInput *input;
private:
	WidgetID mHot;
	WidgetID mActive;
};

class OrbWidget
{
public:
	OrbWidget(const WidgetID &id): wid(id) {}

	const WidgetID wid;
};

class Label : public OrbWidget
{
public:
	Label(const WidgetID &id, const std::string &text, bool enabled = true)
		: OrbWidget(id), mText(text), mEnabled(enabled) {}

	void run(OrbGui &gui, const vec2i &pos);
private:
	std::string mText;
	bool mEnabled;
};

class Button : public OrbWidget
{
public:
	Button(const WidgetID &id, const std::string &text, bool enabled = true)
		: OrbWidget(id), mText(text), mEnabled(enabled) {}

	bool run(OrbGui &gui, const vec2i &pos);
private:
	std::string mText;
	bool mEnabled;
};

class CheckBox : public OrbWidget
{
public:
	CheckBox(const WidgetID &id, const std::string &text, bool checked, bool enabled)
		: OrbWidget(id), mText(text), mChecked(checked), mEnabled(enabled) {}

	bool run(OrbGui &gui, const vec2i &pos);
private:
	std::string mText;
	bool mChecked;
	bool mEnabled;
};

class Slider : public OrbWidget
{
public:
	Slider(const WidgetID &id, double minValue, double maxValue, double value, bool enabled)
		: OrbWidget(id), mMin(minValue), mMax(maxValue), mValue(value), mEnabled(enabled) {}

	double run(OrbGui &gui, const vec2i &pos);
private:
	double mMin;
	double mMax;
	double mValue;
	bool mEnabled;
};

class ComboBox : public OrbWidget
{
public:
	ComboBox(const WidgetID &id, const WidgetID &selected, bool enabled = true)
		: OrbWidget(id), mSelected(selected), mEnabled(enabled) {}

	void add(const std::string &item);
	void add(const WidgetID &itemID, const std::string &item);

	WidgetID run(OrbGui &gui, const vec2i &pos);
private:
	struct Entry
	{
		WidgetID entryID;
		std::string text;
	};

	WidgetID mSelected;
	std::vector<Entry> mEntries;
	bool mEnabled;
};

#endif
