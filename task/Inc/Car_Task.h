//
// Created by lingard on 2026/4/28.
//

#ifndef F407_CAR_CAR_TASK_H
#define F407_CAR_CAR_TASK_H
#ifdef __cplusplus
#include "TaskBase.h"
class CarTask : public TaskBase
{
public: void run() override;
};
#endif

#ifdef __cplusplus
extern "C"{
#endif
    void CarTask_Init();

#ifdef __cplusplus
}
#endif

#endif //F407_CAR_CAR_TASK_H