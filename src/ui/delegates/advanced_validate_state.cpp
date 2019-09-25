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

AdvancedValidator::AdvancedValidator(AdvancedValidateStateId state)
     : state_(state)
{
}

AdvancedValidator::AdvancedValidator(AdvancedValidateStateId state, const Partition::Ptr& partition)
     : partition_(partition), state_(state)
{
}

AdvancedValidateStateId AdvancedValidator::state() const
{
    return state_;
}

const Partition::Ptr AdvancedValidator::partition() const
{
    return partition_;
}

bool AdvancedValidator::equals(const AdvancedValidator& validator) const
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

AdvancedValidateState::AdvancedValidateState()
     : AdvancedValidator::Ptr(nullptr)
{
}

AdvancedValidateState::AdvancedValidateState(AdvancedValidator * p)
     : AdvancedValidator::Ptr(p)
{
}

AdvancedValidateState::AdvancedValidateState(AdvancedValidateStateId state)
     : AdvancedValidator::Ptr(new AdvancedValidator(state))
{
}

AdvancedValidateState::AdvancedValidateState(AdvancedValidateStateId state, const Partition::Ptr& partition)
     : AdvancedValidator::Ptr(new AdvancedValidator(state, partition))
{
}

bool AdvancedValidateState::eqauls(const AdvancedValidateState& state) const
{
        if(isNull() || state.isNull()) {
            return false;
        }
        return state->equals(*get());
}

AdvancedValidateState::operator AdvancedValidateStateId () const
{
        return isNull() ? InvalidId : get()->state();
}

}

bool operator==( const installer::AdvancedValidateState& state1, const installer::AdvancedValidateState& state2) {
    return state1.eqauls(state2);
}
