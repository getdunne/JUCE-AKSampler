//
//  Sampler.cpp
//  AudioKit Core
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//

#include "Sampler.hpp"
#include <math.h>
#include <string.h>

namespace AudioKitCore {
    
    Sampler::Sampler()
    : keyMapValid(false)
    , vibratoDepth(0.0f)
    , ampVelocitySensitivity(1.0f)
    , filterVelocitySensitivity(0.0f)
    , loopThruRelease(true)
    , stoppingAllVoices(false)
    {
        voiceParams.pitchOffset = 0.0f;
        voiceParams.filterStages = 0;
        voiceParams.loopThruRelease = true;

        modParams.masterVolume = 1.0f;
        modParams.cutoffMultiple = 4.0f;
        modParams.cutoffEgStrength = 20.0f;
        modParams.filterQ = 1.0f;
        modParams.filterVel = 1.0f;

        for (int i=0; i < MAX_POLYPHONY; i++)
        {
            voice[i].ampEG.pParameters = &ampEGParams;
            voice[i].filterEG.pParameters = &filterEGParams;
        }
    }
    
    Sampler::~Sampler()
    {
    }
    
    int Sampler::init(double sampleRate)
    {
        ampEGParams.updateSampleRate((float)(sampleRate/CHUNKSIZE));
        filterEGParams.updateSampleRate((float)(sampleRate/CHUNKSIZE));
        vibratoLFO.waveTable.sinusoid();
        vibratoLFO.init(sampleRate/CHUNKSIZE, 5.0f);

        loadTestWaveform();
        buildKeyMap();

        VoicePointerArray vpa;
        for (int i = 0; i < MAX_POLYPHONY; i++)
        {
            voice[i].init(sampleRate, &voiceParams, &modParams);
            vpa.push_back(&voice[i]);
        }
        voiceManager.init(vpa, MAX_POLYPHONY, &voicePrepCallback, &renderPrepCallback, this);

        return 0;   // no error
    }
    
    void Sampler::deinit()
    {
        keyMapValid = false;
        for (KeyMappedSampleBuffer* pBuf : sampleBufferList) delete pBuf;
        sampleBufferList.clear();
        for (int i=0; i < MIDI_NOTENUMBERS; i++) keyMap[i].clear();
    }

    void Sampler::setFilterStages(int n)
    {
        for (int i = 0; i < MAX_POLYPHONY; i++) voice[i].setFilterStages(n);
    }

    // Load a single-cycle sawtooth waveform at C5 (note number 72, 523 Hz, not band-limited)
    void Sampler::loadTestWaveform()
    {
        KeyMappedSampleBuffer* pBuf = new KeyMappedSampleBuffer();
        pBuf->min_note = 0;
        pBuf->max_note = 127;
        pBuf->min_vel = 0;
        pBuf->max_vel = 127;
        sampleBufferList.push_back(pBuf);

        FunctionTable waveTable;
        waveTable.init(92);
        waveTable.sawtooth();

        pBuf->init(48000.0f, 1, 92);
        float* pData = waveTable.pWaveTable;
        for (int i = 0; i < 92; i++) pBuf->setData(i, *pData++);
        pBuf->noteNumber = 72;
        pBuf->noteHz = 523.0f;

        pBuf->bLoop = true;
    }
    
    void Sampler::loadSampleData(AKSampleDataDescriptor& sdd)
    {
        KeyMappedSampleBuffer* pBuf = new KeyMappedSampleBuffer();
        pBuf->min_note = sdd.sd.min_note;
        pBuf->max_note = sdd.sd.max_note;
        pBuf->min_vel = sdd.sd.min_vel;
        pBuf->max_vel = sdd.sd.max_vel;
        sampleBufferList.push_back(pBuf);
        
        pBuf->init(sdd.sampleRateHz, sdd.nChannels, sdd.nSamples);
        float* pData = sdd.pData;
        if (sdd.bInterleaved) for (int i=0; i < sdd.nSamples; i++)
        {
            pBuf->setData(i, *pData++);
            if (sdd.nChannels > 1) pBuf->setData(sdd.nSamples + i, *pData++);
        }
        else for (int i=0; i < sdd.nChannels * sdd.nSamples; i++)
        {
            pBuf->setData(i, *pData++);
        }
        pBuf->noteNumber = sdd.sd.noteNumber;
        pBuf->noteHz = sdd.sd.noteHz;
        
        if (sdd.sd.fStart > 0.0f) pBuf->fStart = sdd.sd.fStart;
        if (sdd.sd.fEnd > 0.0f)   pBuf->fEnd = sdd.sd.fEnd;
        
        pBuf->bLoop = sdd.sd.bLoop;
        if (pBuf->bLoop)
        {
            // fLoopStart, fLoopEnd are usually sample indices, but values 0.0-1.0
            // are interpreted as fractions of the total sample length.
            if (sdd.sd.fLoopStart > 1.0f) pBuf->fLoopStart = sdd.sd.fLoopStart;
            else pBuf->fLoopStart = pBuf->fEnd * sdd.sd.fLoopStart;
            if (sdd.sd.fLoopEnd > 1.0f) pBuf->fLoopEnd = sdd.sd.fLoopEnd;
            else pBuf->fLoopEnd = pBuf->fEnd * sdd.sd.fLoopEnd;
        }
    }
    
    KeyMappedSampleBuffer* Sampler::lookupSample(unsigned noteNumber, unsigned velocity)
    {
        if (!keyMapValid) return nullptr;

        // common case: only one sample mapped to this note - return it immediately
        if (keyMap[noteNumber].size() == 1) return keyMap[noteNumber].front();
        
        // search samples mapped to this note for best choice based on velocity
        for (KeyMappedSampleBuffer* pBuf : keyMap[noteNumber])
        {
            // if sample does not have velocity range, accept it trivially
            if (pBuf->min_vel < 0 || pBuf->max_vel < 0) return pBuf;
            
            // otherwise (common case), accept based on velocity
            if ((int)velocity >= pBuf->min_vel && (int)velocity <= pBuf->max_vel) return pBuf;
        }
        
        // return nil if no samples mapped to note (or sample velocities are invalid)
        return 0;
    }
    
    // re-compute keyMap[] so every MIDI note number is automatically mapped to the sample buffer
    // closest in pitch
    void Sampler::buildSimpleKeyMap()
    {
        // clear out the old mapping entirely
        keyMapValid = false;
        for (int i=0; i < MIDI_NOTENUMBERS; i++) keyMap[i].clear();
        
        for (int nn=0; nn < MIDI_NOTENUMBERS; nn++)
        {
            // scan loaded samples to find the minimum distance to note nn
            int minDistance = MIDI_NOTENUMBERS;
            for (KeyMappedSampleBuffer* pBuf : sampleBufferList)
            {
                int distance = abs(pBuf->noteNumber - nn);
                if (distance < minDistance)
                {
                    minDistance = distance;
                }
            }
            
            // scan again to add only samples at this distance to the list for note nn
            for (KeyMappedSampleBuffer* pBuf : sampleBufferList)
            {
                int distance = abs(pBuf->noteNumber - nn);
                if (distance == minDistance)
                {
                    keyMap[nn].push_back(pBuf);
                }
            }
        }
        keyMapValid = true;
    }
    
    // rebuild keyMap based on explicit mapping data in samples
    void Sampler::buildKeyMap(void)
    {
        // clear out the old mapping entirely
        keyMapValid = false;
        for (int i=0; i < MIDI_NOTENUMBERS; i++) keyMap[i].clear();

        for (int nn=0; nn < MIDI_NOTENUMBERS; nn++)
        {
            for (KeyMappedSampleBuffer* pBuf : sampleBufferList)
            {
                if (nn >= pBuf->min_note && nn <= pBuf->max_note)
                    keyMap[nn].push_back(pBuf);
            }
        }
        keyMapValid = true;
    }

    float Sampler::voicePrepCallback(void* thisPtr, void* voicePtr,
                                     unsigned noteNumber, unsigned velocity, float /*noteHz*/)
    {
        Sampler& self = *((Sampler*)thisPtr);
        SamplerVoice* voice = (SamplerVoice*)voicePtr;

        // assign the appropriate sample buffer, based on both note and velocity
        voice->pSampleBuffer = self.lookupSample(noteNumber, velocity);

        // compute note volume and filter-velocity multipliers, based on velocity
        float velFraction = (velocity / 127.0f);
        float noteVolume = 1.0f - (self.ampVelocitySensitivity * (1.0f - velFraction));
        float noteFVel = 1.0f - (self.filterVelocitySensitivity * (1.0f - velFraction));
        voice->noteFVel = noteFVel * noteFVel;   // filter-velocity effect is squared

        return noteVolume;
    }

    void Sampler::playNote(unsigned noteNumber, unsigned velocity, float noteHz)
    {
        voiceManager.playNote(noteNumber, velocity, noteHz);
    }

    void Sampler::stopNote(unsigned noteNumber, bool immediate)
    {
        voiceManager.stopNote(noteNumber, immediate);
    }

    void Sampler::sustainPedal(bool down)
    {
        voiceManager.sustainPedal(down);
    }

    void Sampler::stopAllVoices()
    {
        // Lock out starting any new notes, and tell Render() to stop all active notes
        stoppingAllVoices = true;

        // Wait until Render() has killed all active notes
        bool noteStillSounding = true;
        while (noteStillSounding)
        {
            noteStillSounding = false;
            for (int i=0; i < MAX_POLYPHONY; i++)
                if (voice[i].noteNumber >= 0) noteStillSounding = true;
        }
    }

    void Sampler::restartVoices()
    {
        // Allow starting new notes again
        stoppingAllVoices = false;
    }
    
    void Sampler::renderPrepCallback(void* thisPtr)
    {
        Sampler& self = *((Sampler*)thisPtr);
        self.modParams.pitchOffset = self.voiceParams.pitchOffset + self.vibratoDepth * self.vibratoLFO.getSample();
    }

    void Sampler::Render(unsigned /*channelCount*/, unsigned sampleCount, float *outBuffers[])
    {
        voiceManager.Render(sampleCount, outBuffers);
    }
}
