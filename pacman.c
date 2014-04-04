#include "hw.h"
#include "gfx.h"
#include <SDL/SDL.h>
#include "Z80/Z80.h"

#define MASTER_CLOCK        (18432000)
#define CPU_CLOCK           (MASTER_CLOCK/6)

Z80 cpu;
int framecounter;
uint8_t interrupt_vector;
uint8_t ROM[0x4000];
uint8_t VIDEO_RAM[0x400];
uint8_t COLOR_RAM[0x400];
uint8_t RAM[0x800];

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

    uint8_t temp[0x1000];
    load_file(temp, "roms/pacman.5e");
    decode_chars(temp);
    load_file(temp, "roms/pacman.5f");
    decode_sprites(temp);
    load_file(temp, "roms/82s123.7f");
    decode_colors(temp);
    load_file(temp, "roms/82s126.4a");
    decode_palettes(temp);
}

uint8_t IN0()
{
    uint8_t ret = 0xFF;

    if (keystate[SDLK_5])
        ret &= ~(1<<5);
    if (keystate[SDLK_6])
        ret &= ~(1<<6);

    return ret;
}

uint8_t IN1()
{
    uint8_t ret = 0xFF;

    if (keystate[SDLK_1])
        ret &= ~(1<<5);
    if (keystate[SDLK_2])
        ret &= ~(1<<6);

    return ret;
}

void WrZ80(register word Addr,register byte Value)
{
    Addr &= 0x7FFF;
    if (Addr < 0x4000) { fprintf(stdout, "[CPU][PC=%04x](%04d) writing to ROM %04hx: %02hhx\n", cpu.PC.W-1, framecounter, Addr, Value); return; }
    if (Addr < 0x4400) { VIDEO_RAM[Addr-0x4000] = Value; return; }
    if (Addr < 0x4800) { COLOR_RAM[Addr-0x4400] = Value; return; }
    if (Addr < 0x5000) { RAM[Addr-0x4800] = Value; return; }
    if (Addr == 0x50C0) { /* watchdog */ return; }
unknown:
    fprintf(stdout, "[CPU][PC=%04x](%04d) unknown write at %04hx: %02hhx\n", cpu.PC.W-1, framecounter, Addr, Value);
}

byte RdZ80(register word Addr)
{
    Addr &= 0x7FFF;
    if (Addr < 0x4000) return ROM[Addr];
    if (Addr < 0x4400) return VIDEO_RAM[Addr-0x4000];
    if (Addr < 0x4800) return COLOR_RAM[Addr-0x4400];
    if (Addr < 0x5000) return RAM[Addr-0x4800];
    if (Addr == 0x5000) { return IN0(); }
    if (Addr == 0x5040) { return IN1(); }
unknown:
    fprintf(stdout, "[CPU][PC=%04x](%04d) unknown read at %04hx\n", cpu.PC.W-1, framecounter, Addr);
    return 0xFF;
}

void OutZ80(register word Port,register byte Value)
{
    Port &= 0xFF;
    if (Port == 0)
    {
        word dest = 0;
        interrupt_vector = Value;
        if (cpu.IFF & IFF_IM2) {
            dest = (cpu.I << 8) + interrupt_vector;
            dest = RdZ80(dest) + (RdZ80(dest+1)<<8);
        }
        fprintf(stdout,
            "[CPU][PC=%04x] IRQ: Vector=%02x Func=%02hx\n",
            cpu.PC.W-1, Value, dest);
        return;
    }
    fprintf(stdout, "[MEM][PC=%04x](%04d) unknown I/O write at %04hx: %02hhx\n", cpu.PC.W-1, framecounter, Port, Value);
}

byte InZ80(register word Port)
{
    Port &= 0xFF;
    fprintf(stdout, "[MEM][PC=%04x](%04d) unknown I/O read at %04hx\n", cpu.PC.W-1, framecounter, Port);
    return 0xFF;
}

void PatchZ80(register Z80 *R) {}

int main(int argc, char *argv[])
{
    hw_init();
    load_roms();

    int delta = 0;
    ResetZ80(&cpu);

    while (hw_poll())
    {
        delta = ExecZ80(&cpu, CPU_CLOCK/60 + delta);

        IntZ80(&cpu, interrupt_vector);

        uint8_t *screen;
        int pitch;

        hw_beginframe(&screen, &pitch);
        gfx_draw(screen, pitch);
        hw_endframe();
        ++framecounter;
    }

    return 0;
}
