#include "Global.h"
#include "OrbGui.h"
#include "OrbInput.h"

void WidgetID::makeHash()
{
	unsigned int h = 0;
	h = MurmurHash2(static_cast<const void*>(name.c_str()), name.size(), h);
	h = MurmurHash2(static_cast<const void*>(&data), sizeof(data), h);
	h = MurmurHash2(static_cast<const void*>(&idx), sizeof(idx), h);
	hash = h;
}
