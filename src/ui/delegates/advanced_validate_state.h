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

#ifndef INSTALLER_UI_DELEGATES_ADVANCED_VALIDATE_STATE_H
#define INSTALLER_UI_DELEGATES_ADVANCED_VALIDATE_STATE_H

#include <QList>
#include <QDebug>
#include <partman/partition.h>

namespace installer {

typedef long ValidateStateId;

class Validator {
public:
   typedef QSharedPointer<Validator> Ptr;
    explicit Validator(ValidateStateId state);
    explicit Validator(ValidateStateId state, const Partition::Ptr& partition);

    ValidateStateId state() const;
    const Partition::Ptr partition() const;
    bool equals(const Validator& validator) const;

private:
    Partition::Ptr partition_;
    ValidateStateId state_;
};

//typedef AdvancedValidator::Ptr AdvancedValidateState;
class ValidateState : public Validator::Ptr
{
public:
    ValidateState();
    ValidateState(Validator * p);
    ValidateState(ValidateStateId state);
    ValidateState(ValidateStateId state, const Partition::Ptr& partition);

    bool eqauls(const ValidateState& state) const;
    operator ValidateStateId () const;

    enum {
     InvalidId = 0x0L,
     BootFsInvalid,  // Filesystem used for /boot is not in supported fs list.
     BootPartNumberInvalid,  // Partition for /boot is not the first partition.
     BootTooSmall,
     BootBeforeLvm,
     EfiMissing,
     EfiTooSmall,
     EfiPartNumberinvalid,
     RootMissing,
     RootTooSmall,
     PartitionTooSmall,
     MaxPrimPartErr,  // All primary partition numbers are used.
     LvmPartNumberInvalid,
     Ok
    };
};

typedef QList <ValidateState> ValidateStates;

}  // namespace installer

bool operator==( const installer::ValidateState& state1, const installer::ValidateState& state2);

#endif  // INSTALLER_UI_DELEGATES_ADVANCED_VALIDATE_STATE_H
