#include <Arduino.h>
typedef struct t_DAVIS_LORA_OUT
{
    uint8_t msg_type; // 07
    uint8_t msg_length;
} DAVIS_LORA_OUT;

class HSDavis
{
public:
    HSDavis();
    DAVIS_LORA_OUT read_latest_values();
};
