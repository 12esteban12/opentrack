/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift_080.h"
#include "opentrack/plugin-api.hpp"
#include "OVR_CAPI.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_0_8_0.h"
#include <cstdio>

using namespace OVR;

Rift_Tracker::Rift_Tracker() : old_yaw(0), hmd(nullptr)
{
}

Rift_Tracker::~Rift_Tracker()
{
    if (hmd)
        ovr_Destroy(hmd);
    ovr_Shutdown();
}

void Rift_Tracker::start_tracker(QFrame*)
{
	ovrGraphicsLuid luid;
    ovrResult result = ovr_Create(&hmd, &luid);
    if (OVR_SUCCESS(result))
    {
        ovr_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, ovrTrackingCap_Orientation);
    }
    else
    {
        // XXX need change ITracker et al api to allow for failure reporting
        // this qmessagebox doesn't give any relevant details either -sh 20141012
        QMessageBox::warning(0,"Error", "Unable to start Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
    }
}

void Rift_Tracker::data(double *data)
{
    if (hmd)
    {
        ovrTrackingState ss = ovr_GetTrackingState(hmd, 0, false);
        if(ss.StatusFlags & ovrStatus_OrientationTracked) {
            auto pose = ss.HeadPose.ThePose;
            Quatf quat = pose.Orientation;
            float yaw, pitch, roll;
            quat.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
            // XXX TODO move to core
            if (s.useYawSpring)
            {
                yaw = old_yaw*s.persistence + (yaw-old_yaw);
                if(yaw > s.deadzone)
                    yaw -= s.constant_drift;
                if(yaw < -s.deadzone)
                    yaw += s.constant_drift;
                old_yaw=yaw;
            }
            constexpr double d2r = 57.295781;
            data[Yaw] = yaw * -d2r;
            data[Pitch] = pitch * d2r;
            data[Roll] = roll * d2r;
            data[TX] = pose.Position.x * -1e2;
            data[TY] = pose.Position.y *  1e2;
            data[TZ] = pose.Position.z *  1e2;
        }
    }
}

OPENTRACK_DECLARE_TRACKER(Rift_Tracker, TrackerControls, FTNoIR_TrackerDll)
