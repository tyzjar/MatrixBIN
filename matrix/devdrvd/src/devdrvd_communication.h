#ifndef DEVDRVD_COMMUNICATION
#define DEVDRVD_COMMUNICATION

#ifdef      __cplusplus
extern "C" {
#endif

#ifndef INLINE
# if __GNUC__ && !__GNUC_STDC_INLINE__
#  define INLINE extern inline
# else
#  define INLINE inline
# endif
#endif

#define START_CHARACTER 0x02
#define STOP_CHARACTER  0x0d

#define PACKET_TYPE(X)  (*(((char *)(X)+1)))

#define PT_WEIGHTPCKT   'W'
#define PT_DIOPCKT      'D'
#define PT_ERRPCKT      'E'

#ifndef UNUSED
#define UNUSED(x) (void)(x);
#endif

#ifndef SET_BIT
#define SET_BIT(val, bitIndex) \
    do { val |= (1 << bitIndex); } while (0)
#endif

#ifndef CLEAR_BIT
#define CLEAR_BIT(val, bitIndex) \
    do { val &= ~(1 << bitIndex); } while (0)
#endif

#ifndef TOGGLE_BIT
#define TOGGLE_BIT(val, bitIndex) \
    do{ val ^= (1 << bitIndex); } while (0)
#endif

#ifndef BIT_IS_SET
#define BIT_IS_SET(val, bitIndex) ((val) & (1<<(bitIndex)))
#endif

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY( a ) (sizeof( a ) / sizeof( a[ 0 ] ))
#endif

#define STABLE 0
#define ZERO 1
#define TARE 2
#define OVERLOAD 4
//<MJKJ
#define NETGROSS 8
//>
#define UNDERLOAD 16

#include <sys/time.h>

extern const char* package_pattern;
extern const char* weight_package_pattern;
extern const char* discrete_package_pattern;
extern const char* error_package_pattern;
extern const char* stop_character;

typedef struct  __pckt_item_t
{
    void                        * pckt_ptr;
    size_t                      pckt_size;
    struct  __pckt_item_t       * next;
}   pckt_item_t, * pckt_item_p;

struct weight_struct {
    struct timeval ts;
    unsigned char channel;
    float weight;
    float tare;
    unsigned int status;
};

/* weight_struct pack/unpack functions group */
/**
 * @brief pack_weight
 * @param data
 * @return int - length of data string
 */
size_t pack_weight(char**, const struct weight_struct*);

/**
 * @brief unpack_weight
 * @param data
 */
void unpack_weight(struct weight_struct*, const char*);

///* weight_struct helper functions */
///**
// * @brief is_stable
// * @return
// */
//INLINE int dis_stable(const unsigned int status)
//{
//    return BIT_IS_SET((int)status, STABLE);
//}

///**
// * @brief is_zeroed
// * @return
// */
//INLINE int dis_zeroed(const unsigned int status)
//{
//    return BIT_IS_SET((int)status, ZERO);
//}

///**
// * @brief is_tared
// * @return
// */
//INLINE int dis_tared(const unsigned int status)
//{
//    return BIT_IS_SET((int)status, TARE);
//}


/**
 * @brief The discrete_struct struct
 */
struct discrete_struct {
    struct timeval ts;
    unsigned char channel;
    unsigned int value;
};

/**
 * @brief pack_discrete
 * @param data
 * @return int length of data packed
 */
size_t pack_discrete(char**, const struct discrete_struct*);

/**
 * @brief unpack_discrete
 * @param data
 */
void unpack_discrete(struct discrete_struct*, const char*);

/**
 * @brief The error_struct struct
 */
struct error_struct {
    unsigned int timestamp;
    unsigned char code;
    char* message;
};

/**
 * @brief The uni_packet union
 */
union   uni_packet {
    struct  weight_struct       wgt_pack;
    struct  discrete_struct     dio_pack;
    struct  error_struct        err_pack;
};

/**
 * @brief pack_error
 * @param data
 */
size_t pack_error(char**, const struct error_struct*);

/**
 * @brief unpack_error
 * @param data
 */
void unpack_error(struct error_struct*, const char* data);

/**
 * @brief parse_buffer
 * @return
 */
pckt_item_p     parse_buffer(void *, size_t);

#ifdef      __cplusplus
}
#endif

#endif
