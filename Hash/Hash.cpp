﻿#include "Hash.h"

namespace calg{
    EXTERN_API int extern_test_getnum() {
        return 1;
    }

    const int hash_length = 2048;
    const int hash_block_length = 128;

    inline bool cmp_a(uchar a, uchar b){ return a < b; }
    inline bool cmp_b(uchar a, uchar b){ return a > b; }

    /// <summary>
    /// 哈希运算器
    /// </summary>
    /// <param name="src">源内容地址</param>
    /// <param name="rst">结果地址</param>
    /// <returns>无返回值</returns>
    EXTERN_API void hash_str(uchar *src, uchar *rst){
        memset(rst, '0', sizeof(uchar) * 2048);                                 // clean                ->  初始化队列
        std::string str(re_ca_charc(src));                                      // byte(C#)             ->  输入转 string
        if (str.length() <= hash_length){                                        // no enough            ->  如果足位
            int index = hash_length - 1;
            for (int i = st_ca_int(str.length()) - 1;                            // copy value           ->  右对齐赋值
                 i >= 0; --i, --index){
                rst[index] = str[i];
            }
            uchar fill = 0;
            for (int i = 0; i < index; ++i, fill = fill == 255 ? 0 : fill + 1){  // fill blank           ->  填补空白
                rst[i] = fill;
            }
        } else{
            long long block = str.length() / hash_block_length;                 // calc block size      ->  计算区块数量
            long long remain = str.length() - block * hash_block_length;        // remain length        ->  计算多余长度
            uchar *blocks[hash_block_length + 1]{};                             // blocks[128][1...n]   ->  二维数组
            for (int i = 1; i <= hash_block_length; ++i)
                blocks[i] = new uchar[block + 1];                               // init                 ->  每个区块分配空间
            for (int i = 1, cur = 0; i <= hash_block_length; ++i)
                for (int j = 1; j <= block; ++j, ++cur)
                    blocks[i][j] = rst[cur];                                    // alloc                ->  按区块分配
            //TODO: 添加CUDA选择器, 可选是否使用CUDA操作
            for (int i = 1; i <= block; ++i){
                ull eq_a = 1, eq_b = 1, eq_c = 1, eq_d = 1;
                for (int k = 1; k <= 32; ++k)
                    eq_a *= (st_ca<int>(blocks[k][i]) << 1);
                for (int k = 32 + 1; k <= 64; ++k)
                    eq_b *= (st_ca<int>(blocks[k][i]) << 1);
                for (int k = 64 + 1; k <= 96; ++k)
                    eq_c *= (st_ca<int>(blocks[k][i]) << 1);
                for (int k = 96 + 1; k <= 128; ++k)
                    eq_d *= (st_ca<int>(blocks[k][i]) << 1);
                eq_a %= 255, eq_b %= 255, eq_c %= 255, eq_d %= 255;
                ull *eq_arr[4] = {&eq_a, &eq_b, &eq_c, &eq_d};
                for (int i = hash_length, index = 0;
                     i < st_ca_int(str.length());
                     ++i, index = (index == 3 ? 0 : index + 1)){
                    (*eq_arr[index]) += str[i];
                    (*eq_arr[index]) <<= 1;
                }
                ull ans = ((eq_a * 32) % 256) * ((eq_b * st_ca_int(std::pow(32, 2))) % 256);
                ans %= 256; ans *= ((eq_c * st_ca_int(std::pow(32, 3))) % 256);
                ans %= 256; ans *= ((eq_c * st_ca_int(std::pow(32, 4))) % 256);
                ans %= 256; rst[i] = st_ca<char>(ans);
            }
            for (int i = 1; i <= block; ++i)                                     // clear memory         ->  释放内存
                delete(blocks[i]);
        }

        /* ============================================ 以下是真哈希部分 ============================================ */

        for (int i = 2; i <= hash_length - 2; ++i){
            uchar a = rst[i - 1], b = rst[i], c = rst[i + 1], tmp;
            tmp = a, a = c, c = tmp;
            b = std::max(std::max(a * c, a * b), b * c) % 255;
            a *= b, c *= b;
            a = c - b, c = a - b;
            rst[i - 1] = (uchar)std::pow(a * c - a - c, c % 5) % 255;
            rst[i] = (uchar)std::pow(b - a - c, a % 5) % 255;
            rst[i + 1] = (uchar)std::pow(c * b - c - b, b % 5) % 255;
            std::sort(rst + i - 2, rst + i + 2, b % 2 == 0 ? cmp_a : cmp_b);
            rst[i + 2] += rst[i - 1] >= rst[i - 2] ? rst[i + 1] <<= 1 : rst[i] >>= 1;
            rst[i - 1] -= rst[i + 2] % 3 == 2 ? rst[i] + (rst[i + 2] >>= 1) : (uchar)((rst[i - 2] + (int)std::pow(a + b, c)) % 255);
            rst[i] *= rst[i + 2] - rst[i - 1];
            rst[i - 2] += rst[i] + (rst[i - 1] <<= 1);
            rst[i + 1] = rst[i + 2] ^ rst[i - 1] + rst[i] & rst[i - 2];

            rst[i - 2] -= rst[i + 1] - rst[i] - rst[i - 1] - rst[i - 2];
            rst[i - 1] -= rst[i] - rst[i - 1] - rst[i - 2];
            rst[i] -= rst[i - 1] - rst[i - 2];
            rst[i + 1] -= rst[i - 2];
            rst[i + 2] <<= 1;
            rst[i - 2] >>= 1;
            rst[i - 1] += rst[i - 2];
            rst[i] += rst[i - 1] - rst[i - 2];
            rst[i + 1] += rst[i] - rst[i - 1] - rst[i - 2];
            rst[i + 2] += rst[i + 1] - rst[i] - rst[i - 1] - rst[i - 2];

            /*rst[i] = std::cos(rst[i - 1]);
            rst[i - 1] = std::sin(rst[i + 1]);
            rst[i + 1] = std::sin(std::cos(rst[i + 2]));
            rst[i + 2] = std::cos(std::sin(rst[i]));*/
        }

        //  解决小于 2048 时前导 '0' 重复导致一摸一样哈希的问题
        for (int i = 0, j = hash_length - 1, launched = 1;
             i != j && i < j && i != j - 1;
             ++i, ++launched, j -= ((~rst[i + 1] ^ rst[i]) % 3 == 0 ? 1 : 2)){
            rst[i] += rst[j] * rst[j - 1];
            rst[i] >>= (rst[i + 1] % 4);
            rst[i + 1] = rst[i] & (rst[j - 1] + rst[j]);
            rst[i + 2] = rst[i + 1] | (rst[j - 1] & rst[i]);
            rst[i] += rst[i] ^ rst[j];
            rst[i] += (rst[0] &= rst[j]) ^ (rst[hash_length - 1] >>= 1);
            rst[0] >>= 1;
            rst[j] -= rst[i] ^ rst[i + 1];
            rst[j - 1] -= (rst[j - 1] + ((rst[(j + i) >> 1] ^ ~rst[((rst[i] * rst[j] - rst[i] - rst[j]) % 2048) >> 1]) >> 1));
            rst[0] = rst[i] + rst[j];
            rst[hash_length - 1] = rst[i] - rst[j];
            if (launched >= 2048) break;
        }

        // 四指针混淆器
        for (int i = 1023, j = 1024, x = 0, y = 2048 - 1;
             i != 0 && j != 2048 - 1 && x != 1023 && y != 1024 && i < j && x < y;
             --i, ++j, ++x, --y){

            /*if(i >= 2){
                rst[i - 1] = rst[i] | st_ca_int(std::sin(rst[j]));
                rst[i - 2] = rst[i - 1] & st_ca_int(std::sin(rst[j - 1]));
                rst[i] = (rst[i - 1] & rst[i - 2]) ^ st_ca_int(std::cos(rst[i - 1] | rst[i - 2]));
            }
            if(j <= 2048 - 3){
                rst[j + 1] = rst[j] & st_ca_int(std::cos(rst[i]));
                rst[j + 2] = rst[j - 1] | st_ca_int(std::cos(rst[i + 1]));
                rst[j] = (rst[j + 1] | rst[j + 2]) ^ st_ca_int(std::sin(rst[j + 1] & rst[j + 2]));
            }*/
            /*rst[x] = rst[x] * ((rst[i] * rst[j] + (rst[i] & rst[j]) - (rst[j] ^ rst[i])) % 255);
            rst[y] = rst[y] * ((rst[i] * rst[j] - (rst[i] | rst[j]) - (rst[i] ^ rst[j])) % 255);*/
        }
    }

    /// <summary>
    /// 哈希压缩器
    /// </summary>
    /// <param name="src">源哈希值地址 (源哈希值长必须为2048位)</param>
    /// <param name="rst">哈希压缩值地址</param>
    /// <returns>无返回值</returns>
    EXTERN_API void hash_compress_str(uchar *src, uchar *rst){

    }

    EXTERN_API int hash_file(uchar *fileName, int type){
        //TODO: 文件哈希

        return 1;
    }
}
