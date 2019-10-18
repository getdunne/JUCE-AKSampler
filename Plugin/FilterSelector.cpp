//
//  FilterSelector.cpp
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#include "FilterSelector.h"

const char* const FilterSelector::names[] = {
    "None", "LP12", "LP24", "LP36", "LP48"
};

void FilterSelector::setIndex(int i)
{
    index = (FilterTypeIndex)i;
}

int FilterSelector::getIndex()
{
    return (int)index;
}

void FilterSelector::setFromName(String name)
{
    for (int i = 0; i < kNumberOfFilterTypes; i++)
    {
        if (name == names[i])
        {
            index = (FilterTypeIndex)i;
            return;
        }
    }

    // Were we given an invalid FilterSelector name?
    jassertfalse;
}

String FilterSelector::name()
{
    return names[index];
}

String FilterSelector::defaultName()
{
    return names[kLP12];
}

void FilterSelector::setupComboBox(ComboBox& cb)
{
    for (int i = 0; i < kNumberOfFilterTypes; i++)
        cb.addItem(names[i], i + 1);
}

void FilterSelector::fromComboBox(ComboBox& cb)
{
    index = (FilterTypeIndex)(cb.getSelectedItemIndex());
}

void FilterSelector::toComboBox(ComboBox& cb)
{
    cb.setSelectedItemIndex((int)index);
}
