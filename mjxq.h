#ifndef __MJXQ__
#define __MJXQ__
#include <stdint.h>
#include <stdbool.h>


#define MJX_PACKET_SIZE 16
#define MJX_ENCODED_PACKET_SIZE 23
#define MJX_RF_NUM_CHANNELS 4
#define MJX_PACKET_TIMEOUT 5000


typedef enum {
  MJX_REINIT = 0,
  MJX_PAIR,
  MJX_BIND,
}mjx_status_t;

typedef struct { // for JJRC H36F
  uint8_t throttle;
  int8_t rudder;
  int8_t elevator;
  int8_t aileron;
  uint8_t unknow[3];
  uint8_t mjx_id[3];
  uint8_t flag_headless:1;
  uint8_t flag_return_to_host:1;
  uint8_t unknow2[3];
  uint8_t flag_flip:1;
  uint8_t flag_unknow:1;
  uint8_t flag_speed:1;
  uint8_t flag_picture:1;
  uint8_t flag_vedio:1;
  uint8_t flag_led:1;
  uint8_t flag_pair:2;
  uint8_t sum;
} mjx_packet_t;

typedef void (*mjx_hopping)(uint8_t);
typedef void (*mjx_disconnection)();

typedef struct mjxq_ctx {
    mjx_status_t status;
    uint8_t * channels;
    uint8_t hopping_index;    
    unsigned long pair_time;
    mjx_hopping hopping_callback;
    /* This is callback function for disconnect, you can add some feature like return to host at this function */
    mjx_disconnection disconnection_callback;
} mjxo_ctx_t;

#ifdef  __cplusplus
extern "C" {
#endif

mjxo_ctx_t * MjxqInit( mjx_hopping cb, mjx_disconnection cb2 );
void MjxqDestory(mjxo_ctx_t * ctx);
void MjxqUpdateFiniteStateMachine( mjxo_ctx_t * ctx, mjx_packet_t * pkt);
// Task for detect packet timeout
void MjxqTask(mjxo_ctx_t *ctx);

#ifdef  __cplusplus
}
#endif
#endif