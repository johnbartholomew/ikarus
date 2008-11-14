
=== Ikarus ===


Usage:
- Run the provided Windows executable in the windows/ directory (or if you have VC++ 2008 installed, you can re-build it using the provided .sln file)
- Options can be changed by fiddling with the controls in the panel on the left.
- Rotate the view by clicking and dragging with the right mouse button on the display
- Zoom in or out with the mouse wheel
- Move the IK target position with the keyboard:

Q  W
A  S  D
Z

Q = up (+y)
Z = down (-y)

A = left (-x)
D = right (+x)

W = forward/in (-z)
S = backward/out (+z)

Missing Functionality:
- The code is not cross-platform - it does not build on Linux.  This wouldn't be technically difficult to do, but would take time that I don't want to spend if it's not necessary.  If this is a problem and you really want to build it yourself and run it on Linux, email me and I'll do the necessary conversion.
- The constraints on the human are not set up correctly at the moment (but constraints on the simple and snake skeletons work reasonably well)

-- John Bartholomew (jb5950)
