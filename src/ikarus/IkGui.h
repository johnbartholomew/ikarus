#ifndef IK_GUI_H
#define IK_GUI_H

class IkGui
{
public:
	bool isKeyUp(int key) const
	{ return ((keys[key] & Down) == 0); }
	bool isKeyDown(int key) const
	{ return (keys[key] & Down); }
	bool wasKeyPressed(int key) const
	{ return (keys[key] == Pressed); }
	bool wasKeyReleased(int key) const
	{ return (keys[key] == Released); }
	bool didKeyChange(int key) const
	{ return (keys[key] & ChangedMask); }
private:
	enum KeyState
	{
		Up       = 0x0,
		Down     = 0x1,
		Released = 0x2,
		Pressed  = 0x3,

		ChangedMask = 0x2
	};

	// mouse state
	vec3i mousePos;
	vec3i mouseDelta;
	vec3i mouseDragBase;
	bool mouseL, mouseR, mouseC;

	std::vector<KeyState> keys;
	std::string inputBuf;

	WidgetID hot;
	WidgetID active;
};


#endif
