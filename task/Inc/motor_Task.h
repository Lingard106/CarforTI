//
// Created by lingard on 2026/4/24.
//

#ifndef F407_DRAFT_MOTOR_TASK_H
#define F407_DRAFT_MOTOR_TASK_H
#pragma once
#include "TaskBase.h"


#ifdef __cplusplus
class MotorTask : public TaskBase
{
public: void run() override;
};
#endif

#ifdef __cplusplus
extern "C"{
#endif
    void MotorTask_Init();

#ifdef __cplusplus
}
#endif

#endif //F407_DRAFT_MOTOR_TASK_H