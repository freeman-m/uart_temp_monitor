#ifndef _D_PLOT_H_
#define _D_PLOT_H_

#define D_MY_PLOT_X_MIN 0
#define D_MY_PLOT_X_MAX 10
#define D_MY_PLOT_Y_MIN 0
#define D_MY_PLOT_Y_MAX 400



// 定时器结构体
typedef struct
{
    int timer_idl;
    int timer_cnt;
}timer_rt;
extern timer_rt user_timer;

#endif
