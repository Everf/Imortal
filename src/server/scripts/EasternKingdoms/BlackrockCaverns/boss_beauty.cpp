/*
* Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 3 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ScriptPCH.h"
#include "blackrock_caverns.h"

class boss_beauty : public CreatureScript
{
public:
    boss_beauty() : CreatureScript("boss_beauty") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_beautyAI (creature);
    }

    struct boss_beautyAI : public ScriptedAI
    {
        boss_beautyAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset() {}

        void EnterCombat(Unit* /*who*/) {}

        void UpdateAI(uint32 Diff) OVERRIDE
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_beauty()
{
    new boss_beauty();
}