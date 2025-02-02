/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

#ifndef DIALOG_COMPONENT__HPP
#define DIALOG_COMPONENT__HPP

#include <mutex>
#include <thread>
#include <rclcpp/rclcpp.hpp>
#include <yarp/os/Network.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/ISpeechSynthesizer.h>
//#include <yarp/dev/IChatBot.h>
#include <yarp/dev/ILLM.h>
#include <yarp/dev/IAudioGrabberSound.h>
#include <dialog_interfaces/srv/get_language.hpp>
#include <dialog_interfaces/srv/set_language.hpp>
#include <dialog_interfaces/srv/enable_dialog.hpp>
#include <dialog_interfaces/srv/set_poi.hpp>
#include <dialog_interfaces/srv/get_state.hpp>
#include <yarp/dev/AudioPlayerStatus.h>

#include "nlohmann/json.hpp"
#include <random>

#include "SpeechTranscriberCallback.hpp"
#include "SpeakerStatusCallback.hpp"
#include "TourStorage.h"

class DialogComponent 
{
public:
    DialogComponent();

    bool start(int argc, char*argv[]);
    bool close();
    void spin();
    bool ConfigureYARP(yarp::os::ResourceFinder &rf);

    void GetLanguage([[maybe_unused]] const std::shared_ptr<dialog_interfaces::srv::GetLanguage::Request> request,
                        std::shared_ptr<dialog_interfaces::srv::GetLanguage::Response> response);
    void SetLanguage(const std::shared_ptr<dialog_interfaces::srv::SetLanguage::Request> request,
                        std::shared_ptr<dialog_interfaces::srv::SetLanguage::Response> response);
    void EnableDialog(const std::shared_ptr<dialog_interfaces::srv::EnableDialog::Request> request,
                        std::shared_ptr<dialog_interfaces::srv::EnableDialog::Response> response);
    void SetPoi(const std::shared_ptr<dialog_interfaces::srv::SetPoi::Request> request,
                        std::shared_ptr<dialog_interfaces::srv::SetPoi::Response> response);
    void GetState(const std::shared_ptr<dialog_interfaces::srv::GetState::Request> request,
                        std::shared_ptr<dialog_interfaces::srv::GetState::Response> response);

private:
    /*Network Wrappers*/
    // Speech Synth
    yarp::dev::PolyDriver m_speechSynthPoly;
    yarp::dev::ISpeechSynthesizer *m_iSpeechSynth{nullptr};
    // Dialog Flow
    //yarp::dev::PolyDriver m_chatBotPoly;
    //yarp::dev::IChatBot *m_iChatBot{nullptr};
    // ChatGPT
    yarp::dev::PolyDriver m_chatGPTPoly;
    yarp::dev::ILLM *m_iChatGPT{nullptr};
    // Microphone
    yarp::dev::PolyDriver m_audioRecorderPoly;
    yarp::dev::IAudioGrabberSound *m_iAudioGrabberSound{nullptr};

    // Callback on SpeechTranscriber port
    SpeechTranscriberCallback m_speechTranscriberCallback;
    std::string m_speechTranscriberClientName;
    std::string m_speechTranscriberServerName;
    yarp::os::BufferedPort<yarp::os::Bottle> m_speechTranscriberPort;

    /*Speakers*/
    yarp::os::BufferedPort<yarp::sig::Sound> m_speakersAudioPort;
    yarp::os::BufferedPort<yarp::dev::AudioPlayerStatus> m_speakersStatusPort;
    SpeakerStatusCallback m_speakerCallback;

    /*ROS2*/
    rclcpp::Node::SharedPtr m_node;
    rclcpp::Service<dialog_interfaces::srv::GetLanguage>::SharedPtr m_getLanguageService;
    rclcpp::Service<dialog_interfaces::srv::SetLanguage>::SharedPtr m_setLanguageService;
    rclcpp::Service<dialog_interfaces::srv::EnableDialog>::SharedPtr m_enableDialogService;
    rclcpp::Service<dialog_interfaces::srv::SetPoi>::SharedPtr m_setPoiService;
    rclcpp::Service<dialog_interfaces::srv::GetState>::SharedPtr m_GetStateService;
    std::mutex m_mutex;

    /*Dialog JSON*/
    std::shared_ptr<TourStorage> m_tourStorage;
    std::string m_currentPoiName;
    std::string m_jsonPath;
    std::string m_tourName;
    bool m_tourLoadedAtStart;

    /*Answers Randomizer*/
    std::mt19937 m_random_gen;
    std::default_random_engine m_rand_engine;
    std::uniform_int_distribution<std::mt19937::result_type> m_uniform_distrib;
    int m_fallback_threshold;
    int m_fallback_repeat_counter;
    std::string m_last_valid_speak;

    /*Status functions*/
    /*bool isSpeaking(bool &result);
    bool isAudioEnabled(bool &result);*/

    /*Threading*/
    std::thread m_dialogThread;
    void DialogExecution();
    bool CommandManager(const std::string &command, PoI currentPoI, PoI genericPoI, std::string & phrase);
    bool InterpretCommand(const std::string &command, PoI currentPoI, PoI genericPoI, std::string & phrase);
    bool m_exit;

    /* Internal State Machine*/
    enum State {
        IDLE = 0,
        RUNNING = 1,
        SUCCESS = 2,
        FAILURE = 3
    };

    State m_state;
};

#endif
