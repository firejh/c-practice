#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define ERR_OTHER                     -1
#define OK                            0
#define ERR_PARAM                     1
#define ERR_MEM                       2
#define ERR_TBL_SET                   3
#define ERR_NOT_FOUND                 10
#define ERR_SAME_VALUE                16
#define ERR_TBL_END                   19
// error of hashmap
#define ERR_INIT_MAP                  91

// error of share memory
#define ERR_SHMGET                    201
#define ERR_SHMAT                     202
#define ERR_SHMINIT                   203

#define ERR_LOG_LEVEL                 211

/******************************************************************************
 * Static Hash Table Container - works in fixed size memory
 * qhasharr.c
 ******************************************************************************/

/* tunable knobs */
#define _Q_HASHARR_KEYSIZE (16)    /*!< knob for maximum key size. */
#define _Q_HASHARR_VALUESIZE (16)  /*!< knob for maximum data size in a slot. */

//////////////////////////////////
// hashtable
//////////////////////////////////

union _slot_data
{
    /*!< key/value data */
    struct _Q_HASHARR_SLOT_KEYVAL
    {
        unsigned char value[_Q_HASHARR_VALUESIZE];  /*!< value */

        char key[_Q_HASHARR_KEYSIZE];  /*!< key string, can be cut */
        uint16_t  keylen;              /*!< original key length */
        unsigned char keymd5[16];      /*!< md5 hash of the key */
    } pair;

    /*!< extended data block, used only when the count value is -2 */
    struct _Q_HASHARR_SLOT_EXT
    {
        unsigned char value[sizeof(struct _Q_HASHARR_SLOT_KEYVAL)];
    } ext;
};

/**
 * qhasharr internal data slot structure
 */
struct qhasharr_slot_s
{
    short  count;   /*!< hash collision counter. 0 indicates empty slot,
                     -1 is used for collision resolution, -2 is used for
                     indicating linked block */
    uint32_t  hash; /*!< key hash. we use FNV32 */

    uint8_t size;   /*!< value size in this slot*/
    int link;       /*!< next link */
    union _slot_data data;
};

/**
 * qhasharr container
 */
struct qhasharr_s
{
    /* private variables - do not access directly */
    int maxslots;       /*!< number of maximum slots */
    int usedslots;      /*!< number of used slots */
    int num;            /*!< number of stored keys */
    char slots[];       /*!< data area pointer */
};

/**
 * object data structure.
 */
struct qobj_s
{
    void *data;         /*!< data */
    size_t size;        /*!< data size */
    uint8_t type;       /*!< data type */
};

/**
 * named-object data structure.
 */
struct qnobj_s
{
    char *name;         /*!< object name */
    void *data;         /*!< data */
    size_t name_size;   /*!< object name size */
    size_t data_size;   /*!< data size */
};

/* types */
typedef struct qhasharr_s qhasharr_t;
typedef struct qhasharr_slot_s qhasharr_slot_t;
typedef struct qnobj_s qnobj_t;      /*!< named-object type*/

/* public functions */
extern qhasharr_t *qhasharr(void *memory, size_t memsize);
extern size_t qhasharr_calculate_memsize(int max);
extern int qhasharr_init(qhasharr_t *tbl, qhasharr_slot_t **_tbl_slots);

/* capsulated member functions */
extern bool qhasharr_put(qhasharr_t *tbl, const char *key, size_t key_size, const void *value, size_t val_size);
extern bool qhasharr_putstr(qhasharr_t *tbl, const char *key, const char *str);
extern bool qhasharr_putint(qhasharr_t *tbl, const char *key, int64_t num);
extern bool qhasharr_exist(qhasharr_t *tbl, const char *key, size_t key_size);
extern void *qhasharr_get(qhasharr_t *tbl, const char *key, size_t key_size, size_t *val_size);
extern char *qhasharr_getstr(qhasharr_t *tbl, const char *key);
extern int64_t qhasharr_getint(qhasharr_t *tbl, const char *key);
extern bool qhasharr_getnext(qhasharr_t *tbl, qnobj_t *obj, int *idx);

extern bool qhasharr_remove(qhasharr_t *tbl, const char *key, size_t key_size);

extern int  qhasharr_size(qhasharr_t *tbl, int *maxslots, int *usedslots);
extern void qhasharr_clear(qhasharr_t *tbl);


/* qhash.c */
extern bool qhashmd5(const void *data, size_t nbytes, void *retbuf);
extern uint32_t qhashmurmur3_32(const void *data, size_t nbytes);

/**
 * translate the binary of md5 into the string of md5
 */
extern int qhashmd5_bin_to_hex(char *md5_str, const unsigned char *md5_int, int md5_int_len);

#ifdef __cplusplus
}
#endif

#endif
