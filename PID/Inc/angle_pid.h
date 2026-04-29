//
// Created by lingard on 2026/1/10.
//

#ifndef MECANUM_ANGLE_PID_H
#define MECANUM_ANGLE_PID_H


#include "pid.h"
#include "main.h"


class AnglePID : public PidBase {
public:
    using PidBase::PidBase;

    void setTarget(int16_t target);

    int16_t calculate( float feedback, float dt);

    // private:
     int16_t target_angle = 0;
    // int32_t handleZeroCross(int32_t taget,int32_t feedback);
};


#ifdef __cplusplus
extern "C" {
#endif

    void angle_pid_clear();
    float  angle_pid_calculate(float setpoint, float feedback, float dt);

#ifdef __cplusplus
}
#endif

#endif //MECANUM_ANGLE_PID_H