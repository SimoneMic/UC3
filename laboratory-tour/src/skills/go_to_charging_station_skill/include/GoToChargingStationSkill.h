/******************************************************************************
 *                                                                            *
 * Copyright (C) 2020 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

# pragma once

#include <mutex>
#include <thread>
#include <rclcpp/rclcpp.hpp>
#include "GoToChargingStationSkillSM.h"
#include <bt_interfaces/msg/action_response.hpp>
#include <navigation_interfaces/srv/go_to_poi_by_name.hpp>
#include <navigation_interfaces/srv/get_navigation_status.hpp>
#include <navigation_interfaces/srv/check_near_to_poi.hpp>
#include <bt_interfaces/srv/tick_action.hpp>

enum class Status{
	undefined,
	running,
	success,
	failure
};

class GoToChargingStationSkill
{
public:
	GoToChargingStationSkill(std::string name );
	bool start(int argc, char * argv[]);
	static void spin(std::shared_ptr<rclcpp::Node> node);
	void tick( [[maybe_unused]] const std::shared_ptr<bt_interfaces::srv::TickAction::Request> request,
			   std::shared_ptr<bt_interfaces::srv::TickAction::Response>      response);

private:
	std::shared_ptr<std::thread> m_threadSpin;
	std::shared_ptr<rclcpp::Node> m_node;
	std::mutex m_requestMutex;
	std::string m_name;
	GoToChargingStationSkillAction m_stateMachine;
	
	std::atomic<Status> m_tickResult{Status::undefined};
	rclcpp::Service<bt_interfaces::srv::TickAction>::SharedPtr m_tickService;
	
};

