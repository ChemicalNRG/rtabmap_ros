/*
Copyright (c) 2010-2016, Mathieu Labbe - IntRoLab - Universite de Sherbrooke
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Universite de Sherbrooke nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ODOMETRYROS_H_
#define ODOMETRYROS_H_

#include <ros/ros.h>
#include <nodelet/nodelet.h>

#include <tf2_ros/transform_broadcaster.h>
#include <tf/transform_listener.h>

#include <std_srvs/Empty.h>
#include <std_msgs/Header.h>
#include <sensor_msgs/Imu.h>

#include <rtabmap_ros/ResetPose.h>
#include <rtabmap/core/SensorData.h>
#include <rtabmap/core/Parameters.h>

#include <boost/thread.hpp>

namespace rtabmap {
class Odometry;
}

namespace rtabmap_ros {

class OdometryROS : public nodelet::Nodelet
{

public:
	OdometryROS(bool stereoParams, bool visParams, bool icpParams);
	virtual ~OdometryROS();

	void processData(const rtabmap::SensorData & data, const ros::Time & stamp);

	bool reset(std_srvs::Empty::Request&, std_srvs::Empty::Response&);
	bool resetToPose(rtabmap_ros::ResetPose::Request&, rtabmap_ros::ResetPose::Response&);
	bool pause(std_srvs::Empty::Request&, std_srvs::Empty::Response&);
	bool resume(std_srvs::Empty::Request&, std_srvs::Empty::Response&);
	bool setLogDebug(std_srvs::Empty::Request&, std_srvs::Empty::Response&);
	bool setLogInfo(std_srvs::Empty::Request&, std_srvs::Empty::Response&);
	bool setLogWarn(std_srvs::Empty::Request&, std_srvs::Empty::Response&);
	bool setLogError(std_srvs::Empty::Request&, std_srvs::Empty::Response&);

	const std::string & frameId() const {return frameId_;}
	const std::string & odomFrameId() const {return odomFrameId_;}
	const rtabmap::ParametersMap & parameters() const {return parameters_;}
	bool isPaused() const {return paused_;}
	rtabmap::Transform getTransform(const std::string & fromFrameId, const std::string & toFrameId, const ros::Time & stamp) const;

protected:
	void startWarningThread(const std::string & subscribedTopicsMsg, bool approxSync);
	void callbackCalled() {callbackCalled_ = true;}

	virtual void flushCallbacks() = 0;
	tf::TransformListener & tfListener() {return tfListener_;}

private:
	void warningLoop(const std::string & subscribedTopicsMsg, bool approxSync);
	virtual void onInit();
	virtual void onOdomInit() = 0;
	virtual void updateParameters(rtabmap::ParametersMap & parameters) {}

	void callbackIMU(const sensor_msgs::ImuConstPtr& msg);
	void reset(const rtabmap::Transform & pose = rtabmap::Transform());

private:
	rtabmap::Odometry * odometry_;
	boost::thread * warningThread_;
	bool callbackCalled_;

	// parameters
	std::string frameId_;
	std::string odomFrameId_;
	std::string groundTruthFrameId_;
	std::string groundTruthBaseFrameId_;
	std::string guessFrameId_;
	double guessMinTranslation_;
	double guessMinRotation_;
	double guessMinTime_;
	bool publishTf_;
	bool waitForTransform_;
	double waitForTransformDuration_;
	bool publishNullWhenLost_;
	rtabmap::ParametersMap parameters_;

	ros::Publisher odomPub_;
	ros::Publisher odomInfoPub_;
	ros::Publisher odomLocalMap_;
	ros::Publisher odomLocalScanMap_;
	ros::Publisher odomLastFrame_;
	ros::ServiceServer resetSrv_;
	ros::ServiceServer resetToPoseSrv_;
	ros::ServiceServer pauseSrv_;
	ros::ServiceServer resumeSrv_;
	ros::ServiceServer setLogDebugSrv_;
	ros::ServiceServer setLogInfoSrv_;
	ros::ServiceServer setLogWarnSrv_;
	ros::ServiceServer setLogErrorSrv_;
	tf2_ros::TransformBroadcaster tfBroadcaster_;
	tf::TransformListener tfListener_;
	ros::Subscriber imuSub_;

	bool paused_;
	int resetCountdown_;
	int resetCurrentCount_;
	bool stereoParams_;
	bool visParams_;
	bool icpParams_;
	rtabmap::Transform guess_;
	rtabmap::Transform guessPreviousPose_;
	double previousStamp_;
	double expectedUpdateRate_;
	int odomStrategy_;
	bool waitIMUToinit_;
};

}

#endif
