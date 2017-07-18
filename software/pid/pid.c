#include <rtthread.h>
#include "pid.h"

pid_t pid_create(void)
{
    return rt_calloc(1, sizeof(struct pid));
}

void pid_delete(pid_t pid)
{
    rt_free(pid);
}

void pid_reset(pid_t pid)
{
    rt_memset(pid, 0, sizeof(struct pid));
}

void pid_config(pid_t pid, float Kp, float Ki, float Kd)
{
    pid->err = 0;
    pid->err_last = 0;
    pid->err_next = 0;
    pid->integral = 0;
    
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

void pid_set_output_limit(pid_t pid, float omin, float omax)
{
    pid->omin = omin;
    pid->omax = omax;
}

//float pid_position_ctrl(pid_t pid, float set, float actual)
//{
//    float output;

//    pid->set = set;
//    pid->actual = actual;
//    pid->err = pid->set - pid->actual;
//    /* todo: must set integral limit */
//    pid->integral += pid->err;
//    output = pid->Kp * pid->err + 
//             pid->Ki * pid->integral + 
//             pid->Kd * (pid->err - pid->err_last);
//    pid->err_last = pid->err;
//    
//    return output;
//}

float pid_incremental_ctrl(pid_t pid, float set, float actual)
{
    float output;

    pid->set = set;
    pid->actual = actual;
    pid->err = pid->set - pid->actual;
    output = pid->actual +
             pid->Kp * (pid->err - pid->err_next) + 
             pid->Ki * pid->err + 
             pid->Kd * (pid->err + pid->err_last - 2 * pid->err_last);
    /* set output limit */
    if (pid->omin != 0 && output < pid->omin)
        output = pid->omin;
    if (pid->omax != 0 && output > pid->omax)
        output = pid->omax;
    pid->err_last = pid->err_next;
    pid->err_next = pid->err;
    
    return output;
}