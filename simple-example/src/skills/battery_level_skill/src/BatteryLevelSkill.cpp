/******************************************************************************
 *                                                                            *
 * Copyright (C) 2020 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

#include "BatteryLevelSkill.h"

#include <QDebug>
#include <QTime>
#include <iostream>
#include <QStateMachine>
#include <thread>

BatteryLevelSkill::BatteryLevelSkill(std::string name ) :
        m_name(std::move(name))
{
    // m_stateMachine.setDataModel(&m_dataModel);
}


void BatteryLevelSkill::spin(std::shared_ptr<rclcpp::Node> node)
{
    rclcpp::spin(node);  
    rclcpp::shutdown();  
}


bool BatteryLevelSkill::start(int argc, char*argv[])
{

    if(!rclcpp::ok())
    {
        rclcpp::init(/*argc*/ argc, /*argv*/ argv);
    }

    m_node = rclcpp::Node::make_shared(m_name + "Skill");

    
    RCLCPP_DEBUG_STREAM(m_node->get_logger(), "BatteryLevelSkill::start");
    qInfo() << "BatteryLevelSkill::start";
    m_tickService = m_node->create_service<bt_interfaces::srv::TickCondition>(m_name + "Skill/tick",  
                                                                                std::bind(&BatteryLevelSkill::tick,
                                                                                this,
                                                                                std::placeholders::_1,
                                                                                std::placeholders::_2));
                                   
    QObject::connect(&m_timerTick, &QTimer::timeout, [&]() {
                if (m_fireTickCallTimer) {
					    m_stateMachine.submitEvent("tickCall");
					    m_fireTickCallTimer = false;
				}
            });
    m_timerTick.start();
    QObject::connect(&m_timerCallback, &QTimer::timeout,  [&]() {
                if (m_fireTopicCallbackTimer) {
					    QVariantMap data;
						data.insert("result", m_batteryValue.load());
						m_stateMachine.submitEvent("BatteryDriverCmpInterface.readLevel", data);
					    m_fireTopicCallbackTimer = false;
				}
            });
    m_timerCallback.start();

    m_stateMachine.start();
    m_threadSpin = std::make_shared<std::thread>(spin, m_node);
    m_subscription = m_node->create_subscription<sensor_msgs::msg::BatteryState>(
     "/battery_status", 10, std::bind(&BatteryLevelSkill::topic_callback, this, std::placeholders::_1));

        //qInfo() << "main thread id" << std::this_thread::get_id();


    m_stateMachine.connectToEvent("tickReturn", [this]([[maybe_unused]]const QScxmlEvent & event){
        std::string result = event.data().toMap()["result"].toString().toStdString();
        //qInfo() << "m_stateMachine.connectToEvent id" << std::this_thread::get_id();


        if (result == "SUCCESS" )
        { 
            m_tickResult.store(Status::success);
        } else if (result == "FAILURE" )
        { 
            m_tickResult.store(Status::failure);
        }
    });

    return true;
}

void BatteryLevelSkill::tick( [[maybe_unused]] const std::shared_ptr<bt_interfaces::srv::TickCondition::Request> request,
                                       std::shared_ptr<bt_interfaces::srv::TickCondition::Response>      response)
{
    std::lock_guard<std::mutex> lock(m_requestMutex);
    // RCLCPP_INFO(m_node->get_logger(), "BatteryLevelSkill::tick");
    m_fireTickCallTimer = true;
    auto message = bt_interfaces::msg::ConditionResponse();
    m_tickResult.store(Status::undefined); //here we can put a struct
        //qInfo() << "BatteryLevelSkill::tick" << std::this_thread::get_id();

    while(m_tickResult.load()== Status::undefined) 
    {
        // qInfo() <<  "active names" << m_stateMachine.activeStateNames();
        std::this_thread::sleep_for (std::chrono::milliseconds(100));
    }
    switch(m_tickResult.load()) 
    {
        case Status::failure:
            response->status.status = message.SKILL_FAILURE;
            break;
        case Status::success:
            response->status.status = message.SKILL_SUCCESS;
            break;            
    }
    // RCLCPP_INFO(m_node->get_logger(), "BatteryLevelSkill::tickDone");

    response->is_ok = true;
}



void BatteryLevelSkill::topic_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg) {
        //qInfo() << "BatteryLevelSkill::topic_callback id" << std::this_thread::get_id();
		m_batteryValue.store(msg->percentage);
		m_fireTopicCallbackTimer = true;
   // RCLCPP_INFO(m_node->get_logger(), "BatteryLevelSkill::topic_callback");
    // foreach (auto key, data.keys())
    // {
    //     qInfo()<<"-- key:"<<key<<" value:"<<data.value(key);
    // }
    

}

