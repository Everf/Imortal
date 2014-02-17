/*
* Copyright (C) 2012-2013 HTCore <http://cata.vfire-core.com/>
* Copyright (C) 2012-2013 WoW Source <http://wow.amgi-it.ro/>
* by Shee Shen
*/

#include "ScriptMgr.h"
#include "the_stonecore.h"

#define SAY_AGGRO "For my death, countless more will fall. The burden is now yours to bear."
#define SAY_P2 "Witness the power bestowed upon me by Deathwing! Feel the fury of earth!"
#define SAY_EARTH "The world will be reborn in flames!"
#define SAY_KILL "A sacrifice for you, master."

enum Spells
{
    // Phase 1
    SPELL_CURSE_OF_BLOOD_N = 59009, // Random player
    SPELL_CURSE_OF_BLOOD_H = 92663,
    SPELL_FORCE_GRIP = 79351, // Victim - Decast
    SPELL_SUMMON_GRAVITY_WELL = 79340, // Summon Puit
    SPELL_SUMMON_GRAVITY_WELL_AURA = 79244, // Puit Aura
    SPELL_SUMMON_GRAVITY_WELL_DAMAGE = 47756, // Puit Damage
    // Phase 2
    SPELL_SEISMIC_SHARD = 79002, // Random player
    SPELL_ENERGY_SHIELD_N = 79050, // Shield
    SPELL_ENERGY_SHIELD_H = 92667, // Shield
};

enum Summons
{
    MOB_DEVOUT_FOLLOWER = 42428,
    MOB_GRAVITY_WELL = 42499,
};

const Position addSpawnLocations[] =
{
    {1387.417725f, 1020.721863f, 211.693954f, 3.581235f},
    {1265.707520f, 958.458191f, 205.395737f, 0.608493f},
    {1331.645508f, 991.484741f, 207.999710f, 0.00f},
};

enum Phases
{
    PHASE_ALL = 0,
    PHASE_NORMAL = 1,
    PHASE_SUMMONS = 2,
};

/*********************
** High Priestess Azil
*********************/
class boss_priestess_azil : public CreatureScript
{
public:
    boss_priestess_azil() : CreatureScript("boss_priestess_azil") { }

    struct boss_priestess_azilAI : public ScriptedAI
    {
        boss_priestess_azilAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
            pInstance = pCreature->GetInstanceScript();
        }

        std::list<uint64> SummonList;

        InstanceScript *pInstance;

        uint8 Phase;
        bool Phased;
        uint8 SpawnCount;
        uint8 PhaseCount;

        uint32 CurseBloodTimer;
        uint32 ForceGripTimer;
        uint32 SummonGravityWellTimer;
        uint32 SummonDevoutTimer;
        uint32 Phase2EndTimer;

        void Reset()
        {
            Phased = false;
            RemoveSummons();

            Phase = PHASE_NORMAL;

            SpawnCount = 20;
            PhaseCount = 0;

            CurseBloodTimer = urand(8000, 13000);
            ForceGripTimer = 22000;
            SummonGravityWellTimer = urand(11000,16000);
            SummonDevoutTimer = urand(19000,27000);

            me->RemoveAurasDueToSpell(DUNGEON_MODE(SPELL_ENERGY_SHIELD_N,SPELL_ENERGY_SHIELD_H));

            me->GetMotionMaster()->MoveTargetedHome();
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            switch(summon->GetEntry())
            {
                case MOB_DEVOUT_FOLLOWER:
                    SpawnCount--;
                    break;
            }
        }

        void RemoveSummons()
        {
            if (SummonList.empty())
                return;

            for (std::list<uint64>::const_iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
            {
                if (Creature* pTemp = Unit::GetCreature(*me, *itr))
                    if (pTemp)
                        pTemp->DisappearAndDie();
            }
            SummonList.clear();
        }

        void JustSummoned(Creature* pSummon)
        {
            switch (pSummon->GetEntry())
            {
                case MOB_DEVOUT_FOLLOWER:
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        pSummon->AI()->AttackStart(pTarget);
                    SummonList.push_back(pSummon->GetGUID());
                break;
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            me->MonsterYell(SAY_AGGRO, LANG_UNIVERSAL, NULL);
        }

        void JustDied(Unit* /*pKiller*/)
        {
            RemoveSummons();
        }
		
        void KilledUnit(Unit * /*victim*/)
        {
            me->MonsterYell(SAY_KILL, LANG_UNIVERSAL, NULL);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (SpawnCount == 0 && Phase == PHASE_SUMMONS)
            {
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
                SpawnCount = 10;
                SetCombatMovement(true);
                Phase = PHASE_NORMAL;
                Phased = false;
                CurseBloodTimer = urand(8000,13000);
                ForceGripTimer = 22000;
                SummonGravityWellTimer = urand(11000,16000);
                SummonDevoutTimer = urand(19000,27000);
                me->RemoveAurasDueToSpell(DUNGEON_MODE(SPELL_ENERGY_SHIELD_N,SPELL_ENERGY_SHIELD_H));
            }

            if (me->HealthBelowPct(67) && Phase == PHASE_NORMAL && PhaseCount == 0)
            {
                me->MonsterYell(SAY_P2, LANG_UNIVERSAL, NULL);

                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
                PhaseCount++;
                SetCombatMovement(false);
                Phase = PHASE_SUMMONS;
                DoTeleportTo(1337.89f, 963.287f, 214.184f, 1.8407);
                DoCast(me, DUNGEON_MODE(SPELL_ENERGY_SHIELD_N,SPELL_ENERGY_SHIELD_H));
                DoCast(SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true), SPELL_SEISMIC_SHARD);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[1].GetPositionX(),addSpawnLocations[1].GetPositionY(),addSpawnLocations[1].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[1].GetPositionX(),addSpawnLocations[1].GetPositionY(),addSpawnLocations[1].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[1].GetPositionX(),addSpawnLocations[1].GetPositionY(),addSpawnLocations[1].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[1].GetPositionX(),addSpawnLocations[1].GetPositionY(),addSpawnLocations[1].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                Phase2EndTimer = 60000;
            }

            if (me->HealthBelowPct(34) && Phase == PHASE_NORMAL && PhaseCount == 1)
            {
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
                me->MonsterYell(SAY_P2, LANG_UNIVERSAL, NULL);

                PhaseCount++;
                SetCombatMovement(false);
                Phase = PHASE_SUMMONS;
                DoTeleportTo(1337.89f, 963.287f, 214.184f, 1.8407);
                DoCast(me, DUNGEON_MODE(SPELL_ENERGY_SHIELD_N,SPELL_ENERGY_SHIELD_H));
                DoCast(SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true), SPELL_SEISMIC_SHARD);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[1].GetPositionX(),addSpawnLocations[1].GetPositionY(),addSpawnLocations[1].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[1].GetPositionX(),addSpawnLocations[1].GetPositionY(),addSpawnLocations[1].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[1].GetPositionX(),addSpawnLocations[1].GetPositionY(),addSpawnLocations[1].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[1].GetPositionX(),addSpawnLocations[1].GetPositionY(),addSpawnLocations[1].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                Phase2EndTimer = 60000;
            }

            if (CurseBloodTimer <= diff && Phase == PHASE_NORMAL)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(target, DUNGEON_MODE(SPELL_CURSE_OF_BLOOD_N,SPELL_CURSE_OF_BLOOD_H));
                CurseBloodTimer = urand(5000,7000);
            } else CurseBloodTimer -= diff;

            if (ForceGripTimer <= diff && Phase == PHASE_NORMAL)
            {
                me->InterruptNonMeleeSpells(true);
                DoCastVictim(SPELL_FORCE_GRIP);
                ForceGripTimer = urand(12000,15000);
            } else ForceGripTimer -= diff;

            if (SummonGravityWellTimer <= diff && Phase == PHASE_NORMAL)
            {
                me->MonsterYell(SAY_EARTH, LANG_UNIVERSAL, NULL);
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true))
                    DoCast(target, SPELL_SUMMON_GRAVITY_WELL);
                SummonGravityWellTimer = urand(13000,16000);
            } else SummonGravityWellTimer -= diff;
			
            if (SummonDevoutTimer <= diff && Phase == PHASE_NORMAL)
            {
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[0].GetPositionX(),addSpawnLocations[0].GetPositionY(),addSpawnLocations[0].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                me->SummonCreature(MOB_DEVOUT_FOLLOWER, addSpawnLocations[1].GetPositionX(),addSpawnLocations[1].GetPositionY(),addSpawnLocations[1].GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                SummonDevoutTimer = urand(19000,27000);
            } else SummonDevoutTimer -= diff;

            if (Phase == PHASE_SUMMONS)
            {
                if (Phase2EndTimer <= diff)
                {
                    SpawnCount = 10;
                    SetCombatMovement(true);
                    Phase = PHASE_NORMAL;
                    Phased = false;
                    CurseBloodTimer = urand(8000,13000);
                    ForceGripTimer = 22000;
                    SummonGravityWellTimer = urand(11000,16000);
                    SummonDevoutTimer = urand(19000,27000);
                    me->RemoveAurasDueToSpell(DUNGEON_MODE(SPELL_ENERGY_SHIELD_N,SPELL_ENERGY_SHIELD_H));
                } else Phase2EndTimer -= diff;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature *pCreature) const
    {
        return new boss_priestess_azilAI (pCreature);
    }
};

/**********************
** Trigger Gravity Well
**********************/
class npc_gravity_well : public CreatureScript
{
public:
    npc_gravity_well() : CreatureScript("npc_gravity_well") { }

    struct npc_gravity_wellAI : public ScriptedAI
    {
        npc_gravity_wellAI(Creature * pCreature) : ScriptedAI(pCreature) {}

        uint32 GravityTimer;
	
        void Reset()
        {
            GravityTimer = 10000;
	
            me->AddAura(SPELL_SUMMON_GRAVITY_WELL_AURA, me);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
            return;
	
            if (GravityTimer <= diff)
            {
                DoCast(SPELL_SUMMON_GRAVITY_WELL_DAMAGE);
                GravityTimer = 10000;
            } else GravityTimer -= diff;
	
            me->DespawnOrUnsummon(20000);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gravity_wellAI(creature);
    }
};

void AddSC_boss_priestess_azil()
{
    new boss_priestess_azil();
    new npc_gravity_well();
}