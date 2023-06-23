/*
 * Copyright (C) 2016-2019 AtieshCore <https://at-wow.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
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

#include "Pet.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "World.h"

class PhasedDueling : public PlayerScript
{
public:
    PhasedDueling() : PlayerScript("PhasedDueling") { }

    void OnDuelStart(Player* firstplayer, Player* secondplayer) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_PLAYER_DUEL_PHASE_ENABLED) || firstplayer->GetZoneId() == 4197)    // on Wintergrasp only in base phase)
            return;

        firstplayer->SetPhaseMask(uint32(PHASEMASK_DUEL), false);
        if (Pet* pet1 = firstplayer->GetPet())
            pet1->SetPhaseMask(uint32(PHASEMASK_DUEL), false);

        secondplayer->SetPhaseMask(uint32(PHASEMASK_DUEL), false);
        if (Pet* pet2 = secondplayer->GetPet())
            pet2->SetPhaseMask(uint32(PHASEMASK_DUEL), false);

        firstplayer->UpdPhaseForAllTotemsAndPetAndTraps();
        secondplayer->UpdPhaseForAllTotemsAndPetAndTraps();

        firstplayer->UpdateObjectVisibility();
        secondplayer->UpdateObjectVisibility();
    }

    void OnDuelEnd(Player* firstplayer, Player* secondplayer, DuelCompleteType /*type*/) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_PLAYER_DUEL_PHASE_ENABLED) || firstplayer->GetZoneId() == 4197)    // on Wintergrasp only in base phase)
            return;

        uint32 newPhasePlr1 = firstplayer->GetNormalPhase();
        uint32 newPhasePlr2 = secondplayer->GetNormalPhase();

        firstplayer->SetPhaseMask(newPhasePlr1, false);
        if (Pet* pet1 = firstplayer->GetPet())
            pet1->SetPhaseMask(newPhasePlr1, false);

        secondplayer->SetPhaseMask(newPhasePlr2, false);
        if (Pet* pet2 = secondplayer->GetPet())
            pet2->SetPhaseMask(newPhasePlr2, false);

        firstplayer->UpdPhaseForAllTotemsAndPetAndTraps();
        secondplayer->UpdPhaseForAllTotemsAndPetAndTraps();

        firstplayer->UpdateObjectVisibility();
        secondplayer->UpdateObjectVisibility();
    }
};

void AddSC_PhasedDueling()
{
    new PhasedDueling();
}
