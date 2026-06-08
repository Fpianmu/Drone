/**
 * @file    file_io.h
 * @brief   文件读写模块 —— 轨迹数据持久化与回放
 * @author  [队友名字]
 * @date    2026-06-08
 *
 * 本模块负责将无人机轨迹数据保存到文件，以及从文件加载回放。
 *
 * 文件格式（自定义二进制格式）：
 *   Header:  [文件魔数 4B] [无人机数量 4B] [帧总数 4B] [帧间隔 4B]
 *   Frame 0: [drone0.x 4B] [drone0.y 4B] [drone0.color 1B] ...
 *   Frame 1: ...
 *
 * 也可采用文本格式（CSV/JSON-like），便于调试和跨程序交换。
 */

#ifndef FILE_IO_H
#define FILE_IO_H

#include "common.h"

/* ==================== 保存 ==================== */

/**
 * @brief 保存当前轨迹到文件
 *
 * 将本次模拟中每帧的无人机位置、灯光状态序列化到文件。
 *
 * @param fleet     无人机编队
 * @param count     无人机数量
 * @param filename  保存路径（如 "show1.dat"）
 * @return 1=成功, 0=失败
 */
int file_save_trajectory(Drone* fleet[], int count, const char* filename);

/**
 * @brief 保存当前编队配置到文本文件
 *
 * 保存编队参数（图案类型、中心、缩放、颜色等），方便下次快速还原。
 *
 * @param formation 编队指针
 * @param filename  保存路径
 * @return 1=成功, 0=失败
 */
int file_save_formation(const Formation* formation, const char* filename);

/* ==================== 加载 ==================== */

/**
 * @brief 从文件加载轨迹（用于回放）
 *
 * 解析 .dat 文件，将帧数据加载到内存。
 *
 * @param fleet      输出：重建的无人机编队
 * @param p_count    输出：无人机数量
 * @param filename   文件路径
 * @return 加载的帧数，0 表示失败
 */
int file_load_trajectory(Drone* fleet[], int* p_count, const char* filename);

/**
 * @brief 从文件加载编队配置
 * @param filename 文件路径
 * @return 加载的 Formation 指针，失败返回 NULL
 */
Formation* file_load_formation(const char* filename);

/**
 * @brief 列出目录下的所有轨迹文件
 * @param out_list  输出文件路径数组
 * @param max_count 最多列出数量
 * @return 实际文件数
 */
int file_list_trajectories(char out_list[][MAX_FILENAME_LEN], int max_count);

#endif // FILE_IO_H
