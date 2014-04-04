#include "hw.h"
#include <SDL/SDL.h>
#include "Z80/Z80.h"

#define MASTER_CLOCK        (18432000)
#define CPU_CLOCK           (MASTER_CLOCK/6)

Z80 cpu;
uint8_t ROM[0x4000];

int load_file(uint8_t *mem, char *fn)
{
    FILE *f = fopen(fn, "rb");
    if (!f)
    {
        fprintf(stderr, "cannot load ROM: %s\n", fn);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    fread(mem, 1, len, f);
    fclose(f);
    return len;
}

void load_roms(void)
{
    load_file(&ROM[0x0000], "roms/pacman.6e");
    load_file(&ROM[0x1000], "roms/pacman.6f");
    load_file(&ROM[0x2000], "roms/pacman.6h");
    load_file(&ROM[0x3000], "roms/pacman.6j");
}

void WrZ80(register word Addr,register byte Value)
{
    fprintf(stdout, "[CPU][PC=%04x] unknown write at %04hx: %02hhx\n", cpu.PC.W-1, Addr, Value);
}

byte RdZ80(register word Addr)
{
    fprintf(stdout, "[CPU][PC=%04x] unknown read at %04hx\n", cpu.PC.W-1, Addr);
    return 0xFF;
}

void OutZ80(register word Port,register byte Value)
{
    fprintf(stdout, "[MEM][PC=%04x] unknown I/O write at %04hx: %02hhx\n", cpu.PC.W-1, Port, Value);
}

byte InZ80(register word Port)
{
    fprintf(stdout, "[MEM][PC=%04x] unknown I/O read at %04hx\n", cpu.PC.W-1, Port);
    return 0xFF;
}

void PatchZ80(register Z80 *R) {}

int main(int argc, char *argv[])
{
    hw_init();

    int delta = 0;
    ResetZ80(&cpu);

    while (hw_poll())
    {
        delta = ExecZ80(&cpu, CPU_CLOCK/60 + delta);

        uint8_t *screen;
        int pitch;

        hw_beginframe(&screen, &pitch);
        hw_endframe();
    }

    return 0;
}
