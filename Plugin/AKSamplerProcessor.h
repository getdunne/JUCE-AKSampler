//
//  AKSamplerProcessor.h
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#pragma once

#include "JuceHeader.h"
#include "AKSampler.h"
#include "PatchParams.h"

class AKSamplerProcessor : public AudioProcessor
                        , public ChangeBroadcaster
{
public:
    AKSamplerProcessor();
    ~AKSamplerProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool loadSampleFile(AKSampleFileDescriptor& sfd);
    bool loadSfz(String folderPath, String sfzFileName);

public:
    // for use by editor
    PatchParams patchParams;
    AKSampler sampler1;
    void parameterChanged();

private:
    // implementation
    CriticalSection lock;
    void handleMidiEvent(const MidiMessage&);
    AudioFormatManager formatManager;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AKSamplerProcessor)
};
