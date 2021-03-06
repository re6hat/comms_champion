//
// Copyright 2014 - 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "comms_champion/field_wrapper/IntValueWrapper.h"
#include "FieldWidget.h"

#include "ui_LongIntValueFieldWidget.h"

namespace comms_champion
{

class LongIntValueFieldWidget : public FieldWidget
{
    Q_OBJECT
    typedef FieldWidget Base;
public:
    using WrapperPtr = field_wrapper::IntValueWrapperPtr;

    explicit LongIntValueFieldWidget(
        WrapperPtr wrapper,
        QWidget* parentObj = nullptr);

    ~LongIntValueFieldWidget();

protected:
    virtual void refreshImpl() override;
    virtual void editEnabledUpdatedImpl() override;
    virtual void updatePropertiesImpl(const QVariantMap& props) override;

private slots:
    void serialisedValueUpdated(const QString& value);
    void valueUpdated(double value);

private:
    using WrapperType = WrapperPtr::element_type;
    using UnderlyingType = WrapperType::UnderlyingType;
    UnderlyingType adjustDisplayedToReal(double val);
    double adjustRealToDisplayed(UnderlyingType val);

    Ui::LongIntValueFieldWidget m_ui;
    WrapperPtr m_wrapper;
    double m_offset = 0.0;
};


}  // namespace comms_champion


