#pragma once

#include "Array.h"
#include "LevelSetupOption.h"
#include "Types.h"

namespace fb {
	struct LevelSetup {
		LevelSetup() {
			m_difficultyIndex = 0;
			m_isSaveGame = false;
			m_forceReloadResources = false;
		};

		String m_name;
		Array<LevelSetupOption> m_inclusionOptions;
		u32 m_difficultyIndex;
		Array<String> m_subLevelNames;
		String m_startPoint;
		Array<s32> m_subLevelStates;
		bool m_isSaveGame;
		bool m_forceReloadResources;
	};
}
