/**
 * FILE     : conf.h
 * DESC     : [string]key-[string]value dictonary
 * AUTHOR   : v0.3.0 written by Alex Stocks
 * DATE     : on Oct 25, 2012
 * LICENCE  : GPL 2.0
 * MOD      :
 **/

#ifndef __CONF_H__
#define __CONF_H__

#include <stddef.h>

/**
 * description    : read a configure file @file and get the dictionary <br/><br/>
 *
 * in@file        : file name
 * in-out@conf    : configure dictionary
 *
 * out@ret        : the return value is 0 if both params are not nil
 *                  and the file @file is valid configure file;
 *                  otherwise -1 if params are illegal or -2 if @file
 *                  does not exist or it does not have any configure key-value.
 **/
int confInit(void** conf, char* file);
/**
 * description    : free the dictionary @conf <br/><br/>
 *
 * in@conf        : configure dictionary
 *
 * out@ret        : return is void
 **/
void confUninit(void** dict);
/**
 * description    : get value of the @key0+@key1 from the dictionary @conf <br/><br/>
 *
 * in@conf        : configure dictionary. pay attention that parameter type is "void*".
 * in@key0        : first key
 * in@key1        : second key
 *
 * out@ret        : if the dictionary @conf is complete and there is value of @key,
 *                  the return is its value, otherwise nil instead.
 **/
const char* confGetValue(void* conf, const char* key0, const char* key1);
/**
 * description    : get the @index-th element of the @key0+@key1 from the dictionary
 *                  @conf <br/><br/>
 *
 * in@conf        : configure dictionary
 * in@key0        : first key
 * in@key1        : second key
 * in@index       : element index. the start index is 0.
 *
 * out@ret        : if the value of @key in@conf is a array and the @index
 *                  is not greater than the maximum element number of its
 *                  related array, the return value is what you wanna, otherwise
 *                  it is nil.
 **/
const char* confGetArrayValue(void* conf, const char* key0, const char* key1, int index);
/**
 * description    : get the size of @key0+@key1's value array. <br/><br/>
 *
 * in@conf        : configure dictionary
 * in@key0        : first key
 * in@key1        : second key
 * in-out@szie    : array size
 *
 * out@ret        : if successful, ret is 0; otherwise -1 if some parameters
 *                  are illegal.
 **/
int confGetArraySize(void* conf, const char* key0, const char* key1, size_t* size);

#define CONF_VALUE(dict, key0, key1) ({                             \
    const char* __conf_ret__ = nil_str;                             \
    __conf_ret__ = confGetValue(                                    \
                                  (void*)(dict),                    \
                                  (const char*)key0,                \
                                  (const char*)key1                 \
                                 );                                 \
    (char*)__conf_ret__;                                            \
})

#define CONF_VALUE_U8(dict, key0, key1)                             \
    str2u8(CONF_VALUE(dict, key0, key1))

#define CONF_ARRAY_VALUE(dict, key0, key1, idx) ({                  \
    const char* __conf_ret__ = nil_str;                             \
    __conf_ret__ = confGetArrayValue(                               \
                                        (void*)(dict),              \
                                        (const char*)key0,          \
                                        (const char*)key1,          \
                                        idx                         \
                                        );                          \
    (char*)__conf_ret__;                                            \
})

#define CONF_ARRAY_VALUE_U8(dict, key0, key1, idx)                  \
    str2u8((char*)CONF_ARRAY_VALUE(dict, key0, key1, idx))

#define CONF_ARRAY_SIZE(dict, key0, key1) ({                        \
    size_t __conf_size__ = 0;                                       \
    confGetArraySize(                                               \
                        (void*)(dict),                              \
                        (const char*)(key0),                        \
                        (const char*)(key1),                        \
                        &__conf_size__                              \
                       );                                           \
})

#endif
