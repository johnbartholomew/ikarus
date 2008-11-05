#ifndef ORB_GUI_H
#define ORB_GUI_H

#include "OrbWidgetID.h"

class OrbInput;

class OrbGui
{
public:
	explicit OrbGui(const OrbInput *input);
	~OrbGui();

private:
	WidgetID mHot;
	WidgetID mActive;

	const OrbInput *mInput;
};


#endif
