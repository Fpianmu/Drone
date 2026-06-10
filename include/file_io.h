/*
 * file_io.h —— 文件读写模块
 *
 * 轨迹数据和编队配置的保存和加载。
 * 当前为桩实现（基础框架已写好，队友可在此基础上完成完整功能）。
 */

#ifndef FILE_IO_H
#define FILE_IO_H

#include "common.h"

// 保存当前轨迹到文件
int file_save_trajectory(Drone* fleet[], int count, const char* filename);

// 保存编队配置到文本文件
int file_save_formation(const Formation* formation, const char* filename);

// 从文件加载轨迹（用于回放），返回帧数
int file_load_trajectory(Drone* fleet[], int* p_count, const char* filename);

// 从文件加载编队配置
Formation* file_load_formation(const char* filename);

// 列出目录下的轨迹文件（最多 max_count 个）
int file_list_trajectories(char out_list[][MAX_FILENAME_LEN], int max_count);

#endif
