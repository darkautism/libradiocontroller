#include "mjxq.h"
#include <stdlib.h>
#if defined(ARDUINO)
#include <Arduino.h>
#define gettime millis
#else
#include <sys/time.h>
static inline long __gettime()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_usec;
}
#define gettime __gettime
#endif

const static uint8_t mjx_default_channel[MJX_RF_NUM_CHANNELS] = {0x2e, 0x36, 0x3e, 0x46};
static const struct
{
    uint8_t mjx_id[3];
    uint8_t channels[4];
} mjx_map[] = {{{0x5F, 0x20, 0x05}, {0x35, 0x3D, 0x45, 0x4D}}};

// Return channels or return zero if nothing found
static uint8_t *select_channel(uint8_t *id)
{
    for (uint8_t i = 0; i < sizeof(mjx_map); i++)
    {
        if (memcmp(mjx_map[i].mjx_id, id, 3) == 0)
        {
            return (uint8_t *)mjx_map[i].channels;
        }
    }
    return 0;
}

mjxo_ctx_t *MjxqInit(mjx_hopping cb, mjx_disconnection cb2)
{
    mjxo_ctx_t *ret = (mjxo_ctx_t *)malloc(sizeof(mjxo_ctx_t));
    ret->status = MJX_REINIT;
    ret->pair_time = 0;
    ret->hopping_index = 0;
    ret->channels = (uint8_t *)mjx_default_channel;
    ret->hopping_callback = cb;
    ret->disconnection_callback = cb2;
    cb(ret->channels[ret->hopping_index]); // We should set channel in first time setup
    return ret;
}

void MjxqDestory(mjxo_ctx_t *ctx)
{
    free(ctx);
}

void MjxqUpdateFiniteStateMachine(mjxo_ctx_t *ctx, mjx_packet_t *pkt)
{
    switch (ctx->status)
    {
    case MJX_REINIT:
        ctx->channels = (uint8_t *)mjx_default_channel; // reset channels to default
        ctx->hopping_index = 255;
        ctx->status = MJX_PAIR;
        break;
    case MJX_PAIR:
        if (pkt->flag_pair)
        {
            ctx->pair_time = gettime();
            ctx->status = MJX_BIND;
            if ((ctx->channels = select_channel(pkt->mjx_id)) == 0)
            { // cannot found channel
                //printf("Cannot found channel\n");
                ctx->channels = (uint8_t *)mjx_default_channel;
                ctx->status = MJX_PAIR;
            }
        }
        break;
    case MJX_BIND:
        ctx->pair_time = gettime();
        break;
    }

    ctx->hopping_index = (ctx->hopping_index + 1) % (2 * MJX_RF_NUM_CHANNELS);
    if (ctx->hopping_index % 2 == 0)
        ctx->hopping_callback(ctx->channels[ctx->hopping_index / 2]);
}

void MjxqTask(mjxo_ctx_t *ctx)
{
    if (ctx->status == MJX_BIND && gettime() - ctx->pair_time > MJX_PACKET_TIMEOUT)
    { // time out, go to pair mode
        if (ctx->disconnection_callback)
        {
            ctx->disconnection_callback();
        }
        ctx->status = MJX_PAIR;
        ctx->channels = (uint8_t *)mjx_default_channel; // reset channels to default
        ctx->hopping_index = 0;
        ctx->hopping_callback(ctx->channels[ctx->hopping_index / 2]);
    }
}