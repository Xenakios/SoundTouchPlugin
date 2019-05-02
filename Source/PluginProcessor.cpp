/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SoundTouchPlugAudioProcessor::SoundTouchPlugAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	m_st = std::make_unique<soundtouch::SoundTouch>();
	m_par_semitones = new AudioParameterFloat{ "shift","Semitones",-24.0,24.0,-4.0 };
	addParameter(m_par_semitones);
}

SoundTouchPlugAudioProcessor::~SoundTouchPlugAudioProcessor()
{
}

//==============================================================================
const String SoundTouchPlugAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SoundTouchPlugAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SoundTouchPlugAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SoundTouchPlugAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SoundTouchPlugAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SoundTouchPlugAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SoundTouchPlugAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SoundTouchPlugAudioProcessor::setCurrentProgram (int index)
{
}

const String SoundTouchPlugAudioProcessor::getProgramName (int index)
{
    return {};
}

void SoundTouchPlugAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SoundTouchPlugAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	m_st->setChannels(2);
	m_st->setSampleRate(sampleRate);
	m_st->setPitchSemiTones(0.0);
	m_st->flush();
	m_buf.resize(2 * samplesPerBlock);
}

void SoundTouchPlugAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SoundTouchPlugAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SoundTouchPlugAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
	const int nch = 2;
	auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
	m_st->setPitchSemiTones(*m_par_semitones);
	// copy input samples in interleaved format to helper buffer
	for (int i = 0; i < nch; ++i)
		for (int j = 0; j < buffer.getNumSamples(); ++j)
			m_buf[j * nch + i] = buffer.getSample(i, j);
	m_st->putSamples(m_buf.data(), buffer.getNumSamples());
	if (m_st->numSamples() >= buffer.getNumSamples()) // does SoundTouch have enough samples ready?
	{
		m_st->receiveSamples(m_buf.data(), buffer.getNumSamples());
		// copy SoundTouch output samples to split format Juce buffer
		for (int i = 0; i < nch; ++i)
			for (int j = 0; j < buffer.getNumSamples(); ++j)
				buffer.setSample(i, j, m_buf[j * nch + i]);
	}
	else
	{
		// SoundTouch didn't have enough output samples, just output silence
		buffer.clear();
	}
}

//==============================================================================
bool SoundTouchPlugAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SoundTouchPlugAudioProcessor::createEditor()
{
	return new GenericAudioProcessorEditor(this);
	//return new SoundTouchPlugAudioProcessorEditor (*this);
}

//==============================================================================
void SoundTouchPlugAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SoundTouchPlugAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoundTouchPlugAudioProcessor();
}
