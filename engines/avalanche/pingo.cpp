/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* PINGO	Full-screen sub-parts of the game. */

#include "avalanche/avalanche.h"

#include "avalanche/pingo.h"
#include "avalanche/gyro.h"
#include "avalanche/lucerna.h"
#include "avalanche/animation.h"
#include "avalanche/scrolls.h"

#include "common/textconsole.h"
#include "common/file.h"

namespace Avalanche {

Pingo::Pingo(AvalancheEngine *vm) {
	_vm = vm;
}

void Pingo::dPlot(int16 x, int16 y, Common::String z) {
	warning("STUB: Pingo::dPlot()");
}

void Pingo::bossKey() {
	warning("STUB: Pingo::bossKey()");
}

void Pingo::copy02() {   // taken from Wobble (below)
	warning("STUB: Pingo::copy02()");
}

void Pingo::copy03() {   // taken from Wobble (below)
	warning("STUB: Pingo::copy03()");
}

void Pingo::copyPage(byte frp, byte top) { // taken from Copy02 (above)
	warning("STUB: Pingo::copyPage()");
}

void Pingo::wobble() {
	warning("STUB: Pingo::wobble()");
}

void Pingo::zl(int16 x1, int16 y1, int16 x2, int16 y2) {
	warning("STUB: Pingo::zl()");
}

void Pingo::zonk() {
	warning("STUB: Pingo::zonk()");
}

void Pingo::winningPic() {
	Common::File f;
	_vm->_lucerna->dusk();

	if (!f.open("finale.avd"))
		error("AVALANCHE: Lucerna: File not found: finale.avd");

#if 0
	for (int bit = 0; bit <= 3; bit++) {
	port[0x3c4] = 2;
	port[0x3ce] = 4;
	port[0x3c5] = 1 << bit;
	port[0x3cf] = bit;
	blockread(f, mem[0xa000 * 0], 16000);
	}
#endif

	f.close();

	warning("STUB: Pingo::winningPic()");

	_vm->_lucerna->dawn();

#if 0
	do {
		_vm->_gyro->check();
	} while (!(keypressed() || (mrelease > 0)));
	while (keypressed())
		char r = readkey();
	major_redraw();
#endif

	warning("STUB: Pingo::winningPic()");
}

} // End of namespace Avalanche.
