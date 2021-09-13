#include "ros/ros.h"
#include <string>
#include <unistd.h>

// SUPORTED SWARM STATE:
// - HORIZONTAL_FORMATION
// - VERTICAL_FORMATION
// - NAVIGATE
// - DISABLED

// THIS FILE CONTROLS THE HIGH LEVEL BEHAVIOUR OF THE SWARM

int main(int argc, char** argv)
{
    ros::init(argc, argv, "main");
    ros::NodeHandle node_handle;

    std::string swarm_state="HORIZONTAL_FORMATION";

    ros::param::set("/swarm_state", swarm_state);

    sleep(15);

    swarm_state="VERTICAL_FORMATION";

    ros::param::set("/swarm_state", swarm_state);

    sleep(5);

    swarm_state="DISABLED";

    ros::param::set("/swarm_state", swarm_state);

    return 1;
}
