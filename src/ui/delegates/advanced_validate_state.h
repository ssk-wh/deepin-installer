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

typedef long AdvancedValidateStateId;

class AdvancedValidator {
public:
   typedef QSharedPointer<AdvancedValidator> Ptr;
    explicit AdvancedValidator(AdvancedValidateStateId state);
    explicit AdvancedValidator(AdvancedValidateStateId state, const Partition::Ptr& partition);

    AdvancedValidateStateId state() const;
    const Partition::Ptr partition() const;
    bool equals(const AdvancedValidator& validator) const;

private:
    Partition::Ptr partition_;
    AdvancedValidateStateId state_;
};

//typedef AdvancedValidator::Ptr AdvancedValidateState;
class AdvancedValidateState : public AdvancedValidator::Ptr
{
public:
    AdvancedValidateState();
    AdvancedValidateState(AdvancedValidator * p);
    AdvancedValidateState(AdvancedValidateStateId state);
    AdvancedValidateState(AdvancedValidateStateId state, const Partition::Ptr& partition);

    bool eqauls(const AdvancedValidateState& state) const;
    operator AdvancedValidateStateId () const;

    enum {
     InvalidId = 0x0L,
     BootFsInvalid,  // Filesystem used for /boot is not in supported fs list.
     BootPartNumberInvalid,  // Partition for /boot is not the first partition.
     BootTooSmall,
     EfiMissing,
     EfiTooSmall,
     RootMissing,
     RootTooSmall,
     PartitionTooSmall,
     Ok
    };
};

typedef QList <AdvancedValidateState> AdvancedValidateStates;

}  // namespace installer

bool operator==( const installer::AdvancedValidateState& state1, const installer::AdvancedValidateState& state2);

#endif  // INSTALLER_UI_DELEGATES_ADVANCED_VALIDATE_STATE_H
