//
// Created by lingard on 2026/4/27.
//

#ifndef F407_CAR_BALANCE_TASK_H
#define F407_CAR_BALANCE_TASK_H
#pragma once
#include "TaskBase.h"

typedef struct {
    float target_speed;
    float target_angle;
    int16_t motor_pwm_out;
} BalanceControl_t;

#ifdef __cplusplus
class BalanceTask : public TaskBase
{
public: void run() override;
};
#endif

#ifdef __cplusplus
extern "C"{
#endif
    void BalanceTask_Init();

#ifdef __cplusplus
}
#endif

#endif //F407_CAR_BALANCE_TASK_H