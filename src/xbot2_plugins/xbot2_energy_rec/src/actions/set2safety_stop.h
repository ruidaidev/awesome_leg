#ifndef SET2SAFETY_STOP_H
#define SET2SAFETY_STOP_H

#include <behaviortree_cpp_v3/action_node.h>
#include <xbot2/journal/journal.h>

#include <xbot2/xbot2.h>
#include <xbot2/ros/ros_support.h>

#include <cartesian_interface/sdk/rt/LockfreeBufferImpl.h>
#include <cartesian_interface/ros/RosServerClass.h>

#include <awesome_leg/SetSafetyStop.h>

using namespace XBot;

namespace BT
{

    class Set2SafetyStop : public AsyncActionNode, public Task
    { // we are working with rt plugins, so we should minimize blocking code
      // that's why we employ asynchronous action nodes

        public:

            Set2SafetyStop(const std::string& name);

        private:

            bool _verbose = false;

            std::string _name;

            int _queue_size = 1;

            std::string _async_service_pattern = "/xbotcore/async_service/";

            std::string _idler_pluginname = "idler_rt";

            std::string _stop_state_servername = "set_cmd_plugins_2safetystop";

            std::string _asynch_servicepath;

            service::Empty empty_msg;

            NodeStatus tick() override;

            awesome_leg::SetSafetyStopRequest _set2stop;
            PublisherPtr<awesome_leg::SetSafetyStopRequest> _set2stop_pub;

    };

}

#endif // SET2SAFETY_STOP_H
