//
//  AKSamplerProcessor.cpp
//  JUCE-AKSampler
//
//  Created by Shane Dunne, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//
#include "AKSamplerProcessor.h"
#include "AKSamplerEditor.h"
#include <math.h>
#include "wavpack.h"

AKSamplerProcessor::AKSamplerProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    sampler1.setupForTesting();
    formatManager.registerBasicFormats();
    patchParams.setDefaultValues();
}

AKSamplerProcessor::~AKSamplerProcessor()
{
    sampler1.deinit();
}

const String AKSamplerProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AKSamplerProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AKSamplerProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AKSamplerProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AKSamplerProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AKSamplerProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AKSamplerProcessor::getCurrentProgram()
{
    return 0;
}

void AKSamplerProcessor::setCurrentProgram (int)
{
}

const String AKSamplerProcessor::getProgramName (int)
{
    return patchParams.programName;
}

void AKSamplerProcessor::changeProgramName (int, const String& newName)
{
    patchParams.programName = newName;
}

//==============================================================================
void AKSamplerProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    sampler1.init(sampleRate);
}

void AKSamplerProcessor::releaseResources()
{
    sampler1.deinit();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AKSamplerProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AKSamplerProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    // clear output buffers
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // render and handle MIDI messages
    MidiBuffer::Iterator midiIterator(midiMessages);
    midiIterator.setNextSamplePosition(0);
    int midiEventPos;
    MidiMessage m;
    const ScopedLock sl(lock);

    float *outBuffers[2];
    outBuffers[0] = buffer.getWritePointer(0);
    outBuffers[1] = buffer.getWritePointer(1);
    int nFrames = buffer.getNumSamples();
    for (int frameIndex = 0; frameIndex < nFrames; frameIndex += CHUNKSIZE)
    {
        int chunkSize = nFrames - frameIndex;
        if (chunkSize > CHUNKSIZE) chunkSize = CHUNKSIZE;

        // Any ramping parameters would be updated here...

        if (!midiIterator.getNextEvent(m, midiEventPos))
        {
            // no MIDI events: just render chunk
            sampler1.Render(totalNumOutputChannels, chunkSize, outBuffers);
            outBuffers[0] += CHUNKSIZE;
            outBuffers[1] += CHUNKSIZE;
        }
        else if (midiEventPos < frameIndex + CHUNKSIZE)
        {
            // MIDI event within this chunk: handle event first, then render chunk
            handleMidiEvent(m);
            sampler1.Render(totalNumOutputChannels, chunkSize, outBuffers);
            outBuffers[0] += CHUNKSIZE;
            outBuffers[1] += CHUNKSIZE;
        }
        else
        {
            // MIDI event after this chunk: render chunk, then handle event
            sampler1.Render(totalNumOutputChannels, chunkSize, outBuffers);
            outBuffers[0] += CHUNKSIZE;
            outBuffers[1] += CHUNKSIZE;
            handleMidiEvent(m);
        }
    }

    // handle any remaining MIDI events
    while (midiIterator.getNextEvent(m, midiEventPos)) handleMidiEvent(m);
}

#define NOTE_HZ(midiNoteNumber) ( 440.0f * pow(2.0f, ((midiNoteNumber) - 69.0f)/12.0f) )

void AKSamplerProcessor::handleMidiEvent(const MidiMessage& m)
{
    const int channel = m.getChannel();

    if (m.isNoteOn())
    {
        int smpNn = m.getNoteNumber() + patchParams.sampler.osc1.pitchOffsetSemitones;
        float fSmpHz = NOTE_HZ(smpNn + patchParams.sampler.osc1.detuneOffsetCents / 100.0f);
        int vel = int(127 * m.getFloatVelocity() + 0.5f);
        if (vel == 0)
        {
            sampler1.stopNote(smpNn, false);
        }
        else
        {
            sampler1.playNote(smpNn, vel, fSmpHz);
        }
    }
    else if (m.isNoteOff())
    {
        int smpNn = m.getNoteNumber() + patchParams.sampler.osc1.pitchOffsetSemitones;
        sampler1.stopNote(smpNn, false);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        for (unsigned nn = 0; nn < MIDI_NOTENUMBERS; nn++)
        {
            sampler1.stopNote(nn, true);
        }
    }
    else if (m.isPitchWheel())
    {
        // TODO give sampler its own bend up/down parameters
        sampler1.pitchBend(2.0f * (int(m.getPitchWheelValue()) - 8192) / 8192.0f);
    }
    else if (m.isAftertouch())
    {
        //handleAftertouch(channel, m.getNoteNumber(), m.getAfterTouchValue());
    }
    else if (m.isChannelPressure())
    {
        //handleChannelPressure(channel, m.getChannelPressureValue());
    }
    else if (m.isController())
    {
        sampler1.controller(m.getControllerNumber(), m.getControllerValue());
    }
    else if (m.isProgramChange())
    {
        //handleProgramChange(channel, m.getProgramChangeNumber());
    }
}

//==============================================================================
bool AKSamplerProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AKSamplerProcessor::createEditor()
{
    return new AKSamplerEditor (*this);
}

//==============================================================================
void AKSamplerProcessor::getStateInformation (MemoryBlock& destData)
{
    XmlElement xml = XmlElement("GuruStudio");
    patchParams.getXml(&xml);
    copyXmlToBinary(xml, destData);
}

void AKSamplerProcessor::setStateInformation (const void* data, int sizeInBytes)
{
#if 1
    std::unique_ptr<XmlElement> xml = getXmlFromBinary(data, sizeInBytes);
    //XmlElement* xpr = xml->getFirstChildElement();
    patchParams.setDefaultValues();
    patchParams.putXml(xml.get());
    parameterChanged();
#endif
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AKSamplerProcessor();
}

bool AKSamplerProcessor::loadSampleFile(AKSampleFileDescriptor& sfd)
{
    File f(sfd.path);
    std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(f));
    if (reader.get() == nullptr) return false;

    AKSampleDataDescriptor sdd;
    sdd.sd = sfd.sd;
    sdd.sampleRateHz = float(reader->sampleRate);
    sdd.nChannels = reader->numChannels;
    sdd.nSamples = int(reader->lengthInSamples);
    sdd.bInterleaved = false;
    sdd.pData = new float[sdd.nChannels * sdd.nSamples];

    float *ptrs[2] = { sdd.pData, sdd.pData + sdd.nSamples };
    if (!reader->read((int**)ptrs, sdd.nChannels, 0, int(reader->lengthInSamples), false))
        return false;
    if (!reader->usesFloatingPointData)
    {
        int *pi = (int*)sdd.pData;
        float *pf = sdd.pData;
        for (int i = 0; i < sdd.nChannels * sdd.nSamples; i++)
        {
            *pf++ = (*pi++) / (float)(0x80000000);
        }
    }

    sampler1.loadSampleData(sdd);
    delete[] sdd.pData;
    return true;
}

bool AKSamplerProcessor::loadCompressedSampleFile(AKSampleFileDescriptor& sfd)
{
    char errMsg[100];
    WavpackContext* wpc = WavpackOpenFileInput(sfd.path, errMsg, OPEN_2CH_MAX, 0);
    if (wpc == 0)
    {
        DBG("Wavpack error loading " << sfd.path << ": " << errMsg);
        //char msg[1000];
        //sprintf(msg, "Wavpack error loading %s: %s\n", sfd.path, errMsg);
        //MessageBox(0, msg, "Wavpack error", MB_OK);
        return false;
    }

    AKSampleDataDescriptor sdd;
    sdd.sd = sfd.sd;
    sdd.sampleRateHz = (float)WavpackGetSampleRate(wpc);
    sdd.nChannels = WavpackGetReducedChannels(wpc);
    sdd.nSamples = WavpackGetNumSamples(wpc);
    sdd.bInterleaved = sdd.nChannels > 1;
    sdd.pData = new float[sdd.nChannels * sdd.nSamples];

    // There are cases where loop end may be off by one
    if (sdd.sd.fLoopEnd > (float)(sdd.nSamples - 1))
        sdd.sd.fLoopEnd = (float)(sdd.nSamples - 1);

    int mode = WavpackGetMode(wpc);
    WavpackUnpackSamples(wpc, (int32_t*)sdd.pData, sdd.nSamples);
    if ((mode & MODE_FLOAT) == 0)
    {
        // convert samples to floating-point
        int bps = WavpackGetBitsPerSample(wpc);
        float scale = 1.0f / (1 << (bps - 1));
        float* pf = sdd.pData;
        int32_t* pi = (int32_t*)pf;
        for (int i = 0; i < (sdd.nSamples * sdd.nChannels); i++)
            *pf++ = scale * *pi++;
    }
    WavpackCloseFile(wpc);

    sampler1.loadSampleData(sdd);
    delete[] sdd.pData;
    return true;
}


static bool hasPrefix(char* string, const char* prefix)
{
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

bool AKSamplerProcessor::loadSfz(String folderPath, String sfzFileName)
{
    sampler1.stopAllVoices();
    sampler1.deinit();     // unload any samples already present

    char buf[1000];
    sprintf(buf, "%s%s", File::addTrailingSeparator(folderPath).toRawUTF8(), sfzFileName.toRawUTF8());

    FILE* pfile = fopen(buf, "r");
    if (!pfile) return false;

    int lokey=0, hikey=127, pitch=64, lovel, hivel;
    bool bLoop;
    float fLoopStart, fLoopEnd;
    char sampleFileName[100];
    char *p, *pp;

    while (fgets(buf, sizeof(buf), pfile))
    {
        p = buf;
        while (*p != 0 && isspace(*p)) p++;

        pp = strrchr(p, '\n');
        if (pp) *pp = 0;

        if (hasPrefix(p, "<group>"))
        {
            p += 7;
            lokey = 0;
            hikey = 127;
            pitch = 60;

            pp = strstr(p, "lokey");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) lokey = atoi(pp);
            }

            pp = strstr(p, "hikey");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) hikey = atoi(pp);
            }

            pp = strstr(p, "pitch_keycenter");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) pitch = atoi(pp);
            }
        }
        else if (hasPrefix(p, "<region>"))
        {
            p += 8;
            lovel = 0;
            hivel = 127;
            sampleFileName[0] = 0;
            bLoop = false;
            fLoopStart = 0.0f;
            fLoopEnd = 0.0f;

            pp = strstr(p, "lovel");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) lovel = atoi(pp);
            }

            pp = strstr(p, "hivel");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) hivel = atoi(pp);
            }

            pp = strstr(p, "loop_mode");
            if (pp)
            {
                bLoop = true;
            }

            pp = strstr(p, "loop_start");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) fLoopStart = (float)atof(pp);
            }

            pp = strstr(p, "loop_end");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) fLoopEnd = (float)atof(pp);
            }

            pp = strstr(p, "sample");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                while (*pp != 0 && isspace(*pp)) pp++;
                strcpy(sampleFileName, pp);
            }

            sprintf(buf, "%s%s", File::addTrailingSeparator(folderPath).toRawUTF8(), sampleFileName);
            AKSampleFileDescriptor sfd;
            sfd.path = buf;
            sfd.sd.bLoop = bLoop;
            sfd.sd.fStart = 0.0;
            sfd.sd.fLoopStart = fLoopStart;
            sfd.sd.fLoopEnd = fLoopEnd;
            sfd.sd.fEnd = 0.0f;
            sfd.sd.noteNumber = pitch;
            sfd.sd.noteHz = NOTE_HZ(sfd.sd.noteNumber);
            sfd.sd.min_note = lokey;
            sfd.sd.max_note = hikey;
            sfd.sd.min_vel = lovel;
            sfd.sd.max_vel = hivel;

            File f(buf);
            if (f.existsAsFile())
            {
                loadSampleFile(sfd);
            }
            else
            {
                char* px = strrchr(sampleFileName, '.');
                strcpy(px, ".wv");
                sprintf(buf, "%s%s", File::addTrailingSeparator(folderPath).toRawUTF8(), sampleFileName);
                loadCompressedSampleFile(sfd);
            }
        }
    }
    fclose(pfile);

    sampler1.buildKeyMap();
    sampler1.restartVoices();

    patchParams.sampler.osc1.sfzName = File::addTrailingSeparator(folderPath) + sfzFileName;
    return true;
}

void AKSamplerProcessor::parameterChanged()
{
    sampler1.setParams(patchParams.sampler);
}
