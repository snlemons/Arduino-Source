/*  Audio Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioOption_H
#define PokemonAutomation_AudioPipeline_AudioOption_H

#include "CommonFramework/Logging/LoggerQt.h"
#include "AudioInfo.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
class AudioSession;
class AudioDisplayWidget;
class AudioSelectorWidget;

// Handles the state of audio: the audio source.
// Call make_ui() to generate the UI friend class AudioSelectorWidget,
// which directly modifies AudioSelector's internal state.
// This separates state from UI.
// TODO: if needed, can add state of FFT parameters (FFT length, sliding
// window step, etc.) here.
class AudioOption{
    static const std::string JSON_INPUT_FILE;
    static const std::string JSON_INPUT_DEVICE;
    static const std::string JSON_INPUT_FORMAT;
    static const std::string JSON_OUTPUT_DEVICE;
    static const std::string JSON_AUDIO_VIS;
    static const std::string JSON_AUDIO_VOLUME;

public:
    enum class AudioDisplayType{
        NO_DISPLAY,
        FREQ_BARS,
        SPECTROGRAM
    };
    static AudioDisplayType stringToAudioDisplayType(const std::string& value);
    static std::string audioDisplayTypeToString(AudioDisplayType type);

public:
    AudioOption();
    AudioOption(const JsonValue& json);

    const std::string& input_file() const{ return m_input_file; }
    const AudioDeviceInfo& input_device() const{ return m_input_device; }
    AudioChannelFormat input_format() const{ return m_input_format; }

    const AudioDeviceInfo& output_device() const{ return m_output_device; }
    AudioDisplayType display_type() const{ return m_display_type; }
    double volume() const{ return m_volume; }

public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;

    AudioSelectorWidget* make_ui(QWidget& parent, LoggerQt& logger, AudioSession& session);

private:
    friend class AudioSession;
    std::string m_input_file;
    AudioDeviceInfo m_input_device;
    AudioChannelFormat m_input_format = AudioChannelFormat::NONE;
    AudioDeviceInfo m_output_device;
    AudioDisplayType m_display_type = AudioDisplayType::NO_DISPLAY;
    double m_volume = 1.0;  //  Volume Range: [0, 1.0]
};





}
#endif
