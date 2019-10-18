//
//  AKSamplerEditor.cpp
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#include "AKSamplerProcessor.h"
#include "AKSamplerEditor.h"

AKSamplerEditor::AKSamplerEditor (AKSamplerProcessor& p)
    : AudioProcessorEditor (&p)
    , processor (p)
    , grpMaster("grpMaster", TRANS("Master"))
    , lblMasterVol("lblMasterVol", TRANS("Level"))
    , lblSemitoneOffset("lblSemitoneOffset", TRANS("Pitch"))
    , lblFineTune("lblFineTune", TRANS("Tune"))
    , grpAmpEnvelope("grpAmpEnvelope", TRANS("Amplitude Envelope"))
    , lblAmpAttack("lblAmpAttack", TRANS("Attack"))
    , lblAmpDecay("lblAmpDecay", TRANS("Decay"))
    , lblAmpSustain("lblAmpSustain", TRANS("Sustain"))
    , lblAmpRelease("lblAmpRelease", TRANS("Release"))
    , lblAmpVelSens("lblAmpVelSens", TRANS("Vel Sens"))
#if !JUCE_IOS
    , btnSfzFolderSelect(TRANS("Folder..."))
#endif
    , grpFilter("grpFilter", TRANS("Per-Voice Filter"))
    , lblFltCutoff("lblFltCutoff", TRANS("Cutoff"))
    , lblFltResonance("lblFltResonance", TRANS("Res"))
    , lblFltEgStrength("lblFltEgStrength", TRANS("Env Amt"))
    , lblFltAttack("lblFltAttack", TRANS("Attack"))
    , lblFltDecay("lblFltDecay", TRANS("Decay"))
    , lblFltSustain("lblFltSustain", TRANS("Sustain"))
    , lblFltRelease("lblFltRelease", TRANS("Release"))
    , lblFltVelSens("lblFltVelSens", TRANS("Vel Sens"))
{
    setLookAndFeel(&myLookAndFeel);

    addAndMakeVisible(grpMaster);
    addAndMakeVisible(grpAmpEnvelope);
    addAndMakeVisible(grpFilter);

    auto initLabel = [this](Label& label)
    {
        addAndMakeVisible(label);
        label.setJustificationType(Justification::horizontallyCentred);
        label.setEditable(false, false, false);
    };
    initLabel(lblMasterVol);
    initLabel(lblSemitoneOffset);
    initLabel(lblFineTune);

    initLabel(lblAmpAttack);
    initLabel(lblAmpDecay);
    initLabel(lblAmpSustain);
    initLabel(lblAmpRelease);
    initLabel(lblAmpVelSens);

    initLabel(lblFltCutoff);
    initLabel(lblFltResonance);
    initLabel(lblFltEgStrength);
    initLabel(lblFltAttack);
    initLabel(lblFltDecay);
    initLabel(lblFltSustain);
    initLabel(lblFltRelease);
    initLabel(lblFltVelSens);

    auto initSlider = [this](Slider& slider)
    {
        addAndMakeVisible(slider);
        slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        slider.setPopupDisplayEnabled(true, true, 0);
        slider.onValueChange = [this, &slider] { sliderValueChanged(&slider); };
    };
    auto initSecondsSlider = [this](Slider& slider)
    {
        addAndMakeVisible(slider);
        slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        slider.setPopupDisplayEnabled(true, true, 0);
        slider.setRange(0, 12, 0);
        slider.setSkewFactor(0.23);
        slider.onValueChange = [this, &slider] { sliderValueChanged(&slider); };
    };

    initSlider(slMasterVol);
    slMasterVol.setRange(0, 1);
    slMasterVol.setPointer(&processor.patchParams.sampler.main.masterLevel);
    slMasterVol.setDoubleClickReturnValue(true, 0.7);

    initSlider(slSemitoneOffset);
    slSemitoneOffset.setRange(-24, +24, 1);
    slSemitoneOffset.setPointer(&processor.patchParams.sampler.osc1.pitchOffsetSemitones);
    slSemitoneOffset.setDoubleClickReturnValue(true, 0);

    initSlider(slFineTune);
    slFineTune.setRange(-50, 50);
    slFineTune.setPointer(&processor.patchParams.sampler.osc1.detuneOffsetCents);
    slFineTune.setDoubleClickReturnValue(true, 0);

    initSecondsSlider(slAmpAttack);
    slAmpAttack.setPointer(&processor.patchParams.sampler.ampEG.attackTimeSeconds);
    slAmpAttack.setDoubleClickReturnValue(true, 0);

    initSecondsSlider(slAmpDecay);
    slAmpDecay.setPointer(&processor.patchParams.sampler.ampEG.decayTimeSeconds);
    slAmpDecay.setDoubleClickReturnValue(true, 0);

    initSlider(slAmpSustain);
    slAmpSustain.setRange(0, 1);
    slAmpSustain.setPointer(&processor.patchParams.sampler.ampEG.sustainLevel);
    slAmpSustain.setDoubleClickReturnValue(true, 1);

    initSecondsSlider(slAmpRelease);
    slAmpRelease.setPointer(&processor.patchParams.sampler.ampEG.releaseTimeSeconds);
    slAmpRelease.setDoubleClickReturnValue(true, 0);

    initSlider(slAmpVelSens);
    slAmpVelSens.setRange(0, 1);
    slAmpVelSens.setSkewFactor(5.0);
    slAmpVelSens.setPointer(&processor.patchParams.sampler.main.ampVelocitySensitivity);
    slAmpVelSens.setDoubleClickReturnValue(true, 1);

#if JUCE_IOS
    addAndMakeVisible(cbBankSelect);
    populateBankComboBox();
    cbBankSelect.setEditableText(false);
    cbBankSelect.setTextWhenNothingSelected("");
    cbBankSelect.onChange = [this] { bankSelect(); };
#else
    addAndMakeVisible(btnSfzFolderSelect);
    btnSfzFolderSelect.onClick = [this] { sfzButtonClicked(); };
#endif

    addAndMakeVisible(cbSfzSelect);
    cbSfzSelect.setEditableText(false);
    cbSfzSelect.setJustificationType(Justification::centredLeft);
    cbSfzSelect.setTextWhenNothingSelected("");
    cbSfzSelect.setTextWhenNoChoicesAvailable(TRANS("(select a bank at left)"));
    cbSfzSelect.onChange = [this] { sfzSelect(); };

    auto initFilterCombo = [this](FilterComboBox& combo)
    {
        addAndMakeVisible(combo);
        combo.setEditableText(false);
        combo.setJustificationType(Justification::centredLeft);
        combo.setTextWhenNothingSelected("");
        combo.setTextWhenNoChoicesAvailable(TRANS("(no choices)"));
        FilterSelector::setupComboBox(combo);
        combo.onChange = [this] { filterChanged(); };
    };
    initFilterCombo(cbFilterType);
    filterSelector.setIndex(processor.patchParams.sampler.filter.stages);
    cbFilterType.setPointer(&filterSelector);

    initSlider(slFltCutoff);
    slFltCutoff.setRange(0, 100);
    slFltCutoff.setPointer(&processor.patchParams.sampler.filter.cutoff);
    slFltCutoff.setDoubleClickReturnValue(true, 100);

    initSlider(slFltResonance);
    slFltResonance.setRange(0, +30);
    slFltResonance.setPointer(&processor.patchParams.sampler.filter.resonance);
    slFltResonance.setDoubleClickReturnValue(true, 0);

    initSlider(slFltEgStrength);
    slFltEgStrength.setRange(0, 100);
    slFltEgStrength.setPointer(&processor.patchParams.sampler.filter.envAmount);
    slFltEgStrength.setDoubleClickReturnValue(true, 0);

    initSecondsSlider(slFltAttack);
    slFltAttack.setPointer(&processor.patchParams.sampler.filterEG.attackTimeSeconds);
    slFltAttack.setDoubleClickReturnValue(true, 0);

    initSecondsSlider(slFltDecay);
    slFltDecay.setPointer(&processor.patchParams.sampler.filterEG.decayTimeSeconds);
    slFltDecay.setDoubleClickReturnValue(true, 0);

    initSlider(slFltSustain);
    slFltSustain.setRange(0, 1);
    slFltSustain.setPointer(&processor.patchParams.sampler.filterEG.sustainLevel);
    slFltSustain.setDoubleClickReturnValue(true, 1);

    initSecondsSlider(slFltRelease);
    slFltRelease.setPointer(&processor.patchParams.sampler.filterEG.releaseTimeSeconds);
    slFltRelease.setDoubleClickReturnValue(true, 0);

    initSlider(slFltVelSens);
    slFltVelSens.setRange(0, 1);
    slFltVelSens.setSkewFactor(4.0f);
    slFltVelSens.setPointer(&processor.patchParams.sampler.main.filterVelocitySensitivity);
    slFltVelSens.setDoubleClickReturnValue(true, 0);

    const int windowWidth = 560;
    const int windowHeight = 300;
    setSize(windowWidth, windowHeight);    // note this does nothing on mobile platforms
}

AKSamplerEditor::~AKSamplerEditor()
{
    setLookAndFeel(nullptr);
}

void AKSamplerEditor::paint (Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void AKSamplerEditor::resized()
{
    const int gutter = 8;
    const int nudge = 2;
    const int topOffset = 10;
    const int masterGroupWidth = 200;
    const int ampGroupWidth = 340;
    const int rowHeight = 120;
    const int knobWidth = 60;
    const int cbWidth = 100;
    const int cbHeight = 18;
    const int sfzSelectHeight = 40;
#if !JUCE_IOS
    const int sfzButtonWidth = 80;
#endif
    const int sfzSelectSqueeze = 4;

    auto area = getLocalBounds();
    area.reduce(gutter, gutter);
    auto rowArea = area.removeFromTop(rowHeight);

    auto groupArea = rowArea.removeFromLeft(masterGroupWidth);
    grpMaster.setBounds(groupArea);
    groupArea.removeFromTop(topOffset);
    groupArea.reduce(gutter, gutter);
    auto knobArea = groupArea.removeFromLeft(knobWidth);
    slMasterVol.setBounds(knobArea.removeFromTop(knobWidth));
    lblMasterVol.setBounds(knobArea);
    knobArea = groupArea.removeFromLeft(knobWidth);
    slSemitoneOffset.setBounds(knobArea.removeFromTop(knobWidth));
    lblSemitoneOffset.setBounds(knobArea);
    knobArea = groupArea.removeFromLeft(knobWidth);
    slFineTune.setBounds(knobArea.removeFromTop(knobWidth));
    lblFineTune.setBounds(knobArea);

    groupArea = rowArea.removeFromRight(ampGroupWidth);
    grpAmpEnvelope.setBounds(groupArea);
    groupArea.removeFromTop(topOffset);
    groupArea.reduce(gutter, gutter);

    knobArea = groupArea.removeFromRight(knobWidth);
    slAmpVelSens.setBounds(knobArea.removeFromTop(knobWidth));
    lblAmpVelSens.setBounds(knobArea);
    groupArea.removeFromRight(2 * gutter);

    knobArea = groupArea.removeFromRight(knobWidth);
    slAmpRelease.setBounds(knobArea.removeFromTop(knobWidth));
    lblAmpRelease.setBounds(knobArea);
    knobArea = groupArea.removeFromRight(knobWidth);
    slAmpSustain.setBounds(knobArea.removeFromTop(knobWidth));
    lblAmpSustain.setBounds(knobArea);
    knobArea = groupArea.removeFromRight(knobWidth);
    slAmpDecay.setBounds(knobArea.removeFromTop(knobWidth));
    lblAmpDecay.setBounds(knobArea);
    knobArea = groupArea.removeFromRight(knobWidth);
    slAmpAttack.setBounds(knobArea.removeFromTop(knobWidth));
    lblAmpAttack.setBounds(knobArea);

    groupArea = area.removeFromTop(sfzSelectHeight);
    groupArea.reduce(sfzSelectSqueeze, gutter);
#if JUCE_IOS
    cbBankSelect.setBounds(groupArea.removeFromLeft((groupArea.getWidth() - gutter) / 2));
    groupArea.removeFromLeft(gutter / 2);
#else
    btnSfzFolderSelect.setBounds(groupArea.removeFromLeft(sfzButtonWidth));
    groupArea.removeFromLeft(gutter);
#endif
    cbSfzSelect.setBounds(groupArea);

    groupArea = area.removeFromBottom(rowHeight);
    grpFilter.setBounds(groupArea);
    auto cbArea = groupArea;
    cbArea.setY(cbArea.getY() - nudge);
    cbArea.removeFromLeft(130);
    cbArea = cbArea.removeFromLeft(cbWidth + 2 * gutter);
    cbFilterType.setBounds(cbArea.removeFromTop(cbHeight).removeFromLeft(cbWidth));
    groupArea.removeFromTop(topOffset);
    groupArea.reduce(gutter, gutter);

    knobArea = groupArea.removeFromLeft(knobWidth);
    slFltCutoff.setBounds(knobArea.removeFromTop(knobWidth));
    lblFltCutoff.setBounds(knobArea);
    knobArea = groupArea.removeFromLeft(knobWidth);
    slFltResonance.setBounds(knobArea.removeFromTop(knobWidth));
    lblFltResonance.setBounds(knobArea);
    knobArea = groupArea.removeFromLeft(knobWidth);
    slFltEgStrength.setBounds(knobArea.removeFromTop(knobWidth));
    lblFltEgStrength.setBounds(knobArea);

    knobArea = groupArea.removeFromRight(knobWidth);
    slFltVelSens.setBounds(knobArea.removeFromTop(knobWidth));
    lblFltVelSens.setBounds(knobArea);
    groupArea.removeFromRight(2 * gutter);

    knobArea = groupArea.removeFromRight(knobWidth);
    slFltRelease.setBounds(knobArea.removeFromTop(knobWidth));
    lblFltRelease.setBounds(knobArea);
    knobArea = groupArea.removeFromRight(knobWidth);
    slFltSustain.setBounds(knobArea.removeFromTop(knobWidth));
    lblFltSustain.setBounds(knobArea);
    knobArea = groupArea.removeFromRight(knobWidth);
    slFltDecay.setBounds(knobArea.removeFromTop(knobWidth));
    lblFltDecay.setBounds(knobArea);
    knobArea = groupArea.removeFromRight(knobWidth);
    slFltAttack.setBounds(knobArea.removeFromTop(knobWidth));
    lblFltAttack.setBounds(knobArea);
}

void AKSamplerEditor::changeListenerCallback(ChangeBroadcaster*)
{
    slMasterVol.notify();
    slSemitoneOffset.notify();
    slFineTune.notify();

    slAmpAttack.notify();
    slAmpDecay.notify();
    slAmpSustain.notify();
    slAmpRelease.notify();
    slAmpVelSens.notify();

    for (int i = 0; i < cbSfzSelect.getNumItems(); i++)
    {
        if (cbSfzSelect.getItemText(i) == processor.patchParams.sampler.osc1.sfzName)
        {
            cbSfzSelect.setSelectedItemIndex(i);
            break;
        }
    }

    cbFilterType.notify();
    slFltCutoff.notify();
    slFltResonance.notify();
    slFltEgStrength.notify();
    slFltAttack.notify();
    slFltDecay.notify();
    slFltSustain.notify();
    slFltRelease.notify();
    slFltVelSens.notify();
}

void AKSamplerEditor::sliderValueChanged(Slider*)
{
    processor.parameterChanged();
}

void AKSamplerEditor::filterChanged()
{
    int stages = filterSelector.getIndex();
    bool on = stages > 0;
    processor.patchParams.sampler.filter.stages = stages;

    slFltCutoff.setEnabled(on);
    slFltResonance.setEnabled(on);
    slFltEgStrength.setEnabled(on);
    slFltAttack.setEnabled(on);
    slFltDecay.setEnabled(on);
    slFltSustain.setEnabled(on);
    slFltRelease.setEnabled(on);
    slFltVelSens.setEnabled(on);

    processor.parameterChanged();
}

void AKSamplerEditor::sfzSelect()
{
    processor.loadSfz(dirUrl.getLocalFile().getFullPathName(), cbSfzSelect.getText() + ".sfz");
}

#if JUCE_IOS
void AKSamplerEditor::populateBankComboBox()
{
    auto docsDir = getSharedResourceFolder().getChildFile("Sounds");
    cbBankSelect.clear();

    StringArray items;
    Array<File> subdirs;
    docsDir.findChildFiles(subdirs, File::findDirectories, false);
    for (int i = 0; i < subdirs.size(); i++)
    {
        File dir = subdirs.getReference(i);
        items.add(dir.getFileName());
    }

    items.sort(true);

    int count = 0;
    for (auto item : items) cbBankSelect.addItem(item, ++count);
}

void AKSamplerEditor::bankSelect()
{
    auto docsDir = getSharedResourceFolder().getChildFile("Sounds");
    dirUrl = URL(docsDir.getChildFile(cbBankSelect.getText()));
    populateSfzComboBox(dirUrl);
}
#else
void AKSamplerEditor::sfzButtonClicked()
{
    fc.reset(new FileChooser("Choose a directory...", File::getCurrentWorkingDirectory(), "*", true));
    fc->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories,
        [this](const FileChooser& chooser)
    {
        auto result = chooser.getURLResult();
        if (result.isLocalFile())
        {
            this->populateSfzComboBox(result);
        }
    });
}
#endif

void AKSamplerEditor::populateSfzComboBox(URL dir)
{
    dirUrl = dir;
    cbSfzSelect.clear();

    StringArray items;

    DirectoryIterator iter(dir.getLocalFile(), true);
    while (iter.next())
    {
        if (iter.getFile().hasFileExtension("sfz"))
            items.add(iter.getFile().getFileNameWithoutExtension());
    }

    items.sort(true);

    int count = 0;
    for (auto item : items)
        cbSfzSelect.addItem(item, ++count);
}
