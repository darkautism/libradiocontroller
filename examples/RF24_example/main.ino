#include <SPI.h>
#include "RF24.h"
#include "printf.h"
#include <xn297.h>
#include <mjxq.h>

RF24 radio(9, 10);

void packet_debuger( mjx_packet_t * pkt ) {
  printf("throttle\t:%d\n", pkt->throttle);
  printf("rudder\t:%d\n", pkt->rudder);
  printf("elevator\t:%d\n", pkt->elevator);
  printf("aileron\t:%d\n", pkt->aileron);
  printf("mjx_id\t:0x%02x%02x%02x\n", pkt->mjx_id[0], pkt->mjx_id[1], pkt->mjx_id[2]);
  printf("sum\t:%d\n", pkt->sum);
  printf("status\t:");
  if(pkt->flag_speed) {
    printf("binded");
  }
  if(pkt->flag_pair) {
    printf("pair");
  }
  printf("\nfeature\t:");
  if(pkt->flag_headless) {
    printf("headless, ");
  }
  if(pkt->flag_return_to_host) {
    printf("return_to_host, ");
  }
  if(pkt->flag_flip) {
    printf("flip, ");
  }
  if(pkt->flag_picture) {
    printf("picture, ");
  }
  if(pkt->flag_vedio) {
    printf("vedio, ");
  }
  if(pkt->flag_led) {
    printf("led, ");
  }
  printf("\n");
}

mjxo_ctx_t * mjxq1;
void setup(void)
{
  Serial.begin(115200);
  while (Serial)
    ;
    delay(5000);
  const uint64_t tx_pipe = 0x000C710F55LL; // 0x2F7D872649LL;
  printf_begin();
  radio.begin();
  while (!radio.isChipConnected())
    delay(100);

  radio.setAddressWidth(3);
  radio.openReadingPipe(0, tx_pipe);
  radio.setPayloadSize(MJX_ENCODED_PACKET_SIZE);
  radio.setAutoAck(false);
  radio.setRetries(0, 0);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.disableDynamicPayloads();
  radio.disableCRC();
  radio.openReadingPipe(0, tx_pipe);
  mjxq1 = MjxqInit([](uint8_t channel){
    printf("Channel:%d\n", channel);
    radio.stopListening();
    radio.setChannel(channel);
    radio.startListening();
  }, 0);
  radio.printDetails();
}

void loop(void)
{
  static mjx_packet_t packet;
  static byte buf[MJX_ENCODED_PACKET_SIZE];
  MjxqTask(mjxq1);

  if (radio.available())
  {
    memset(buf, 0, MJX_ENCODED_PACKET_SIZE);
    radio.read(buf, MJX_ENCODED_PACKET_SIZE);
    if(XN297Decode((uint8_t*)&packet, buf, MJX_ENCODED_PACKET_SIZE) ) {
      //packet_debuger(&packet);
      MjxqUpdateFiniteStateMachine(mjxq1, &packet);
    }
  }  
}
