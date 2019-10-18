//
//  GuiComponentUtils.cpp
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#include "GuiComponentUtils.h"

// Change the look of our "rotary sliders" so they're more like traditional knobs. This code is adapted
// from the example at https://www.juce.com/doc/tutorial_look_and_feel_customisation.
void MyLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float rotaryStartAngle, const float rotaryEndAngle,
    Slider& slider)
{
    ignoreUnused(slider);

    const float radius = jmin(width / 2, height / 2) - 10.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // fill
    g.setColour(slider.isEnabled() ? Colours::steelblue : Colours::slategrey);
    g.fillEllipse(rx, ry, rw, rw);
    // outline
    g.setColour(Colours::slategrey);
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    Path p;
    const float pointerLength = radius * 0.5f;
    const float pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));

    // pointer
    g.setColour(Colours::lightblue);
    g.fillPath(p);
}

// See https://forum.juce.com/t/app-group-folder-access/23102
File getSharedResourceFolder()
{
    File bundle = File::getSpecialLocation(File::invokedExecutableFile).getParentDirectory();

    // macOS uses Contents/MacOS structure, iOS bundle structure is flat
#if JUCE_MAC
    bundle = bundle.getParentDirectory().getParentDirectory();
#endif

    // appex is in a PlugIns folder inside the parent bundle
    if (SystemStats::isRunningInAppExtensionSandbox())
        bundle = bundle.getParentDirectory().getParentDirectory();

#if JUCE_MAC
    bundle = bundle.getChildFile("Resources");
#endif

    return bundle;
}
