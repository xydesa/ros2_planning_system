// Copyright 2019 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "plansys2_problem_expert/ProblemExpertNode.hpp"

#include <string>
#include <memory>
#include <vector>

std::vector<std::string> tokenize(const std::string & string, const std::string & delim)
{
  std::string::size_type lastPos = 0, pos = string.find_first_of(delim, lastPos);
  std::vector<std::string> tokens;

  while (lastPos != std::string::npos) {
    if (pos != lastPos) {
      tokens.push_back(string.substr(lastPos, pos - lastPos));
    }
    lastPos = pos;
    if (lastPos == std::string::npos || lastPos + 1 == string.length()) {
      break;
    }
    pos = string.find_first_of(delim, ++lastPos);
  }

  return tokens;
}

namespace plansys2
{

ProblemExpertNode::ProblemExpertNode()
: rclcpp_lifecycle::LifecycleNode("problem_expert")
{
  declare_parameter("model_file", "");

  add_problem_goal_service_ = create_service<plansys2_msgs::srv::AddProblemGoal>(
    "problem_expert/add_problem_goal",
    std::bind(
      &ProblemExpertNode::add_problem_goal_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  add_problem_assignment_service_ = create_service<plansys2_msgs::srv::AddProblemAssignment>(
    "problem_expert/add_problem_assignment",
    std::bind(
      &ProblemExpertNode::add_problem_assignment_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  add_problem_instance_service_ = create_service<plansys2_msgs::srv::AddProblemInstance>(
    "problem_expert/add_problem_instance",
    std::bind(
      &ProblemExpertNode::add_problem_instance_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  add_problem_predicate_service_ = create_service<plansys2_msgs::srv::AddProblemPredicate>(
    "problem_expert/add_problem_predicate",
    std::bind(
      &ProblemExpertNode::add_problem_predicate_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  get_problem_goal_service_ = create_service<plansys2_msgs::srv::GetProblemGoal>(
    "problem_expert/get_problem_goal",
    std::bind(
      &ProblemExpertNode::get_problem_goal_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  get_problem_instance_details_service_ =
    create_service<plansys2_msgs::srv::GetProblemInstanceDetails>(
    "problem_expert/get_problem_instance_details",
    std::bind(
      &ProblemExpertNode::get_problem_instance_details_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  get_problem_instances_service_ = create_service<plansys2_msgs::srv::GetProblemInstances>(
    "problem_expert/get_problem_instances",
    std::bind(
      &ProblemExpertNode::get_problem_instances_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  get_problem_predicate_details_service_ =
    create_service<plansys2_msgs::srv::GetProblemPredicateDetails>(
    "problem_expert/get_problem_predicate_details", std::bind(
      &ProblemExpertNode::get_problem_predicate_details_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  get_problem_predicates_service_ = create_service<plansys2_msgs::srv::GetProblemPredicates>(
    "problem_expert/get_problem_predicates",
    std::bind(
      &ProblemExpertNode::get_problem_predicates_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  get_problem_service_ = create_service<plansys2_msgs::srv::GetProblem>(
    "problem_expert/get_problem", std::bind(
      &ProblemExpertNode::get_problem_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  remove_problem_goal_service_ = create_service<plansys2_msgs::srv::RemoveProblemGoal>(
    "problem_expert/remove_problem_goal",
    std::bind(
      &ProblemExpertNode::remove_problem_goal_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  remove_problem_instance_service_ = create_service<plansys2_msgs::srv::RemoveProblemInstance>(
    "problem_expert/remove_problem_instance",
    std::bind(
      &ProblemExpertNode::remove_problem_instance_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  remove_problem_predicate_service_ = create_service<plansys2_msgs::srv::RemoveProblemPredicate>(
    "problem_expert/remove_problem_predicate",
    std::bind(
      &ProblemExpertNode::remove_problem_predicate_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  exist_problem_predicate_service_ = create_service<plansys2_msgs::srv::ExistProblemPredicate>(
    "problem_expert/exist_problem_predicate",
    std::bind(
      &ProblemExpertNode::exist_problem_predicate_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));

  update_pub_ = create_publisher<std_msgs::msg::Empty>(
    "problem_expert/update_notify",
    rclcpp::QoS(100));

  knownledge_pub_ = create_publisher<plansys2_msgs::msg::Knownledge>(
    "problem_expert/knownledge",
    rclcpp::QoS(100).transient_local());
}


using CallbackReturnT =
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

CallbackReturnT
ProblemExpertNode::on_configure(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Configuring...", get_name());

  // (fmrico) Here we could have a discussion if we should read the domain from file or
  // from the domain_expert service. Then, we should configure first domain_expert node
  auto model_file = get_parameter("model_file").get_value<std::string>();

  auto model_files = tokenize(model_file, ":");

  std::ifstream domain_first_ifs(model_files[0]);
  std::string domain_first_str((
      std::istreambuf_iterator<char>(domain_first_ifs)),
    std::istreambuf_iterator<char>());

  auto domain_expert = std::make_shared<DomainExpert>(domain_first_str);

  for (size_t i = 1; i < model_files.size(); i++) {
    std::ifstream domain_ifs(model_files[i]);
    std::string domain_str((
        std::istreambuf_iterator<char>(domain_ifs)),
      std::istreambuf_iterator<char>());
    domain_expert->extendDomain(domain_str);
  }

  problem_expert_ = std::make_shared<ProblemExpert>(domain_expert);
  RCLCPP_INFO(get_logger(), "[%s] Configured", get_name());
  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
ProblemExpertNode::on_activate(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Activating...", get_name());
  update_pub_->on_activate();
  knownledge_pub_->on_activate();
  RCLCPP_INFO(get_logger(), "[%s] Activated", get_name());
  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
ProblemExpertNode::on_deactivate(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Deactivating...", get_name());
  update_pub_->on_deactivate();
  knownledge_pub_->on_deactivate();
  RCLCPP_INFO(get_logger(), "[%s] Deactivated", get_name());

  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
ProblemExpertNode::on_cleanup(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Cleaning up...", get_name());
  RCLCPP_INFO(get_logger(), "[%s] Cleaned up", get_name());

  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
ProblemExpertNode::on_shutdown(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Shutting down...", get_name());
  RCLCPP_INFO(get_logger(), "[%s] Shutted down", get_name());

  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
ProblemExpertNode::on_error(const rclcpp_lifecycle::State & state)
{
  RCLCPP_ERROR(get_logger(), "[%s] Error transition", get_name());

  return CallbackReturnT::SUCCESS;
}

void
ProblemExpertNode::add_problem_goal_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::AddProblemGoal::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::AddProblemGoal::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    if (request->goal != "(and )") {
      plansys2::Goal goal;
      goal.fromString(request->goal);
      response->success = problem_expert_->setGoal(goal);

      if (response->success) {
        update_pub_->publish(std_msgs::msg::Empty());
        knownledge_pub_->publish(*get_knowledge_as_msg());
      } else {
        response->error_info = "Goal not valid";
      }
    } else {
      response->success = false;
      response->error_info = "Malformed expression";
    }
  }
}

void
ProblemExpertNode::add_problem_assignment_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::AddProblemAssignment::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::AddProblemAssignment::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    plansys2::Assignment assignment;

    assignment.name = request->assignment;

    for (std::string param_name : request->arguments) {
      plansys2::Param p;
      p.name = param_name;
      p.type = "";
      assignment.parameters.push_back(p);
    }
    assignment.value = request->value;

    response->success = problem_expert_->addAssignment(assignment);

    if (response->success) {
      update_pub_->publish(std_msgs::msg::Empty());
      knownledge_pub_->publish(*get_knowledge_as_msg());
    } else {
      response->error_info = "Invalid function assignment";
    }
  }
}

void
ProblemExpertNode::add_problem_instance_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::AddProblemInstance::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::AddProblemInstance::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    plansys2::Instance instance;
    instance.name = request->instance;
    instance.type = request->type;

    response->success = problem_expert_->addInstance(instance);

    if (response->success) {
      update_pub_->publish(std_msgs::msg::Empty());
      knownledge_pub_->publish(*get_knowledge_as_msg());
    } else {
      response->error_info = "Instance not valid";
    }
  }
}

void
ProblemExpertNode::add_problem_predicate_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::AddProblemPredicate::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::AddProblemPredicate::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    plansys2::Predicate predicate;
    predicate.name = request->predicate;

    for (const auto & param_name : request->arguments) {
      plansys2::Param param;
      param.name = param_name;
      predicate.parameters.push_back(param);
    }

    response->success = problem_expert_->addPredicate(predicate);

    if (response->success) {
      update_pub_->publish(std_msgs::msg::Empty());
      knownledge_pub_->publish(*get_knowledge_as_msg());
    } else {
      response->error_info = "Predicate not valid";
    }
  }
}

void
ProblemExpertNode::get_problem_goal_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemGoal::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemGoal::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    response->success = true;
    response->goal = problem_expert_->getGoal().toString();
  }
}

void
ProblemExpertNode::get_problem_instance_details_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemInstanceDetails::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemInstanceDetails::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    auto instance = problem_expert_->getInstance(request->instance);
    if (instance) {
      response->success = true;
      response->type = instance.value().type;
    } else {
      response->success = false;
      response->error_info = "Instance not found";
    }
  }
}

void
ProblemExpertNode::get_problem_instances_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemInstances::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemInstances::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    auto instances = problem_expert_->getInstances();
    response->success = true;
    for (auto const & instance : instances) {
      response->instances.push_back(instance.name);
      response->types.push_back(instance.type);
    }
  }
}

void
ProblemExpertNode::get_problem_predicate_details_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemPredicateDetails::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemPredicateDetails::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    auto predicates = problem_expert_->getPredicates();

    bool found = false;
    int i = 0;

    while (!found && i < predicates.size()) {
      if (predicates[i].name == request->predicate) {
        found = true;
      } else {
        i++;
      }
    }

    if (found) {
      response->success = true;

      for (size_t j = 0; j < predicates[i].parameters.size(); j++) {
        response->argument_params.push_back(predicates[i].parameters[j].name);
      }
    } else {
      response->success = false;
      response->error_info = "Predicate not found";
    }
  }
}

void
ProblemExpertNode::get_problem_predicates_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemPredicates::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetProblemPredicates::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    auto predicates = problem_expert_->getPredicates();
    response->success = true;
    for (const auto & predicate : predicates) {
      response->predicates.push_back(predicate.toString());
    }
  }
}

void
ProblemExpertNode::get_problem_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetProblem::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetProblem::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    response->success = true;
    response->problem = problem_expert_->getProblem();
  }
}

void
ProblemExpertNode::remove_problem_goal_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::RemoveProblemGoal::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::RemoveProblemGoal::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    response->success = problem_expert_->clearGoal();

    if (response->success) {
      update_pub_->publish(std_msgs::msg::Empty());
      knownledge_pub_->publish(*get_knowledge_as_msg());
    } else {
      response->error_info = "Error clearing goal";
    }
  }
}

void
ProblemExpertNode::remove_problem_instance_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::RemoveProblemInstance::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::RemoveProblemInstance::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    response->success = problem_expert_->removeInstance(request->instance);

    if (response->success) {
      update_pub_->publish(std_msgs::msg::Empty());
      knownledge_pub_->publish(*get_knowledge_as_msg());
    } else {
      response->error_info = "Error removing instance";
    }
  }
}

void
ProblemExpertNode::remove_problem_predicate_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::RemoveProblemPredicate::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::RemoveProblemPredicate::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    plansys2::Predicate predicate;
    predicate.name = request->predicate;

    for (const auto & param_name : request->arguments) {
      plansys2::Param param;
      param.name = param_name;
      predicate.parameters.push_back(param);
    }

    response->success = problem_expert_->removePredicate(predicate);

    if (response->success) {
      update_pub_->publish(std_msgs::msg::Empty());
      knownledge_pub_->publish(*get_knowledge_as_msg());
    } else {
      response->error_info = "Error removing predicate";
    }
  }
}

void
ProblemExpertNode::exist_problem_predicate_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::ExistProblemPredicate::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::ExistProblemPredicate::Response> response)
{
  if (problem_expert_ == nullptr) {
    response->exist = false;
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    plansys2::Predicate predicate;
    predicate.name = request->predicate;

    for (const auto & param_name : request->arguments) {
      plansys2::Param param;
      param.name = param_name;
      predicate.parameters.push_back(param);
    }

    response->exist = problem_expert_->existPredicate(predicate);
  }
}

plansys2_msgs::msg::Knownledge::SharedPtr
ProblemExpertNode::get_knowledge_as_msg() const
{
  auto ret_msgs = std::make_shared<plansys2_msgs::msg::Knownledge>();

  for (const auto & instance : problem_expert_->getInstances()) {
    ret_msgs->instances.push_back(instance.name);
  }

  for (const auto & predicate : problem_expert_->getPredicates()) {
    ret_msgs->predicates.push_back(predicate.toString());
  }

  ret_msgs->goal = problem_expert_->getGoal().toString();

  return ret_msgs;
}

}  // namespace plansys2
