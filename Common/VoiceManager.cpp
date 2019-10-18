//
//  VoiceManager.cpp
//  AudioKit Core
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//

#include "VoiceManager.hpp"

namespace AudioKitCore {
    
    VoiceManager::VoiceManager()
    : nCurrentPolyphony(0)
    , eventCounter(0)
    , doRenderPrep(0)
    {
    }
    
    VoiceManager::~VoiceManager()
    {
        voice.clear();
    }
    
    int VoiceManager::init(VoicePointerArray voiceArray, int polyphony,
                           VoicePrepCallback voicePrepCallback,
                           RenderPrepCallback renderPrepCallback,
                           void* callbackPtr)
    {
        voice = voiceArray;
        setPolyphony(polyphony);
        for (int i = 0; i < nCurrentPolyphony; i++)
        {
            voice[i]->event = 0;
            voice[i]->noteNumber = -1;
        }

        memset(keyIsDown, 0, sizeof(keyIsDown));
        pedalIsDown = false;

        doVoicePrep = voicePrepCallback;
        doRenderPrep = renderPrepCallback;
        cbPtr = callbackPtr;

        eventCounter = 0;
        return 0;   // no error
    }
    
    void VoiceManager::deinit()
    {
    }

    bool VoiceManager::setPolyphony(int polyphony)
    {
        int nVoices = polyphony;
        if (nVoices < 1) nVoices = 1;
        if (nVoices > (int)voice.size()) nVoices = (int)voice.size();
        nCurrentPolyphony = nVoices;
        return nVoices == polyphony;    // return true only if we got exactly what was requested
    }

    void VoiceManager::playNote(unsigned noteNumber, unsigned velocity, float noteHz)
    {
        eventCounter++;
        keyIsDown[noteNumber] = true;
        play(noteNumber, velocity, noteHz);
    }
    
    void VoiceManager::stopNote(unsigned noteNumber, bool immediate)
    {
        eventCounter++;
        keyIsDown[noteNumber] = false;
        if (!pedalIsDown)
            stop(noteNumber, immediate);
    }
    
    void VoiceManager::sustainPedal(bool down)
    {
        eventCounter++;
        if (down && !pedalIsDown)
        {
            // pedal is now down
            pedalIsDown = true;
        }
        else if (!down && pedalIsDown)
        {
            // pedal has just come up: release all notes except those whose key is down
            for (int i = 0; i < nCurrentPolyphony; i++)
            {
                VoiceBase* pVoice = voice[i];
                int noteNumber = pVoice->noteNumber;
                if (!keyIsDown[noteNumber])
                    pVoice->release(eventCounter);
            }
            pedalIsDown = false;
        }
    }

    void VoiceManager::play(unsigned noteNumber, unsigned velocity, float noteHz)
    {
        //printf("playNote nn=%d vel=%d %.2f Hz\n", noteNumber, velocity, noteHz);
        
        // find a free voice (with noteNumber < 0) to play the note
        for (int i=0; i < nCurrentPolyphony; i++)
        {
            VoiceBase* pVoice = voice[i];
            if (pVoice->noteNumber < 0)
            {
                // found a free voice: assign it to play this note
                float noteVolume = doVoicePrep(cbPtr, pVoice, noteNumber, velocity, noteHz);
                pVoice->start(eventCounter, noteNumber, noteHz, noteVolume);
                //printf("Play note %d (%.2f Hz) vel %d\n", noteNumber, noteHz, velocity);
                return;
            }
        }
        
        // all oscillators in use: find "stalest" voice to steal
        unsigned greatestDiffOfAll = 0;
        VoiceBase* pStalestVoiceOfAll = 0;
        unsigned greatestDiffInRelease = 0;
        VoiceBase* pStalestVoiceInRelease = 0;
        for (int i=0; i < nCurrentPolyphony; i++)
        {
            VoiceBase* pVoice = voice[i];
            unsigned diff = eventCounter - pVoice->event;
            if (pVoice->isReleasing())
            {
                if (diff > greatestDiffInRelease)
                {
                    greatestDiffInRelease = diff;
                    pStalestVoiceInRelease = pVoice;
                }
            }
            if (diff > greatestDiffOfAll)
            {
                greatestDiffOfAll = diff;
                pStalestVoiceOfAll = pVoice;
            }
        }

        if (pStalestVoiceInRelease != 0)
        {
            // We have a stalest note in its release phase: restart that one
            doVoicePrep(cbPtr, pStalestVoiceInRelease, noteNumber, velocity, noteHz);
            pStalestVoiceInRelease->restart(eventCounter, noteNumber, noteHz, pStalestVoiceInRelease->noteVol);
        }
        else
        {
            // No notes in release phase: restart the "stalest" one we could find
            doVoicePrep(cbPtr, pStalestVoiceOfAll, noteNumber, velocity, noteHz);
            pStalestVoiceOfAll->restart(eventCounter, noteNumber, noteHz, pStalestVoiceOfAll->noteVol);
        }
    }
    
    void VoiceManager::stop(unsigned noteNumber, bool immediate)
    {
        // release ALL voices playing given note number
        for (int i = 0; i < nCurrentPolyphony; i++)
        {
            VoiceBase* pVoice = voice[i];
            if (pVoice->noteNumber == (int)noteNumber)
            {
                if (immediate)
                    pVoice->stop(eventCounter);
                else if (!pVoice->isReleasing())
                    pVoice->release(eventCounter);
            }
        }
    }

    void VoiceManager::stopAll(void)
    {
        for (int i = 0; i < nCurrentPolyphony; i++)
            voice[i]->stop(eventCounter);
    }
    
    void VoiceManager::Render(unsigned sampleCount, float *outBuffers[])
    {
        if (!doRenderPrep) return;

        float* pOutLeft = outBuffers[0];
        float* pOutRight = outBuffers[1];
        
        if (doRenderPrep) doRenderPrep(cbPtr);

        for (int i=0; i < nCurrentPolyphony; i++)
        {
            VoiceBase* pVoice = voice[i];
            int nn = pVoice->noteNumber;
            if (nn >= 0)
            {
                if (pVoice->doModulation() || pVoice->getSamples(sampleCount, pOutLeft, pOutRight))
                {
                    pVoice->stop(eventCounter);
                }
            }
        }
    }
}
