/*
 * file_io.cpp —— 文件读写模块（桩实现）
 *
 * 当前提供基本接口框架，供编译通过。
 * 队友后续可实现完整的轨迹帧序列保存/加载。
 */

#include "../include/file_io.h"

/* ==================== 保存 ==================== */

int file_save_trajectory(Drone* fleet[], int count, const char* filename)
{
    if (fleet == NULL || filename == NULL || count <= 0) return 0;

    // 打开文件（二进制写入）
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("[FILE_IO] 无法创建文件: %s\n", filename);
        return 0;
    }

    // TODO: 队友实现完整的轨迹帧数据保存
    // 当前仅写入基本头信息作为占位
    int magic = 0x44524F4E;  // "DRON"
    fwrite(&magic, sizeof(int), 1, fp);
    fwrite(&count, sizeof(int), 1, fp);

    fclose(fp);
    printf("[FILE_IO] 轨迹已保存: %s\n", filename);
    return 1;
}

int file_save_formation(const Formation* formation, const char* filename)
{
    if (formation == NULL || filename == NULL) return 0;

    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("[FILE_IO] 无法创建文件: %s\n", filename);
        return 0;
    }

    // TODO: 队友实现编队配置文本保存
    fprintf(fp, "# 编队配置\n");
    fprintf(fp, "name=%s\n", formation->name);
    fprintf(fp, "pattern=%d\n", formation->pattern);
    fprintf(fp, "scale=%.1f\n", formation->scale);

    fclose(fp);
    printf("[FILE_IO] 编队配置已保存: %s\n", filename);
    return 1;
}

/* ==================== 加载 ==================== */

int file_load_trajectory(Drone* fleet[], int* p_count, const char* filename)
{
    if (fleet == NULL || p_count == NULL || filename == NULL) return 0;

    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("[FILE_IO] 无法打开文件: %s\n", filename);
        return 0;
    }

    // TODO: 队友实现完整轨迹加载
    int magic, count;
    if (fread(&magic, sizeof(int), 1, fp) != 1
     || fread(&count, sizeof(int), 1, fp) != 1) {
        printf("[FILE_IO] 文件格式错误: %s\n", filename);
        fclose(fp);
        return 0;
    }

    if (magic != 0x44524F4E) {
        printf("[FILE_IO] 文件魔数不匹配: %s\n", filename);
        fclose(fp);
        return 0;
    }

    fclose(fp);
    printf("[FILE_IO] 轨迹已加载: %s (%d 架无人机)\n", filename, count);
    *p_count = count;
    return count;
}

Formation* file_load_formation(const char* filename)
{
    if (filename == NULL) return NULL;

    // TODO: 队友实现编队配置加载
    printf("[FILE_IO] 编队配置加载（桩）: %s\n", filename);
    return NULL;  // 桩返回 NULL
}

int file_list_trajectories(char out_list[][MAX_FILENAME_LEN], int max_count)
{
    if (out_list == NULL || max_count <= 0) return 0;

    // TODO: 队友实现文件列表
    // 使用 FindFirstFile / FindNextFile 列出 *.dat 文件
    printf("[FILE_IO] 轨迹文件列表（桩）\n");
    return 0;
}
