/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/delegates/advanced_validate_state.h"

namespace installer {

Validator::Validator(ValidateStateId state)
     : state_(state)
{
}

Validator::Validator(ValidateStateId state, const Partition::Ptr& partition)
     : partition_(partition), state_(state)
{
}

ValidateStateId Validator::state() const
{
    return state_;
}

const Partition::Ptr Validator::partition() const
{
    return partition_;
}

bool Validator::equals(const Validator& validator) const
{
        if (state() != validator.state()){
            return false;
        }
        if (partition().isNull() && validator.partition().isNull()) {
            return true;
        }
        else if (!partition().isNull() && !validator.partition().isNull()) {
            return partition()->path == validator.partition()->path
                 && partition()->device_path == validator.partition()->device_path;
        }
        else {
            return false;
        }
}

ValidateState::ValidateState()
     : Validator::Ptr(nullptr)
{
}

ValidateState::ValidateState(Validator * p)
     : Validator::Ptr(p)
{
}

ValidateState::ValidateState(ValidateStateId state)
     : Validator::Ptr(new Validator(state))
{
}

ValidateState::ValidateState(ValidateStateId state, const Partition::Ptr& partition)
     : Validator::Ptr(new Validator(state, partition))
{
}

bool ValidateState::eqauls(const ValidateState& state) const
{
        if(isNull() || state.isNull()) {
            return false;
        }
        return state->equals(*get());
}

ValidateState::operator ValidateStateId () const
{
        return isNull() ? static_cast<ValidateStateId>(InvalidId) : get()->state();
}

}

bool operator==( const installer::ValidateState& state1, const installer::ValidateState& state2) {
    return state1.eqauls(state2);
}
