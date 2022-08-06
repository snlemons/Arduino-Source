/*  Audio Passthrough Pair (Qt separate thread)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      Same as AudioPassthroughPairQt, but with the audio running
 *  on a separate thread to avoid UI thread noise.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioPassthroughPairQtThread_H
#define PokemonAutomation_AudioPipeline_AudioPassthroughPairQtThread_H

#include <QThread>
#include "CommonFramework/AudioPipeline/AudioPassthroughPair.h"

namespace PokemonAutomation{

class Logger;
class AudioPassthroughPairQt;


class AudioPassthroughPairQtThread : private QThread, public AudioPassthroughPair{
public:
    virtual void add_listener(FFTListener& listener) override;
    virtual void remove_listener(FFTListener& listener) override;

public:
    AudioPassthroughPairQtThread(Logger& logger);
    ~AudioPassthroughPairQtThread();

    virtual void reset(
        const std::string& file,
        const AudioDeviceInfo& output, float volume
    ) override;
    virtual void reset(
        const AudioDeviceInfo& input, AudioChannelFormat format,
        const AudioDeviceInfo& output, float volume
    ) override;

    virtual void clear_audio_source() override;
    virtual void set_audio_source(const std::string& file) override;
    virtual void set_audio_source(const AudioDeviceInfo& device, AudioChannelFormat format) override;

    virtual void clear_audio_sink() override;
    virtual void set_audio_sink(const AudioDeviceInfo& device, float volume) override;

    virtual void set_sink_volume(double volume) override;

private:
    virtual void run() override;

private:
    Logger& m_logger;
    std::atomic<AudioPassthroughPairQt*> m_body;
};




}
#endif
