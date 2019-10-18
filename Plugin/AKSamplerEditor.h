//
//  AKSamplerEditor.h
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#pragma once

#include "JuceHeader.h"
#include "AKSamplerProcessor.h"
#include "GuiComponentUtils.h"

class AKSamplerEditor    : public AudioProcessorEditor
                        , public ChangeListener
{
public:
    AKSamplerEditor(AKSamplerProcessor&);
    ~AKSamplerEditor();

    void changeListenerCallback(ChangeBroadcaster*) override;

    void paint (Graphics&) override;
    void resized() override;

protected:
    AKSamplerProcessor& processor;

    MyLookAndFeel myLookAndFeel;

    GroupComponent grpMaster;
    Label lblMasterVol;
    FloatParamSlider slMasterVol;
    Label lblSemitoneOffset;
    IntParamSlider slSemitoneOffset;
    Label lblFineTune;
    FloatParamSlider slFineTune;

    GroupComponent grpAmpEnvelope;
    Label lblAmpAttack;
    FloatParamSlider slAmpAttack;
    Label lblAmpDecay;
    FloatParamSlider slAmpDecay;
    Label lblAmpSustain;
    FloatParamSlider slAmpSustain;
    Label lblAmpRelease;
    FloatParamSlider slAmpRelease;
    Label lblAmpVelSens;
    FloatParamSlider slAmpVelSens;

    ComboBox cbSfzSelect;
#if JUCE_IOS
    ComboBox cbBankSelect;
#else
    TextButton btnSfzFolderSelect;
#endif

    GroupComponent grpFilter;
    FilterComboBox cbFilterType;
    Label lblFltCutoff;
    FloatParamSlider slFltCutoff;
    Label lblFltResonance;
    FloatParamSlider slFltResonance;
    Label lblFltEgStrength;
    FloatParamSlider slFltEgStrength;
    Label lblFltAttack;
    FloatParamSlider slFltAttack;
    Label lblFltDecay;
    FloatParamSlider slFltDecay;
    Label lblFltSustain;
    FloatParamSlider slFltSustain;
    Label lblFltRelease;
    FloatParamSlider slFltRelease;
    Label lblFltVelSens;
    FloatParamSlider slFltVelSens;

    std::unique_ptr<FileChooser> fc;
    URL dirUrl;
    FilterSelector filterSelector;

    void sliderValueChanged(Slider*);

    void filterChanged();
    void sfzSelect();
#if JUCE_IOS
    void populateBankComboBox();
    void bankSelect();
#else
    void sfzButtonClicked();
#endif
    void populateSfzComboBox(URL dir);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AKSamplerEditor)
};
