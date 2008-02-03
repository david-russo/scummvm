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
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#include "common/advancedDetector.h"
#include "common/config-manager.h"

#include "igor/igor.h"

struct IgorGameDescription {
	Common::ADGameDescription desc;
	int gameVersion;
	int gameFlags;
};

static const IgorGameDescription igorGameDescriptions[] = {
	{
		{
			"igor",
			"Demo 1.00s",
			{
				{ "IGOR.DAT", 0, 0, 4086790 },
				{ "IGOR.FSD", 0, 0,  462564 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_DEMO
		},
		Igor::kIdEngDemo100,
		Igor::kFlagDemo | Igor::kFlagFloppy
	},
	{
		{
			"igor",
			"Demo 1.10s",
			{
				{ "IGOR.DAT", 0, 0, 4094103 },
				{ "IGOR.FSD", 0, 0,  462564 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_DEMO
		},
		Igor::kIdEngDemo110,
		Igor::kFlagDemo | Igor::kFlagFloppy
	},
	{
		{
			"igor",
			"Talkie",
			{
				{ "IGOR.EXE", 0, 0,  9115648 },
				{ "IGOR.DAT", 0, 0, 61682719 },
				{ 0, 0, 0, 0 }
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		Igor::kIdSpaCD,
		Igor::kFlagTalkie
	},
	{ AD_TABLE_END_MARKER, 0, 0 }
};

static const PlainGameDescriptor igorGameDescriptors[] = {
	{ "igor", "Igor: Objective Uikokahonia" },
	{ 0, 0 }
};

static const Common::ADParams igorDetectionParams = {
	(const byte *)igorGameDescriptions,
	sizeof(IgorGameDescription),
	1024, // number of md5 bytes
	igorGameDescriptors,
	0,
	"igor",
	0,
	0,
	Common::kADFlagAugmentPreferredTarget
};

class IgorMetaEngine : public Common::AdvancedMetaEngine {
public:
	IgorMetaEngine() : Common::AdvancedMetaEngine(igorDetectionParams) {}

	virtual const char *getName() const {
		return "Igor: Objective Uikokahonia";
	}

	virtual const char *getCopyright() const {
		return "Igor: Objective Uikokahonia (C) Pendulo Studios";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const Common::EncapsulatedADGameDesc &encapsulatedDesc) const;
};

bool IgorMetaEngine::createInstance(OSystem *syst, Engine **engine, const Common::EncapsulatedADGameDesc &encapsulatedDesc) const {
	const IgorGameDescription *gd = (const IgorGameDescription *)(encapsulatedDesc.realDesc);
	if (gd) {
		Igor::DetectedGameVersion dgv;
		dgv.version = gd->gameVersion;
		dgv.flags = gd->gameFlags;
		dgv.language = gd->desc.language;
		dgv.ovlFileName = gd->desc.filesDescriptions[0].fileName;
		dgv.sfxFileName = gd->desc.filesDescriptions[1].fileName;
		*engine = new Igor::IgorEngine(syst, &dgv);
	}
	return gd != 0;
}

META_COMPATIBILITY_WRAPPER(IGOR, IgorMetaEngine);

REGISTER_PLUGIN(IGOR, "Igor: Objective Uikokahonia", "Igor: Objective Uikokahonia (C) Pendulo Studios");
