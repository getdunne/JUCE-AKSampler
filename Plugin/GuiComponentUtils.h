//
//  GuiComponentUtils.h
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#pragma once
#include "JuceHeader.h"
#include "FilterSelector.h"

class FloatParamSlider : public Slider
{
private:
    float *pParameter;
    float scaleFactor;

public:
    FloatParamSlider(float sf = 1.0f) : Slider(), pParameter(0), scaleFactor(sf) {}

    // initial setup
    void setPointer(float* pf) { pParameter = pf; notify(); }
    void setScale(float sf) { scaleFactor = sf; }

    // gets called whenever slider value is changed by user
    virtual void valueChanged() { if (pParameter) *pParameter = (float)getValue() / scaleFactor; }

    // call this when parameter value changes, to update slider
    void notify() { if (pParameter) setValue(*pParameter * scaleFactor); }
};

class IntParamSlider : public Slider
{
private:
    int* pParameter;

public:
    IntParamSlider() : Slider(), pParameter(0) {}

    void setPointer(int* pi) { pParameter = pi; notify(); }

    virtual void valueChanged() { if (pParameter) *pParameter = (int)getValue(); }

    void notify() { if (pParameter) setValue(*pParameter); }
};

class FilterComboBox : public ComboBox
{
private:
    FilterSelector * pSelector;

public:
    FilterComboBox() : ComboBox(), pSelector(0) {}

    void setPointer(FilterSelector* pWf) { pSelector = pWf; notify(); }

    virtual void valueChanged(Value&) { if (pSelector) pSelector->fromComboBox(*this); }

    void notify() { if (pSelector) pSelector->toComboBox(*this); }
};

class MyLookAndFeel : public LookAndFeel_V4
{
public:
    void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle,
        Slider& slider) override;

    // see https://forum.juce.com/t/popupmenu-not-showing-on-auv3-plugin/17763/10
    Component* getParentComponentForMenuOptions(const PopupMenu::Options& options) override
    {
#if JUCE_IOS
        if (PluginHostType::getPluginLoadedAs() == AudioProcessor::wrapperType_AudioUnitv3)
        {
            if (options.getParentComponent() == nullptr && options.getTargetComponent() != nullptr)
                return options.getTargetComponent()->getTopLevelComponent();
        }
#endif

        return LookAndFeel_V2::getParentComponentForMenuOptions(options);
    }

};

File getSharedResourceFolder();
