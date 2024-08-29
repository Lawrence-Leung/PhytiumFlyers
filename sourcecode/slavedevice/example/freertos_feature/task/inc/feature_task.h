/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: feature_task.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for task function define
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */


#ifndef FEATURE_TASK_H
#define FEATURE_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

/* creating task */
void CreateTasks(void);
void DeleteTasks(void);

/* using parameter */
void CreateTasksForParamterTest(void);
void DeleteTasksForParamterTest(void);


/* test priority */
void CreateTasksForPriorityTest(void);
void DeleteTasksForPriorityTest(void);


/* test block state */
void CreateTasksForBlockTest(void);
void DeleteTasksForBlockTest(void);

/* tesk until delay */

void CreateTasksForDelayUntilTest(void);
void DeleteTasksForDelayUntilTest(void);

/* blocking or none */
void CreateTasksForBlockingOrNone(void);
void DeleteTasksForBlockingOrNone(void);

/* idle task */
void CreateTasksForIdleTask(void);
void DeleteTasksForForIdleTask(void);

/* change priority task */
void CreateTasksForChangePriorityTest(void);
void DeleteTasksForChangePriorityTest(void);

#ifdef __cplusplus
}
#endif

#endif // !