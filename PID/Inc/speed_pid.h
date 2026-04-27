//
// Created by lingard on 2025/12/31.
//

#ifndef MECANUM_SPEED_PID_H
#define MECANUM_SPEED_PID_H
#pragma once
#include "pid.h"
#include "main.h"
#include "speed_pid.h"

class SpeedPID : public PidBase {
public:
    using PidBase::PidBase;
};


#ifdef __cplusplus
extern "C" {
#endif

    void speed_pid_clear();
    int16_t  speed_pid_calculate(float setpoint, float feedback, float dt);

#ifdef __cplusplus
}
#endif

#endif //MECANUM_SPEED_PID_H#endif