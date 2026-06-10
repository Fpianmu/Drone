/*
 * drone.cpp —— 无人机实体模块实现
 *
 * 详细说明见 drone.h。
 */

#include "../include/drone.h"

/*
 * 颜色映射表 —— 把 LightColor 枚举值映射为控制台颜色常量
 *
 * 下标就是 LightColor 的值，O(1) 查表。
 * 颜色常量 CON_RED 等定义在 common.h 中，本质是 Windows 控制台的
 * 前景色属性（FOREGROUND_RED | FOREGROUND_INTENSITY 等）。
 */
static const ConsoleColor g_color_table[] = {
    CON_BLACK,     // COLOR_OFF
    CON_RED,       // COLOR_RED
    CON_GREEN,     // COLOR_GREEN
    CON_BLUE,      // COLOR_BLUE
    CON_WHITE,     // COLOR_WHITE
    CON_YELLOW,    // COLOR_YELLOW
    CON_CYAN,      // COLOR_CYAN
    CON_PURPLE,    // COLOR_PURPLE
    CON_ORANGE,    // COLOR_ORANGE
};
#define COLOR_TABLE_SIZE (sizeof(g_color_table) / sizeof(g_color_table[0]))

// 把 LightColor 枚举值转为控制台颜色值
static ConsoleColor lightcolor_to_console(LightColor color)
{
    int idx = (int)color;
    if (idx < 0 || idx >= (int)COLOR_TABLE_SIZE) {
        return CON_GRAY;   // 无效颜色用灰色兜底
    }
    return g_color_table[idx];
}

/* ================================================================
 * 生命周期管理
 * ================================================================ */

Drone* drone_create(int id, float x, float y, float height)
{
    Drone* drone = (Drone*)malloc(sizeof(Drone));
    if (drone == NULL) return NULL;

    drone->id         = id;
    drone->position.x = x;
    drone->position.y = y;
    drone->height     = height;
    drone->is_active  = 1;

    // 默认白灯常亮
    drone->light.color          = COLOR_WHITE;
    drone->light.mode           = LIGHT_STEADY;
    drone->light.blink_interval = BLINK_INTERVAL_MS;
    drone->light.blink_timer    = 0;
    drone->light.is_visible     = 1;

    return drone;
}

void drone_destroy(Drone* drone)
{
    if (drone != NULL) free(drone);
}

Drone** drone_create_fleet(int count, int start_id)
{
    if (count <= 0 || count > MAX_DRONE_COUNT) return NULL;

    // 先分配指针数组
    Drone** fleet = (Drone**)malloc(sizeof(Drone*) * count);
    if (fleet == NULL) return NULL;

    // 逐架创建，初始位置放在舞台中央
    for (int i = 0; i < count; i++) {
        fleet[i] = drone_create(
            start_id + i,
            (float)(STAGE_COLS / 2),
            (float)(STAGE_ROWS / 2),
            0.0f
        );
        if (fleet[i] == NULL) {
            // 创建失败，回滚：释放已创建的部分
            for (int j = 0; j < i; j++) drone_destroy(fleet[j]);
            free(fleet);
            return NULL;
        }
    }
    return fleet;
}

void drone_destroy_fleet(Drone** fleet, int count)
{
    if (fleet == NULL) return;
    for (int i = 0; i < count; i++) drone_destroy(fleet[i]);
    free(fleet);
}

/* ================================================================
 * 位置操作
 * ================================================================ */

void drone_set_position(Drone* drone, float x, float y, float height)
{
    if (drone == NULL) return;
    drone->position.x = x;
    drone->position.y = y;
    drone->height     = height;
}

void drone_move(Drone* drone, float dx, float dy, float dh)
{
    if (drone == NULL) return;
    drone->position.x += dx;
    drone->position.y += dy;
    drone->height     += dh;
    if (drone->height < 0.0f) drone->height = 0.0f;  // 高度不能为负
}

void drone_get_display_pos(const Drone* drone, int* x, int* y)
{
    if (drone == NULL || x == NULL || y == NULL) return;
    // 四舍五入转整数坐标
    *x = (int)(drone->position.x + 0.5f);
    *y = (int)(drone->position.y + 0.5f);
}

/* ================================================================
 * 灯光操作
 * ================================================================ */

void drone_set_light_color(Drone* drone, LightColor color)
{
    if (drone == NULL) return;
    drone->light.color = color;
}

void drone_set_light_mode(Drone* drone, LightMode mode)
{
    if (drone == NULL) return;
    drone->light.mode = mode;

    // 模式切换时重置闪烁计数器和可见性
    if (mode == LIGHT_STEADY) {
        drone->light.is_visible  = 1;
        drone->light.blink_timer = 0;
    } else if (mode == LIGHT_BLINK) {
        drone->light.is_visible  = 1;   // 闪烁从亮开始
        drone->light.blink_timer = 0;
    } else {  // LIGHT_OFF
        drone->light.is_visible  = 0;
        drone->light.blink_timer = 0;
    }
}

void drone_set_blink_interval(Drone* drone, int interval_ms)
{
    if (drone == NULL) return;
    if (interval_ms < 50) interval_ms = 50;  // 太快没意义
    drone->light.blink_interval = interval_ms;
}

void drone_light_onoff(Drone* drone, int on)
{
    if (drone == NULL) return;
    if (on) {
        drone->light.mode       = LIGHT_STEADY;
        drone->light.is_visible = 1;
    } else {
        drone->light.mode       = LIGHT_OFF;
        drone->light.is_visible = 0;
    }
}

/* ================================================================
 * 状态查询 —— 驱动闪烁逻辑
 * ================================================================ */

int drone_is_active(const Drone* drone)
{
    if (drone == NULL) return 0;
    return drone->is_active;
}

/*
 * 获取本帧颜色 —— 闪烁逻辑的核心
 *
 * 每帧都会调用，传入本帧时间增量 delta_ms。
 * 闪烁模式下的计时器在这里更新：累加 delta_ms，到达 blink_interval 就翻转
 * is_visible，然后减去一个周期（保留余数，保证精确）。
 */
ConsoleColor drone_get_current_color(Drone* drone, int delta_ms)
{
    if (drone == NULL) return CON_BLACK;

    switch (drone->light.mode) {
    case LIGHT_OFF:
        return CON_BLACK;

    case LIGHT_STEADY:
        return lightcolor_to_console(drone->light.color);

    case LIGHT_BLINK:
        drone->light.blink_timer += delta_ms;
        if (drone->light.blink_timer >= drone->light.blink_interval) {
            // 满一个周期，翻转亮灭
            drone->light.is_visible = !drone->light.is_visible;
            drone->light.blink_timer -= drone->light.blink_interval;
        }
        return drone->light.is_visible
            ? lightcolor_to_console(drone->light.color)
            : CON_BLACK;

    default:
        return CON_BLACK;
    }
}

// 颜色枚举 → 中文名，供 UI 显示
const char* color_to_name(LightColor color)
{
    switch (color) {
    case COLOR_RED:    return "红色";
    case COLOR_GREEN:  return "绿色";
    case COLOR_BLUE:   return "蓝色";
    case COLOR_WHITE:  return "白色";
    case COLOR_YELLOW: return "黄色";
    case COLOR_CYAN:   return "青色";
    case COLOR_PURPLE: return "紫色";
    case COLOR_ORANGE: return "橙色";
    default:           return "未知";
    }
}
