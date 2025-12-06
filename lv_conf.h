#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_USE_PERF_MONITOR     0
#define LV_USE_LOG              0
#define LV_USE_ASSERT_NULL      0
#define LV_USE_DRAW_SW          1
#define LV_USE_FS_STDIO         0
#define LV_USE_DEV_CUSTOM       0

#define LV_COLOR_DEPTH          16
#define LV_COLOR_SCREEN_TRANSP  0

#define LV_TICK_CUSTOM          0

#define LV_USE_USER_DATA        0
#define LV_USE_LAYER            0
#define LV_MEM_SIZE             (32U * 1024U)  // 32KB heap

#define LV_USE_TIMER            0
#define LV_USE_LABEL            1
#define LV_USE_BTN              0
#define LV_USE_DISPLAY          1

// Disable unused widgets
#define LV_USE_BTNMATRIX        0
#define LV_USE_SWITCH           0
#define LV_USE_SLIDER           0
#define LV_USE_ARC              0
#define LV_USE_IMG              0
#define LV_USE_CALENDAR         0
#define LV_USE_SPINBOX          0
#define LV_USE_SPINNER          0
#define LV_USE_DROPDOWN         0
#define LV_USE_TEXTAREA         0
#define LV_USE_KEYBOARD         0

#endif // LV_CONF_H
