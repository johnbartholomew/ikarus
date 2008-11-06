#!BPY
"""
Name: 'Skeleton (.skl)...'
Blender: 248a
Group: 'Export'
Tip: 'Export armature data to a simple text Skeleton format.'
"""

__version__ = '0.1'
__bpydoc__  = 'A simple skeleton exporter'

#########################################################################################

import math
import Blender
from Blender import Armature
from Blender import Scene
from Blender.Mathutils import *

class BoneInfo:
	__slots__ = ['parentId', 'beginChildren', 'endChildren', 'id', 'name', 'headPos', 'dir', 'numChildren']
	pass

def armature_root(bones):
	bones = [bone for bone in bones.values() if bone.hasParent() == False]
	if len(bones) == 1:
		return bones[0]
	else:
		raise Exception('Could not find root bone.')
	return None

def applyParentIds(bones, begin, end, parent_id):
	for info in bones[begin:end]:
		info.parentId = parent_id
		applyParentIds(bones, info.beginChildren, info.endChildren, info.id)

def processBones(root, scale):
	bones = []
	next_bones = [root]
	current_id = 0
	while current_id < len(next_bones):
		bone = next_bones[current_id]
		info = BoneInfo()

		info.id = current_id
		info.name = bone.name
		info.headPos = Vector(bone.head['ARMATURESPACE'])
		info.dir = Vector(bone.tail['ARMATURESPACE']) - info.headPos
		info.numChildren = len(bone.children)

		next_bones.extend(bone.children)

		bones.append(info)
		current_id += 1

	# work out the child sequences
	next_id = 1
	for info in bones:
		info.beginChildren = next_id
		info.endChildren = next_id + info.numChildren
		next_id += info.numChildren

	# work out the parent ids
	applyParentIds(bones, 0, 1, -1)

	return bones

def writeBones(fl, bones):
	for b in bones:
		line = 'bone   %-18s   % 8f % 8f % 8f   % 8f % 8f % 8f   % 3d   % 3d % 3d' % (
					b.name,
					b.headPos[0], b.headPos[1], b.headPos[2],
					b.dir[0], b.dir[1], b.dir[2],
					b.parentId, b.beginChildren, b.endChildren
				)
		fl.write(line)
		fl.write('\n')
		print line

def export_skl(filename):
	if not filename.endswith('.skl'):
		filename += '.skl'

	# get the selected object
	scn = Scene.GetCurrent()
	arm = scn.objects.active

	# check that it's an armature
	if not arm or arm.type != 'Armature':
		Draw.PupMenu('The active object must be an armature')
		return

	# get the actual armature object
	arm = arm.getData()

	fl = open(filename, 'w')
	fl.write('skeleton\n')
	bones = processBones(armature_root(arm.bones), 1.0)
	fl.write('bonecount %d\n' % len(bones))
	writeBones(fl, bones)
	fl.close()


def export_skl_ui(filename):
	Blender.Window.WaitCursor(1)
	export_skl(filename)
	Blender.Window.WaitCursor(0)

if __name__ == '__main__':
	Blender.Window.FileSelector(export_skl_ui, 'Skeleton Export', Blender.Get('filename').replace('.blend', '.skl'))
