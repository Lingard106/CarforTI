//
// Created by lingard on 2025/12/31.
//

#include "../Inc/speed_pid.h"
extern "C"
{
    static SpeedPID speed_pid(0.7f,0.01f,0.01f,5000,5000);

    void speed_pid_clear()
    {
        speed_pid.Clear();
    }

    int16_t speed_pid_calculate(float setpoint, float feedback, float dt)
    {
        float output=speed_pid.Calculate(setpoint, feedback, dt);
        return static_cast<int16_t>(output);
    }
}