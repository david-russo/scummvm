/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "parallaction/parser.h"
#include "parallaction/parallaction.h"
#include "parallaction/graphics.h"
#include "parallaction/inventory.h"
#include "parallaction/zone.h"

namespace Parallaction {

void freeScript(Program*);

void freeDialogue(Dialogue *d);

Zone *Parallaction::findZone(const char *name) {

	Zone *v4 = (Zone*)_zones._next;

	while (v4) {
		if (!scumm_stricmp(name, v4->_label._text)) return v4;
		v4 = (Zone*)v4->_next;
	}

	Animation *a = findAnimation(name);
	return (a == NULL ? NULL : &a->_zone);
}




void Parallaction::parseZone(Script &script, Node *list, char *name) {
//	printf("parseZone(%s)", name);

	if (findZone(name)) {
		while (scumm_stricmp(_tokens[0], "endzone")) {
			fillBuffers(script, true);
		}
		return;
	}

	Zone *z = new Zone;

	z->_label._text = (char*)malloc(strlen(name)+1);
	strcpy(z->_label._text, name);

	addNode(list, z);

	fillBuffers(script, true);
	while (scumm_stricmp(_tokens[0], "endzone")) {
//		printf("token[0] = %s", _tokens[0]);

		if (!scumm_stricmp(_tokens[0], "limits")) {
			z->_left = atoi(_tokens[1]);
			z->_top = atoi(_tokens[2]);
			z->_right = atoi(_tokens[3]);
			z->_bottom = atoi(_tokens[4]);
		}
		if (!scumm_stricmp(_tokens[0], "moveto")) {
			z->_moveTo.x = atoi(_tokens[1]);
			z->_moveTo.y = atoi(_tokens[2]);
		}
		if (!scumm_stricmp(_tokens[0], "type")) {
			if (_tokens[2][0] != '\0') {
				z->_type = (4 + searchTable(_tokens[2], const_cast<const char **>(_objectsNames))) << 16;
			}
			int16 _si = searchTable(_tokens[1], _zoneTypeNames);
			if (_si != -1) {
				z->_type |= 1 << (_si - 1);
				parseZoneTypeBlock(script, z);
				continue;
			}
		}
		if (!scumm_stricmp(_tokens[0], "commands")) {
			z->_commands = parseCommands(script);
		}
		if (!scumm_stricmp(_tokens[0], "label")) {
//			printf("label: %s", _tokens[1]);
			_vm->_gfx->makeCnvFromString(&z->_label._cnv, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "flags")) {
			uint16 _si = 1;

			do {
				char _al = searchTable(_tokens[_si], _zoneFlagNames);
				_si++;
				z->_flags |= 1 << (_al - 1);
			} while (!scumm_stricmp(_tokens[_si++], "|"));
		}

		fillBuffers(script, true);
	}

	return;
}

void Parallaction::freeZones(Node *list) {
	debugC(1, kDebugLocation, "freeZones: kEngineQuit = %i", _engineFlags & kEngineQuit);

	Zone *z = (Zone*)list;
	Zone *v8 = NULL;

	for (; z; ) {

		// WORKAROUND: this huge condition is needed because we made TypeData a collection of structs
		// instead of an union. So, merge->_obj1 and get->_icon were just aliases in the original engine,
		// but we need to check it separately here. The same workaround is applied in hitZone.
		if (((z->_top == -1) ||
			((z->_left == -2) && (
				(((z->_type & 0xFFFF) == kZoneMerge) && ((isItemInInventory(MAKE_INVENTORY_ID(z->u.merge->_obj1)) != 0) || (isItemInInventory(MAKE_INVENTORY_ID(z->u.merge->_obj2)) != 0))) ||
				(((z->_type & 0xFFFF) == kZoneGet) && ((isItemInInventory(MAKE_INVENTORY_ID(z->u.get->_icon)) != 0)))
			))) &&
			((_engineFlags & kEngineQuit) == 0)) {

			debugC(1, kDebugLocation, "freeZones preserving zone '%s'", z->_label._text);

			v8 = (Zone*)z->_next;
			removeNode(z);
			addNode(&helperNode, z);
			z = v8;
			continue;
		}


		switch (z->_type & 0xFFFF) {
		case kZoneExamine:
			free(z->u.examine->_filename);
			free(z->u.examine->_description);
			delete z->u.examine;
			break;

		case kZoneDoor:
			free(z->u.door->_location);
			free(z->u.door->_background);
			_vm->_gfx->freeCnv(z->u.door->_cnv);
			if (z->u.door->_cnv)
				delete z->u.door->_cnv;
			delete  z->u.door;
			break;

		case kZoneSpeak:
			freeDialogue(z->u.speak->_dialogue);
			delete z->u.speak;
			break;

		case kZoneGet:
			free(z->u.get->_backup);
			_vm->_gfx->freeStaticCnv(&z->u.get->_cnv);
			delete z->u.get;
			break;

		case kZoneHear:
			delete z->u.hear;
			break;

		case kZoneMerge:
			delete z->u.merge;
			break;

		default:
			break;
		}

		free(z->_label._text);
		z->_label._text = NULL;
		_vm->_gfx->freeStaticCnv(&z->_label._cnv);
		freeCommands(z->_commands);

		// TODO: delete Zone

		z=(Zone*)z->_next;

	}

	return;
}









void Parallaction::parseZoneTypeBlock(Script &script, Zone *z) {
//	printf("parseZoneTypeBlock()");

	TypeData *u = &z->u;

	switch (z->_type & 0xFFFF) {
	case kZoneExamine:	// examine Zone alloc
		u->examine = new ExamineData;
		break;

	case kZoneDoor: // door Zone alloc
		u->door = new DoorData;
		break;

	case kZoneGet:	// get Zone alloc
		u->get = new GetData;
		break;

	case kZoneMerge:	// merge Zone alloc
		u->merge = new MergeData;
		break;

	case kZoneHear: // hear Zone alloc
		u->hear = new HearData;
		break;

	case kZoneSpeak:	// speak Zone alloc
		u->speak = new SpeakData;
		break;

	}

	char vC8[PATH_LEN];

//	printf("type = %x", z->_type);

	do {

		switch (z->_type & 0xFFFF) {
		case kZoneExamine: // examine Zone init
			if (!scumm_stricmp(_tokens[0], "file")) {
				u->examine->_filename = (char*)malloc(strlen(_tokens[1])+1);
				strcpy(u->examine->_filename, _tokens[1]);
			}
			if (!scumm_stricmp(_tokens[0], "desc")) {
				u->examine->_description = parseComment(script);
			}
			break;

		case kZoneDoor: // door Zone init
			if (!scumm_stricmp(_tokens[0], "slidetext")) {
				strcpy(_slideText[0], _tokens[1]);
//				printf("%s\t", _slideText[0]);
				strcpy(_slideText[1], _tokens[2]);
			}

			if (!scumm_stricmp(_tokens[0], "location")) {
				u->door->_location = (char*)malloc(strlen(_tokens[1])+1);
				strcpy(u->door->_location, _tokens[1]);
			}

			if (!scumm_stricmp(_tokens[0], "file")) {
//				printf("file: '%s'", _tokens[0]);

				u->door->_cnv = new Cnv;
				strcpy(vC8, _tokens[1]);

				StaticCnv vE0;
				_disk->loadFrames(vC8, u->door->_cnv);

//				printf("door width: %i, height: %i", doorcnv->_width, doorcnv->_height );

				vE0._width = u->door->_cnv->_width;
				vE0._height = u->door->_cnv->_height;

				uint16 _ax = (z->_flags & kFlagsClosed ? 0 : 1);
				vE0._data0 = u->door->_cnv->_array[_ax];

//				_ax = (z->_flags & kFlagsClosed ? 0 : 1);
//				vE0._data1 = doorcnv->field_8[_ax];

				u->door->_background = (byte*)malloc(vE0._width*vE0._height);
				_gfx->backupDoorBackground(u->door, z->_left, z->_top);

				_gfx->flatBlitCnv(&vE0, z->_left, z->_top, Gfx::kBitBack);
			}

			if (!scumm_stricmp(_tokens[0],	"startpos")) {
				u->door->_startPos.x = atoi(_tokens[1]);
				u->door->_startPos.y = atoi(_tokens[2]);
				u->door->_startFrame = atoi(_tokens[3]);
			}
			break;

		case kZoneGet: // get Zone init
			if (!scumm_stricmp(_tokens[0], "file")) {
				StaticCnv *vE4 = &u->get->_cnv;
				strcpy(vC8, _tokens[1]);
				_disk->loadStatic(vC8, vE4);
				u->get->_backup = (byte*)malloc(vE4->_width*vE4->_height);

				if ((z->_flags & kFlagsRemove) == 0) {
					_gfx->backupGetBackground(u->get, z->_left, z->_top);
					_gfx->flatBlitCnv(vE4, z->_left, z->_top, Gfx::kBitBack);
				}
			}

			if (!scumm_stricmp(_tokens[0], "icon")) {
				u->get->_icon = 4 + searchTable(_tokens[1], const_cast<const char **>(_objectsNames));
			}
			break;

		case kZoneMerge: // merge Zone init
			if (!scumm_stricmp(_tokens[0], "obj1")) {
				u->merge->_obj1 = 4 + searchTable(_tokens[1], const_cast<const char **>(_objectsNames));
			}
			if (!scumm_stricmp(_tokens[0], "obj2")) {
				u->merge->_obj2 = 4 + searchTable(_tokens[1], const_cast<const char **>(_objectsNames));
			}
			if (!scumm_stricmp(_tokens[0], "newobj")) {
				u->merge->_obj3 = 4 + searchTable(_tokens[1], const_cast<const char **>(_objectsNames));
			}
			break;

		case kZoneHear: // hear Zone init
			if (!scumm_stricmp(_tokens[0], "sound")) {
				strcpy(u->hear->_name, _tokens[1]);
			}
			break;

		case kZoneSpeak: // speak Zone init
			if (!scumm_stricmp(_tokens[0], "file")) {
				strcpy(u->speak->_name, _tokens[1]);
//				printf("speak file name: %s", u.speak._name);
			}
			if (!scumm_stricmp(_tokens[0], "Dialogue")) {
				u->speak->_dialogue = parseDialogue(script);
			}
			break;
		}

		fillBuffers(script, true);
	} while (scumm_stricmp(_tokens[0], "endzone"));

	return;
}

//	displays character head commenting an examined object
//
//	works on the frontbuffer
//
void displayCharacterComment(ExamineData *data) {
	if (data->_description == NULL) return;

	StaticCnv v3C;
	v3C._width = _vm->_char._talk->_width;
	v3C._height = _vm->_char._talk->_height;
	v3C._data0 = _vm->_char._talk->_array[0];
	v3C._data1 = NULL; //_talk->field_8[0];

	_vm->_gfx->setFont("comic");
	_vm->_gfx->flatBlitCnv(&v3C, 190, 80, Gfx::kBitFront);

	int16 v26, v28;
	_vm->_gfx->getStringExtent(data->_description, 130, &v28, &v26);
	Common::Rect r(v28, v26);
	r.moveTo(140, 10);
	_vm->_gfx->drawBalloon(r, 0);
	_vm->_gfx->displayWrappedString(data->_description, 140, 10, 130, 0);

	waitUntilLeftClick();

	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	return;
}

//
//	ZONE TYPE: EXAMINE
//

//	display detail view of an item (and eventually comments)
//
//	works on the frontbuffer
//

void displayItemComment(ExamineData *data) {

	if (data->_description == NULL) return;

	char v68[PATH_LEN];
	strcpy(v68, data->_filename);
	_vm->_disk->loadStatic(v68, &data->_cnv);
	_vm->_gfx->flatBlitCnv(&data->_cnv, 140, (SCREEN_HEIGHT - data->_cnv._height)/2, Gfx::kBitFront);
	_vm->_gfx->freeStaticCnv(&data->_cnv);

	int16 v6A = 0, v6C = 0;

	_vm->_gfx->setFont("comic");
	_vm->_gfx->getStringExtent(data->_description, 130, &v6C, &v6A);
	Common::Rect r(v6C, v6A);
	r.moveTo(0, 90);
	_vm->_gfx->drawBalloon(r, 0);
	_vm->_gfx->flatBlitCnv(_vm->_char._head, 100, 152, Gfx::kBitFront);
	_vm->_gfx->displayWrappedString(data->_description, 0, 90, 130, 0);

	jobEraseAnimations((void*)1, NULL);

	waitUntilLeftClick();

	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	return;
}



uint16 runZone(Zone *z) {
	debugC(3, kDebugLocation, "runZone (%s)", z->_label._text);

	uint16 subtype = z->_type & 0xFFFF;

	debugC(3, kDebugLocation, "type = %x, object = %x", subtype, (z->_type & 0xFFFF0000) >> 16);
	switch(subtype) {

	case kZoneExamine:
		if (z->u.examine->_filename) {
			displayItemComment(z->u.examine);
		} else {
			displayCharacterComment(z->u.examine);
		}
		break;

	case kZoneGet:
		if (z->_flags & kFlagsFixed) break;
		if (pickupItem(z) != 0) {
			return 1;
		}
		z->_flags |= kFlagsRemove;
		break;

	case kZoneDoor:
		if (z->_flags & kFlagsLocked) break;
		z->_flags ^= kFlagsClosed;
		if (z->u.door->_cnv == NULL) break;
		_vm->addJob(&jobToggleDoor, z, kPriority18 );
		break;

	case kZoneHear:
		strcpy(_soundFile, z->u.hear->_name);
		break;

	case kZoneSpeak:
		runDialogue(z->u.speak);
		break;

	}

	debugC(3, kDebugLocation, "runZone completed");

	return 0;
}

//
//	ZONE TYPE: DOOR
//
void jobToggleDoor(void *parm, Job *j) {

	static byte count = 0;

	Zone *z = (Zone*)parm;

	StaticCnv v14;

	if (z->u.door->_cnv) {
		v14._width = z->u.door->_cnv->_width;
		v14._height = z->u.door->_cnv->_height;

		Common::Rect r(z->_left, z->_top, z->_left+z->u.door->_cnv->_width, z->_top+z->u.door->_cnv->_height);

		_vm->_gfx->restoreZoneBackground(r, z->u.door->_background);

		uint16 _ax = (z->_flags & kFlagsClosed ? 0 : 1);

		v14._data0 = z->u.door->_cnv->_array[_ax];

		_vm->_gfx->flatBlitCnv(&v14, z->_left, z->_top, Gfx::kBitBack);
		_vm->_gfx->flatBlitCnv(&v14, z->_left, z->_top, Gfx::kBit2);
	}

	count++;
	if (count == 2) {
		j->_finished = 1;
		count = 0;
	}

	return;
}






//
//	ZONE TYPE: GET
//

void jobRemovePickedItem(void *parm, Job *j) {

	Zone *z = (Zone*)parm;

	static uint16 count = 0;

	if (z->u.get->_cnv._width != 0) {
		Common::Rect r(z->_left, z->_top, z->_left + z->u.get->_cnv._width, z->_top + z->u.get->_cnv._height);

		_vm->_gfx->restoreZoneBackground(r, z->u.get->_backup);
	}

	count++;
	if (count == 2) {
		count = 0;
		j->_finished = 1;
	}

	return;
}

void jobDisplayDroppedItem(void *parm, Job *j) {
//	printf("jobDisplayDroppedItem...");

	Zone *z = (Zone*)parm;

	if (&z->u.get->_cnv != NULL) {
		if (z->u.get->_cnv._data0 != NULL) {
			_vm->_gfx->backupGetBackground(z->u.get, z->_left, z->_top);
		}

		_vm->_gfx->flatBlitCnv(&z->u.get->_cnv, z->_left, z->_top, Gfx::kBitBack);
		_vm->_gfx->flatBlitCnv(&z->u.get->_cnv, z->_left, z->_top, Gfx::kBit2);
	}

	j->_count++;
	if (j->_count == 2) {
		j->_count = 0;
		j->_finished = 1;
	}

//	printf("done");

	return;
}




Zone *Parallaction::hitZone(uint32 type, uint16 x, uint16 y) {
//	printf("hitZone(%i, %i, %i)", type, x, y);

	uint16 _di = y;
	uint16 _si = x;
	Zone *z = (Zone*)_zones._next;

	for (; z; z = (Zone*)z->_next) {
//		printf("Zone name: %s", z->_name);

		if (z->_flags & kFlagsRemove) continue;

		Common::Rect r;
		z->getRect(r);
		r.right++;		// adjust border because Common::Rect doesn't include bottom-right edge
		r.bottom++;

		r.grow(-1);		// allows some tolerance for mouse click

		if (!r.contains(_si, _di)) {

			// out of Zone, so look for special values
			if ((z->_left == -2) || (z->_left == -3)) {

				// WORKAROUND: this huge condition is needed because we made TypeData a collection of structs
				// instead of an union. So, merge->_obj1 and get->_icon were just aliases in the original engine,
				// but we need to check it separately here. The same workaround is applied in freeZones.
				if ((((z->_type & 0xFFFF) == kZoneMerge) && (((_si == z->u.merge->_obj1) && (_di == z->u.merge->_obj2)) || ((_si == z->u.merge->_obj2) && (_di == z->u.merge->_obj1)))) ||
					(((z->_type & 0xFFFF) == kZoneGet) && ((_si == z->u.get->_icon) || (_di == z->u.get->_icon)))) {

					// special Zone
					if ((type == 0) && ((z->_type & 0xFFFF0000) == 0))
						return z;
					if (z->_type == type)
						return z;
					if ((z->_type & 0xFFFF0000) == type)
						return z;

				}
			}

			if (z->_left != -1)
				continue;
			if (_si < _vm->_char._ani._zone._left)
				continue;
			if (_si > (_vm->_char._ani._zone._left + _vm->_char._ani.width()))
				continue;
			if (_di < _vm->_char._ani._zone._top)
				continue;
			if (_di > (_vm->_char._ani._zone._top + _vm->_char._ani.height()))
				continue;

		}

		// normal Zone
		if ((type == 0) && ((z->_type & 0xFFFF0000) == 0))
			return z;
		if (z->_type == type)
			return z;
		if ((z->_type & 0xFFFF0000) == type)
			return z;

	}

	Animation *a = (Animation*)_animations._next;

	int16 _a, _b, _c, _d, _e, _f;
	for (; a; a = (Animation*)a->_zone._next) {
//		printf("Animation name: %s", a->_zone._name);

		_a = (a->_zone._flags & kFlagsActive) ? 1 : 0;															   // _a: active Animation
		_e = ((_si >= a->_zone._left + a->width()) || (_si <= a->_zone._left)) ? 0 : 1;		// _e: horizontal range
		_f = ((_di >= a->_zone._top + a->height()) || (_di <= a->_zone._top)) ? 0 : 1;		// _f: vertical range

		_b = ((type != 0) || (a->_zone._type == kZoneYou)) ? 0 : 1; 										 // _b: (no type specified) AND (Animation is not the character)
		_c = (a->_zone._type & 0xFFFF0000) ? 0 : 1; 															// _c: Animation is not an object
		_d = ((a->_zone._type & 0xFFFF0000) != type) ? 0 : 1;													// _d: Animation is an object of the same type

		if ((_a != 0 && _e != 0 && _f != 0) && ((_b != 0 && _c != 0) || (a->_zone._type == type) || (_d != 0))) {

			return &a->_zone;

		}

	}

	return NULL;

}

} // namespace Parallaction
