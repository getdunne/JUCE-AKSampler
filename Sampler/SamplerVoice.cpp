//
//  SamplerVoice.cpp
//  AudioKit Core
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//

#include "SamplerVoice.hpp"
#include <stdio.h>

namespace AudioKitCore
{
    void SamplerVoice::init(double sampleRate, SamplerVoiceParams* pTimbreParameters, SamplerModParameters* pModParameters)
    {
        VoiceBase::init(sampleRate, pTimbreParameters, pModParameters);

        filterL.init(sampleRate);
        filterR.init(sampleRate);
        ampEG.init();
        filterEG.init();
    }
    
    void SamplerVoice::start(unsigned evt, unsigned noteNum, float freqHz, float volume)
    {
        if (!pSampleBuffer) return;

        SampleBuffer* pBuf = pSampleBuffer;
        oscillator.fIndex = pSampleBuffer->fStart;
        oscillator.fIncrement = (pBuf->sampleRateHz / sampleRateHz) * (freqHz / pBuf->noteHz);
        oscillator.fIncMul = 1.0;
        oscillator.bLooping = pBuf->bLoop;
        
        ampEG.updateParams();
        ampEG.start();
        
        filterL.updateSampleRate(sampleRateHz);
        filterR.updateSampleRate(sampleRateHz);
        filterEG.updateParams();
        filterEG.start();
        
        // Call base-class version last, after rest of setup
        VoiceBase::start(evt, noteNum, freqHz, volume);
    }
    
    void SamplerVoice::restart(unsigned evt, float volume)
    {
        if (!pSampleBuffer) return;

        pNewSampleBuffer = pSampleBuffer;
        ampEG.updateParams();
        ampEG.restart();

        filterEG.updateParams();
        filterEG.start();

        VoiceBase::restart(evt, this->noteNumber, this->noteHz, volume);
    }
    
    void SamplerVoice::restart(unsigned evt, unsigned noteNum, float freqHz, float volume)
    {
        if (!pSampleBuffer) return;

        pNewSampleBuffer = pSampleBuffer;
        ampEG.updateParams();
        ampEG.restart();

        filterEG.updateParams();
        filterEG.start();

        VoiceBase::restart(evt, noteNum, freqHz, volume);
    }

    void SamplerVoice::release(unsigned evt)
    {
        SamplerVoiceParams *pVoiceParams = (SamplerVoiceParams*)pTimbreParams;

        if (!pVoiceParams->loopThruRelease) oscillator.bLooping = false;
        ampEG.release();
        filterEG.release();

        VoiceBase::release(evt);
    }
    
    void SamplerVoice::stop(unsigned evt)
    {
        // For stop, call base-class version first, so note stops right away
        VoiceBase::stop(evt);

        ampEG.reset();
        filterEG.reset();
    }
    
    bool SamplerVoice::doModulation(void)
    {
        if (ampEG.isIdle()) return true;

        SamplerVoiceParams *timbreParams = (SamplerVoiceParams*)pTimbreParams;
        SamplerModParameters *modParams = (SamplerModParameters*)pModParams;

        if (ampEG.isPreStarting())
        {
            float ampeg = ampEG.getSample();
            tempGain = modParams->masterVolume * noteVol * ampeg;
            if (!ampEG.isPreStarting())
            {
                noteVol = newNoteVol;
                tempGain = modParams->masterVolume * noteVol * ampeg;

                if (newNoteNumber >= 0)
                {
                    // restarting a "stolen" voice with a new note number
                    pSampleBuffer = pNewSampleBuffer;
                    oscillator.fIndex = pSampleBuffer->fStart;
                    oscillator.bLooping = pSampleBuffer->bLoop;
                    noteNumber = newNoteNumber;
                }
                ampEG.start();
                filterEG.start();

                pSampleBuffer = pNewSampleBuffer;
                oscillator.fIndex = pSampleBuffer->fStart;
                oscillator.bLooping = pSampleBuffer->bLoop;
            }
        }
        else
            tempGain = modParams->masterVolume * noteVol * ampEG.getSample();
        oscillator.setPitchOffsetSemitones(modParams->pitchOffset);
        
        float feg = filterEG.getSample();
        double cutoffHz = noteHz * (1.0f + modParams->cutoffMultiple + modParams->cutoffEgStrength * noteFVel * feg);
        filterL.setParams(cutoffHz, modParams->filterQ);
        filterR.setParams(cutoffHz, modParams->filterQ);

        return false;
    }
    
    bool SamplerVoice::getSamples(int nSamples, float* pOutLeft, float* pOutRight)
    {
        for (int i=0; i < nSamples; i++)
        {
            float leftSample, rightSample;
            if (oscillator.getSamplePair(pSampleBuffer, &leftSample, &rightSample, tempGain)) return true;
            *pOutLeft++ += filterL.process(leftSample);
            *pOutRight++ += filterR.process(rightSample);
        }
        return false;
    }

}
