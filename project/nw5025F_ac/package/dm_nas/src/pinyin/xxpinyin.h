#ifndef _XX_PINYIN_H_INCLUDED_
#define _XX_PINYIN_H_INCLUDED_
/**
* UTF-8字符串获pinyin
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct xx_pinyin_s xx_pinyin_t;

struct xx_pinyin_s {
    char i_bit1;
    /* 第一个字节对应有数值 */
    char i_bit2;
    /* 第二个字节对应有数值 */
    char i_bit3;
    /* 第三个字节对应有数值 */
//    int v_count;
    /*拼音个数*/
    char v_1[8];
    /*第一个拼音*/
    char v_2[8];
//    /*第二个拼音*/
    char v_3[8];
//    /*第三个拼音*/

//    char v_f_1;
//    /*第一个拼音首字母*/
//    char v_f_2;
//    /*第二个拼音首字母*/
//    char v_f_3;/*第三个拼音首字母*/
};

struct pinyin_list{
    xx_pinyin_t *xx_pinyin_229db;
    xx_pinyin_t *xx_pinyin_233db;
    xx_pinyin_t *xx_pinyin_230db;
    xx_pinyin_t *xx_pinyin_231db;
    xx_pinyin_t *xx_pinyin_232db;
    xx_pinyin_t *xx_pinyin_228db;
    xx_pinyin_t *xx_pinyin_other_db;
};



void xxInitPinyinDB();
void xxDestoryPinyinDB();

void xxInsertPinyin(char i_1, char i_2, char i_3, int count, char *v_1, char *v_2, char *v_3, int *index);

xx_pinyin_t *xxGetCharPinyin(char *in);

/*获取一个字的拼音*/
void xxGetPinyin(char *in, char *out, char *f_out);/*获取拼音字符串*/
#endif /* _XX_PINYIN_H_INCLUDED_ */
