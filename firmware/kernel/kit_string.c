/*****************************************************************************
 * @copyright       2024-202,AO DI LTD. 
 * @file            xxx.c
 * @brief           xxxx
 * @author          xx
 * @date            2024/12/30
 * @remark          初修订
 *****************************************************************************/

#include "kit_string.h"


/*****************************************************************************
 * @brief           将字符串转换为整数。如果字符串为空或为 NULL，则返回 0。
 * @param[in]       str: 需要转换的字符串。
 * @return          转换后的整数值。如果字符串无效（NULL 或长度为 0），返回 0。
 *****************************************************************************/
int kit_atoi(char *str)
{
    // 检查输入字符串是否为 NULL
    if (str == NULL)
    {
        return 0;
    }
    // 检查字符串是否为空（长度为 0）
    else if (strlen(str) == 0)
    {
        return 0;
    }
    else
    {
        // 使用标准库函数 atoi 进行字符串到整数的转换
        return atoi(str);
    }
}

/*****************************************************************************
 * @brief           去除字符串右侧的无效字符（空格、回车、换行符、制表符）。
 * @param[in]       str: 需要处理的字符串，操作将直接修改输入的字符串。
 * @return          无返回值，输入字符串将被修改。
 *****************************************************************************/
void kit_trim_right(char *str)
{
    // 找到字符串末尾，并向左检查无效字符
    char *end = str + strlen(str) - 1;
    while (end > str && ((unsigned char)(*end) == ' ' || *end == '\r' || *end == '\n' || *end == '\t')) {
        end--;
    }

    // 在去除右侧无效字符后，添加字符串终止符 '\0'
    *(end + 1) = '\0';
}

/*****************************************************************************
 * @brief           去除字符串左侧的无效字符（空格、回车、换行符、制表符）。
 * @param[in]       str: 需要处理的字符串，操作将直接修改输入的字符串。
 * @return          无返回值，输入字符串将被修改。
 *****************************************************************************/
void kit_trim_left(char *str)
{
    // 找到字符串左侧的第一个有效字符
    char *start = str;
    while (*start && ((unsigned char)(*start) == ' ' || *start == '\r' || *start == '\n' || *start == '\t')) {
        start++;
    }

    // 将有效部分的字符串移动到开头
    memmove(str, start, strlen(start) + 1);
}

/*****************************************************************************
 * @brief           去除字符串两端的无效字符（空格、回车、换行符、制表符）。
 * @param[in]       str: 需要处理的字符串，操作将直接修改输入的字符串。
 * @return          无返回值，输入字符串将被修改。
 *****************************************************************************/
void kit_trim_sides(char *str)
{
    // 处理空指针和空字符串的情况
    if (str == NULL || *str == '\0') {
        return;
    }

    // 从左侧移除无效字符（空格、回车、换行符、制表符）
    char *start = str;
    while (*start && ((unsigned char)(*start) == ' ' || *start == '\r' || *start == '\n' || *start == '\t')) {
        start++;
    }

    // 如果整个字符串都是无效字符，则返回空字符串
    if (*start == '\0') {
        str[0] = '\0';
        return;
    }

    // 将有效字符部分移到字符串的开头
    memmove(str, start, strlen(start) + 1);

    // 从右侧移除无效字符
    char *end = str + strlen(str) - 1;
    while (end > str && ((unsigned char)(*end) == ' ' || *end == '\r' || *end == '\n' || *end == '\t')) {
        end--;
    }

    // 在右侧无效字符之后添加字符串终止符
    *(end + 1) = '\0';
}

/*****************************************************************************
 * @brief           用于将字符数组拼接成一个字符串，separator分隔
 * @param[in]       count:字符数组数量
 * @param[in]       texts:字符串数组
 * @param[in]       separator:分隔字符
 * @return          拼接起来的字符串 
 *****************************************************************************/
char* kit_join_text(int count, const char** texts, const char* separator)
{
    if (count == 0) return NULL;
    
    size_t len = 0;
    for (int i = 0; i < count; i++) {
        len += strlen(texts[i]);
        if (i < count - 1) {
            len += strlen(separator);
        }
    }

    char* result = (char*)malloc(len + 1);  // 分配内存
    if (!result) return NULL;

    strcpy(result, texts[0]);
    for (int i = 1; i < count; i++) {
        strcat(result, separator);
        strcat(result, texts[i]);
    }
    
    return result;
}