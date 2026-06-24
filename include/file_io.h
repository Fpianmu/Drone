/*
 * file_io.h - 文件读写模块
 *
 * 轨迹数据和编队配置的保存和加载。
 * 目前基础框架已写好，完整功能待完善。
 */

#ifndef FILE_IO_H
#define FILE_IO_H

#include "common.h"

// 保存轨迹状态到文件
int file_save_trajectory(Drone* fleet[], int count, const char* filename);

// 保存编队配置到文本文件
int file_save_formation(const Formation* formation, const char* filename);

// 从文件加载轨迹（回放用）
int file_load_trajectory(Drone* fleet[], int* p_count, const char* filename);

// 从文件加载编队配置
Formation* file_load_formation(const char* filename);

// 列出当前目录下的轨迹文件
int file_list_trajectories(char out_list[][MAX_FILENAME_LEN], int max_count);

#endif
