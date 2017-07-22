#include <rtthread.h>
#include <rtdevice.h>
#include <math.h>
#include "pwm.h"
#include "pid.h"
#include "dmp.h"
#ifdef RT_USING_ANOP
#include "anop.h"
#endif

#define PENDULUM_CYCLE      1108        /* 1108 ms, rod length: 0.307m */
#define MACHINE_HEIGTH      49          /* heigth of the machine in cm */
#define PI                  3.14159f    /* ¦° value in float type */
#define RADIAN_TO_ANGLE     180 / PI    /* convert radian to angle */

#define SAMPLE_INTERVAL     1000 / DEFAULT_MPU_HZ   /* sample interval in ms */
#define OUTPUT_LIMIT        80          /* output limit for duty ratio in % */
#define RADIUS_MIN          0.0f        /* min radius in cm */
#define RADIUS_MAX          60.0f       /* max radius in cm */
#define RADIUS_DEFAULT      30.0f       /* default radius in cm */
#define ANGLE_MIN           0.0f        /* min angle */
#define ANGLE_MAX           90.0f       /* max angle */
#define ANGLE_DEFAULT       0.0f        /* default angle */

#define INC_KP              0           /* Kp for incremental pid controller */
#define INC_KI              0           /* Ki for incremental pid controller */
#define INC_KD              0           /* Kd for incremental pid controller */

#define POS_KP              0           /* Kp for position pid controller */
#define POS_KI              0           /* Ki for position pid controller */
#define POS_KD              0           /* Kd for position pid controller */

static pid_t pid_x;
static pid_t pid_y;
static struct rt_timer timer;
static struct rt_semaphore sem;

static float radius = RADIUS_DEFAULT;
static float angle = ANGLE_DEFAULT;
static rt_tick_t t = 0;

static void swing_move(int duty_ratio_x, int duty_ratio_y)
{
    /* direction x */
    if (duty_ratio_x >= 0)
    {
        /* forward */
        pwm_set_duty_ratio(1, 0);
        pwm_set_duty_ratio(2, duty_ratio_x);
    }
    else
    {
        /* backward */
        pwm_set_duty_ratio(1, -duty_ratio_x);
        pwm_set_duty_ratio(2, 0);
    }
    
    /* direction y */
    if (duty_ratio_y >= 0)
    {
        /* forward */
        pwm_set_duty_ratio(4, 0);
        pwm_set_duty_ratio(3, duty_ratio_y);
    }
    else
    {
        /* backward */
        pwm_set_duty_ratio(4, -duty_ratio_y);
        pwm_set_duty_ratio(3, 0);
    }
}

static void swing_timer_cb(void *args)
{
    rt_enter_critical();
    t += SAMPLE_INTERVAL;
    rt_exit_critical();
    
    rt_sem_release(&sem);
}

static void swing_thread_entry(void *parameter)
{
    void (*swing_mode)(void) = (void(*)(void))parameter;
    
    /* execute the setting mode */
    swing_mode();
}

void swing_mode_1(void)
{
    struct euler_angle el;
    float duty_ratio_x;
    float duty_ratio_y;
    float theta;
    float angle_x;
    float set_pitch;
    float set_roll;
    
    t = 0;
    
    while (1)
    {
        rt_sem_take(&sem, RT_WAITING_FOREVER);
        
        /* calculate angle amplitude */
        angle_x = atan(RADIUS_DEFAULT / MACHINE_HEIGTH) * RADIAN_TO_ANGLE;
        /* calculate current target radian */
        theta = t * (2 * PI / PENDULUM_CYCLE);
        /* calculate current target angle */
        set_pitch = angle_x * sin(theta);
        set_roll  = 0.0f;
        /* fetch currnet euler angle */
        dmp_get_eulerangle(&el);
        /* calculate the output duty ratio */
        duty_ratio_x = pid_incremental_ctrl(pid_x, set_pitch, el.pitch);
        duty_ratio_y = pid_incremental_ctrl(pid_y, set_roll, el.roll);
        swing_move(duty_ratio_x, duty_ratio_y);
        
    #ifdef RT_USING_ANOP
        anop_upload_float(ANOP_FUNC_CUSTOM_1, &set_pitch, 1);
        anop_upload_float(ANOP_FUNC_CUSTOM_2, &el.pitch, 1);
        anop_upload_float(ANOP_FUNC_CUSTOM_3, &duty_ratio_x, 1);
        
//        anop_upload_float(ANOP_FUNC_CUSTOM_6, &set_roll, 1);
//        anop_upload_float(ANOP_FUNC_CUSTOM_7, &el.roll, 1);
//        anop_upload_float(ANOP_FUNC_CUSTOM_8, &duty_ratio_y, 1);
    #endif
    }
}

void swing_mode_2(void)
{
    struct euler_angle el;
    float duty_ratio_x;
    float duty_ratio_y;
    float theta;
    float angle_x;
    float set_pitch;
    float set_roll;
    
    t = 0;
    
    while (1)
    {
        rt_sem_take(&sem, RT_WAITING_FOREVER);
        
        /* calculate angle amplitude */
        angle_x = atan(radius / MACHINE_HEIGTH) * RADIAN_TO_ANGLE;
        /* calculate current target radian */
        theta = t * (2 * PI / PENDULUM_CYCLE);
        /* calculate current target angle */
        set_pitch = angle_x * sin(theta);
        set_roll  = 0.0f;
        /* fetch currnet euler angle */
        dmp_get_eulerangle(&el);
        /* calculate the output duty ratio */
        duty_ratio_x = pid_incremental_ctrl(pid_x, set_pitch, el.pitch);
        duty_ratio_y = pid_incremental_ctrl(pid_y, set_roll, el.roll);
        swing_move(duty_ratio_x, duty_ratio_y);
        
    #ifdef RT_USING_ANOP
        anop_upload_float(ANOP_FUNC_CUSTOM_1, &set_pitch, 1);
        anop_upload_float(ANOP_FUNC_CUSTOM_2, &el.pitch, 1);
        anop_upload_float(ANOP_FUNC_CUSTOM_3, &duty_ratio_x, 1);
        
//        anop_upload_float(ANOP_FUNC_CUSTOM_6, &set_roll, 1);
//        anop_upload_float(ANOP_FUNC_CUSTOM_7, &el.roll, 1);
//        anop_upload_float(ANOP_FUNC_CUSTOM_8, &duty_ratio_y, 1);
    #endif
    }
}

void swing_mode_3(void)
{
    struct euler_angle el;
    float duty_ratio_x;
    float duty_ratio_y;
    float theta;
    float angle_x;
    float angle_y;
    float set_pitch;
    float set_roll;
    
    t = 0;
    
    while (1)
    {
        rt_sem_take(&sem, RT_WAITING_FOREVER);
        
        /* calculate angle amplitude */
        angle_x = atan(radius * cos(angle) / MACHINE_HEIGTH) * RADIAN_TO_ANGLE;
        angle_y = atan(radius * sin(angle) / MACHINE_HEIGTH) * RADIAN_TO_ANGLE;
        /* calculate current target radian */
        theta = t * (2 * PI / PENDULUM_CYCLE);
        /* calculate current target angle */
        set_pitch = angle_x * sin(theta);
        set_roll  = angle_y * sin(theta);
        /* fetch currnet euler angle */
        dmp_get_eulerangle(&el);
        /* calculate the output duty ratio */
        duty_ratio_x = pid_incremental_ctrl(pid_x, set_pitch, el.pitch);
        duty_ratio_y = pid_incremental_ctrl(pid_y, set_roll, el.roll);
        swing_move(duty_ratio_x, duty_ratio_y);
        
    #ifdef RT_USING_ANOP
        anop_upload_float(ANOP_FUNC_CUSTOM_1, &set_pitch, 1);
        anop_upload_float(ANOP_FUNC_CUSTOM_2, &el.pitch, 1);
        anop_upload_float(ANOP_FUNC_CUSTOM_3, &duty_ratio_x, 1);
        
//        anop_upload_float(ANOP_FUNC_CUSTOM_6, &set_roll, 1);
//        anop_upload_float(ANOP_FUNC_CUSTOM_7, &el.roll, 1);
//        anop_upload_float(ANOP_FUNC_CUSTOM_8, &duty_ratio_y, 1);
    #endif
    }
}

void swing_mode_4(void)
{
    struct euler_angle el;
    float duty_ratio_x;
    float duty_ratio_y;
    float set_pitch;
    float set_roll;
    
    while(1)
    {
        rt_sem_take(&sem, RT_WAITING_FOREVER);
        
        /* calculate current target angle */
        set_pitch = 0.0f;
        set_roll  = 0.0f;
        /* fetch currnet euler angle */
        dmp_get_eulerangle(&el);
        /* calculate the output duty ratio */
        duty_ratio_x = pid_incremental_ctrl(pid_x, set_pitch, el.pitch);
        duty_ratio_y = pid_incremental_ctrl(pid_y, set_roll, el.roll);
        swing_move(duty_ratio_x, duty_ratio_y);
        
    #ifdef RT_USING_ANOP
        anop_upload_float(ANOP_FUNC_CUSTOM_1, &set_pitch, 1);
        anop_upload_float(ANOP_FUNC_CUSTOM_2, &el.pitch, 1);
        anop_upload_float(ANOP_FUNC_CUSTOM_3, &duty_ratio_x, 1);
        
//        anop_upload_float(ANOP_FUNC_CUSTOM_6, &set_roll, 1);
//        anop_upload_float(ANOP_FUNC_CUSTOM_7, &el.roll, 1);
//        anop_upload_float(ANOP_FUNC_CUSTOM_8, &duty_ratio_y, 1);
    #endif
    }
}

void swing_mode_5(void)
{
    while(1)
    {
        rt_sem_take(&sem, RT_WAITING_FOREVER);
    }
}

void swing_set_radius(float r)
{
    rt_enter_critical();
    if (RADIUS_MIN <= r && r <= RADIUS_MAX)
        radius = r;
    rt_exit_critical();
}

void swing_set_angle(float a)
{
    rt_enter_critical();
    if (ANGLE_MIN <= a && a <= ANGLE_MAX)
        angle = a;
    rt_exit_critical();
}

void swing_init(int mode)
{
    rt_thread_t tid;
    void (*_mode)(void);
    
    pid_x = pid_create();
    pid_y = pid_create();
    pid_set_output_limit(pid_x, -OUTPUT_LIMIT, OUTPUT_LIMIT);
    pid_set_output_limit(pid_y, -OUTPUT_LIMIT, OUTPUT_LIMIT);
    pid_config(pid_x, INC_KP, INC_KI, INC_KD);
    pid_config(pid_y, INC_KP, INC_KI, INC_KD);
    
    switch(mode)
    {
        case 1: _mode = swing_mode_1;break;
        case 2: _mode = swing_mode_2;break;
        case 3: _mode = swing_mode_3;break;
        case 4: _mode = swing_mode_4;break;
        case 5: _mode = swing_mode_5;break;
    }
    
    rt_sem_init(&sem, "swing_sem", 0, RT_IPC_FLAG_FIFO);
    tid = rt_thread_create("swing", swing_thread_entry, 
        _mode, 1024, RT_THREAD_PRIORITY_MAX/2, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    rt_timer_init(&timer, "swing_timer", swing_timer_cb, RT_NULL, 
        rt_tick_from_millisecond(SAMPLE_INTERVAL), RT_TIMER_FLAG_PERIODIC);
    rt_timer_start(&timer);
}

void swing_deinit(void)
{
    rt_thread_t tid;
    
    rt_timer_stop(&timer);
    rt_timer_detach(&timer);
    tid = rt_thread_find("swing");
    if (tid != RT_NULL)
        rt_thread_delete(tid);
    rt_sem_detach(&sem);
    
    pid_reset(pid_x);
    pid_reset(pid_y);
    pid_delete(pid_x);
    pid_delete(pid_y);
    
    swing_move(0, 0);
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void cmd_swing_radius(int argc, char *argv[])
{
    if (argc == 2)
        swing_set_radius(atof(argv[1]));
    else
        rt_kprintf("Usage: swradius <radius>\n");
}
MSH_CMD_EXPORT_ALIAS(cmd_swing_radius, swradius, set swing radius);

void cmd_swing_angle(int argc, char *argv[])
{
    if (argc == 2)
        swing_set_angle(atof(argv[1]));
    else
        rt_kprintf("Usage: swangle <angle>\n");
}
MSH_CMD_EXPORT_ALIAS(cmd_swing_angle, swangle, set swing angle);

void cmd_swing_pid(int argc, char *argv[])
{
    if (argc >= 3)
    {
        int i;
        float Kp = pid_x->Kp;
        float Ki = pid_x->Ki;
        float Kd = pid_x->Kd;

        for (i = 1; i < argc; i++)
        {
            switch (argv[i++][1])
            {
                case 'p': Kp = atof(argv[i]);break;
                case 'i': Ki = atof(argv[i]);break;
                case 'd': Kd = atof(argv[i]);break;
                default:
                    break;
            }
        }
        pid_config(pid_x, Kp, Ki, Kd);
        pid_config(pid_y, Kp, Ki, Kd);
    }
    else
        rt_kprintf("Usage: swpid [-p kp] [-i ki] [-d kd]\n");
}
MSH_CMD_EXPORT_ALIAS(cmd_swing_pid, swpid, config pid value for wind swing);

void cmd_swing_init(int argc, char *argv[])
{
    if (argc == 2)
        swing_init(atoi(argv[1]));
    else
        rt_kprintf("Usage: swinit <mode: 1~5>\n");
}
MSH_CMD_EXPORT_ALIAS(cmd_swing_init, swinit, initialize wind swing with mode);

void cmd_swing_deinit(int argc, char *argv[])
{
    swing_deinit();
}
MSH_CMD_EXPORT_ALIAS(cmd_swing_deinit, swdeinit, deinitialize wind swing);
#endif
