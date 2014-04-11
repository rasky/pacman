#include <assert.h>
#include <stdio.h>
#include <stdint.h>

extern uint8_t SOUND_ROM[8][32];
static uint8_t REGS[0x20];
static int v1_counter = 0;

#define BUF_SIZE 96000 / 60
int16_t SOUND_BUF[3][BUF_SIZE];


void sound_register_w(uint8_t addr, uint8_t value)
{
    assert(addr < 0x20);
    REGS[addr] = value;
}

static int read_4bit_int(uint8_t *data, int num_nibbles)
{
    int value = 0;

    while (num_nibbles >= 0)
    {
        value <<= 4;
        value |= data[--num_nibbles] & 0xF;
    }
    return value;
}

static void write_4bit_int(uint8_t *data, int num_nibbles, int value)
{
    while (num_nibbles--)
    {
        *data++ = value & 0xF;
        value >>= 4;
    }
}

static void sound_emulate_voice(int16_t *out, int ticks, int *counter, int frequency, int waveform, int volume)
{

}


void sound_emulate(int ticks)
{
    assert(ticks <= BUF_SIZE);

    {
        int v1_counter = read_4bit_int(&REGS[0x0], 5);
        int v1_waveform = REGS[0x5] & 0x7;
        int v1_frequency = read_4bit_int(&REGS[0x10], 5);
        int v1_volume = REGS[0x15];
        if (v1_volume != 0)
            printf("[SOUND] Voice 1: cnt=%d wave=%d freq=%d vol=%d\n", v1_counter, v1_waveform, v1_frequency, v1_volume);
        sound_emulate_voice(SOUND_BUF[0], ticks, &v1_counter, v1_frequency, v1_waveform, v1_volume);
        write_4bit_int(&REGS[0x0], 5, v1_counter);
    }

    {
        int v2_counter = read_4bit_int(&REGS[0x6], 4);
        int v2_waveform = REGS[0xA] & 0x7;
        int v2_frequency = read_4bit_int(&REGS[0x16], 4);
        int v2_volume = REGS[0x1A];
        if (v2_volume != 0)
            printf("[SOUND] Voice 2: cnt=%d wave=%d freq=%d vol=%d\n", v2_counter, v2_waveform, v2_frequency, v2_volume);
        sound_emulate_voice(SOUND_BUF[1], ticks, &v2_counter, v2_frequency, v2_waveform, v2_volume);
    }

    {
        int v3_counter = read_4bit_int(&REGS[0xB], 4);
        int v3_waveform = REGS[0xF] & 0x7;
        int v3_frequency = read_4bit_int(&REGS[0x1B], 4);
        int v3_volume = REGS[0x1F];
        if (v3_volume != 0)
            printf("[SOUND] Voice 3: cnt=%d wave=%d freq=%d vol=%d\n", v3_counter, v3_waveform, v3_frequency, v3_volume);
        sound_emulate_voice(SOUND_BUF[2], ticks, &v3_counter, v3_frequency, v3_waveform, v3_volume);
    }
}


void sound_play(int16_t *audio, int samples)
{
    int ticks = 96000 / 60;
    sound_emulate(ticks);

    int step = (ticks << 16) / samples;
    int cur = 0;

    for (int i=0;i<samples;++i)
    {
        int16_t s1 = SOUND_BUF[0][cur >> 16];
        int16_t s2 = SOUND_BUF[1][cur >> 16];
        int16_t s3 = SOUND_BUF[2][cur >> 16];
        *audio++ = s1;
        cur += step;
    }
}
