//
//  FilterSelector.h
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#pragma once
#include "JuceHeader.h"

class FilterSelector
{
private:
    enum FilterTypeIndex {
        kNone, kLP12, kLP24, kLP36, kLP48,
        kNumberOfFilterTypes
    } index;

public:
    FilterSelector() : index(kLP12) {}

    // initialize with 0-based index
    void setIndex(int i);
    int getIndex();

    // deserialize: set index based on given name
    void setFromName(String name);

    // convenience functions to allow selecting filter type from a juce::comboBox
    static void setupComboBox(ComboBox& cb);
    void fromComboBox(ComboBox& cb);
    void toComboBox(ComboBox& cb);

    // serialize: get human-readable name of this GFilter
    String name();
    static String defaultName();

private:
    // FilterSelector names: ordered list of string literals
    static const char* const names[];
};
