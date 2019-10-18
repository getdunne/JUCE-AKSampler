//
//  PatchParams.cpp
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#include "PatchParams.h"

void AKSamplerParams::setDefaultValues()
{
    main.masterLevel = 0.707f;
    main.pitchBendUpSemitones = 2;
    main.pitchBendDownSemitones = -2;
    main.ampVelocitySensitivity = 0.6f;
    main.filterVelocitySensitivity = 0.0f;

    osc1.sfzName = "";
    osc1.pitchOffsetSemitones = 0;
    osc1.detuneOffsetCents = 0.0f;

    ampEG.attackTimeSeconds = 0.01f;
    ampEG.decayTimeSeconds = 0.0f;
    ampEG.sustainLevel = 1.0f;
    ampEG.releaseTimeSeconds = 0.25f;

    filter.stages = 0;
    filter.cutoff = 0.0f;
    filter.resonance = 0.0f;
    filter.envAmount = 30.0f;
    filterEG.attackTimeSeconds = 0.0f;
    filterEG.decayTimeSeconds = 0.0f;
    filterEG.sustainLevel = 1.0f;
    filterEG.releaseTimeSeconds = 0.0f;
}

void AKSamplerParams::getXml(XmlElement* xml)
{
    xml->setAttribute("masterLevel", main.masterLevel);
    xml->setAttribute("pitchBendUpSemitones", main.pitchBendUpSemitones);
    xml->setAttribute("pitchBendDownSemitones", main.pitchBendDownSemitones);
    xml->setAttribute("ampVelocitySensitivity", main.ampVelocitySensitivity);
    xml->setAttribute("filterVelocitySensitivity", main.filterVelocitySensitivity);

    xml->setAttribute("osc1Sfz", osc1.sfzName);
    xml->setAttribute("osc1PitchOffsetSemitones", osc1.pitchOffsetSemitones);
    xml->setAttribute("osc1DetuneOffsetCents", osc1.detuneOffsetCents);

    xml->setAttribute("ampEgAttackTimeSeconds", ampEG.attackTimeSeconds);
    xml->setAttribute("ampEgDecayTimeSeconds", ampEG.decayTimeSeconds);
    xml->setAttribute("ampEgSustainLevel", ampEG.sustainLevel);
    xml->setAttribute("ampEgReleaseTimeSeconds", ampEG.releaseTimeSeconds);

    xml->setAttribute("filterStages", filter.stages);
    xml->setAttribute("filterCutoff", filter.cutoff);
    xml->setAttribute("filterResonance", filter.resonance);
    xml->setAttribute("filterEnvAmount", filter.envAmount);
    xml->setAttribute("filterEgAttackTimeSeconds", filterEG.attackTimeSeconds);
    xml->setAttribute("filterEgDecayTimeSeconds", filterEG.decayTimeSeconds);
    xml->setAttribute("filterEgSustainLevel", filterEG.sustainLevel);
    xml->setAttribute("filterEgReleaseTimeSeconds", filterEG.releaseTimeSeconds);
}

void AKSamplerParams::putXml(XmlElement* xml)
{
    main.masterLevel = (float)xml->getDoubleAttribute("masterLevel", 1.0);
    main.pitchBendUpSemitones = xml->getIntAttribute("pitchBendUpSemitones", 2);
    main.pitchBendDownSemitones = xml->getIntAttribute("pitchBendDownSemitones", 2);
    main.ampVelocitySensitivity = float(xml->getDoubleAttribute("ampVelocitySensitivity", 1.0));
    main.filterVelocitySensitivity = float(xml->getDoubleAttribute("filterVelocitySensitivity", 0.0));

    osc1.sfzName = xml->getStringAttribute("osc1Sfz");
    osc1.pitchOffsetSemitones = xml->getIntAttribute("osc1PitchOffsetSemitones", 0);
    osc1.detuneOffsetCents = (float)xml->getDoubleAttribute("osc1DetuneOffsetCents", 0.0);

    ampEG.attackTimeSeconds = (float)xml->getDoubleAttribute("ampEgAttackTimeSeconds", 0.1);
    ampEG.decayTimeSeconds = (float)xml->getDoubleAttribute("ampEgDecayTimeSeconds", 0.1);
    ampEG.sustainLevel = (float)xml->getDoubleAttribute("ampEgSustainLevel", 0.8);
    ampEG.releaseTimeSeconds = (float)xml->getDoubleAttribute("ampEgReleaseTimeSeconds", 0.5);

    filter.stages = xml->getIntAttribute("filterStages", 0);
    filter.cutoff = (float)xml->getDoubleAttribute("filterCutoff", 1.0);
    filter.resonance = (float)xml->getDoubleAttribute("filterResonance", 1000.0);
    filter.envAmount = (float)xml->getDoubleAttribute("filterEnvAmount", 1.0);
    filterEG.attackTimeSeconds = (float)xml->getDoubleAttribute("filterEgAttackTimeSeconds", 0.0);
    filterEG.decayTimeSeconds = (float)xml->getDoubleAttribute("filterEgDecayTimeSeconds", 0.0);
    filterEG.sustainLevel = (float)xml->getDoubleAttribute("filterEgSustainLevel", 1.0);
    filterEG.releaseTimeSeconds = (float)xml->getDoubleAttribute("filterEgReleaseTimeSeconds", 0.0);
}

void PatchParams::setDefaultValues()
{
    programName = "Default";
    sampler.setDefaultValues();
}

void PatchParams::getXml(XmlElement* xml)
{
    xml->setAttribute("name", programName);
    XmlElement* childXml = new XmlElement("sampler");
    sampler.getXml(childXml);
    xml->addChildElement(childXml);
}

void PatchParams::putXml(XmlElement* xml)
{
    programName = xml->getStringAttribute("name");
    sampler.putXml(xml->getChildByName("sampler"));
}
