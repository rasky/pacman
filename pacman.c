#include "hw.h"
#include "gfx.h"
#include <SDL/SDL.h>
#include "Z80/Z80.h"

#define MASTER_CLOCK        (18432000)
#define CPU_CLOCK           (MASTER_CLOCK/6)

Z80 cpu;
uint8_t interrupt_vector;
uint8_t ROM[0x4000];
uint8_t VIDEO_RAM[0x400];
uint8_t RAM2[0x400];
uint8_t RAM3[0x400];

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

    uint8_t romchars[0x1000];
    load_file(romchars, "roms/pacman.5e");
    decode_chars(romchars);
}

void WrZ80(register word Addr,register byte Value)
{
    Addr &= 0x7FFF;
    if (Addr < 0x4000) { fprintf(stdout, "[CPU][PC=%04x] writing to ROM %04hx: %02hhx\n", cpu.PC.W-1, Addr, Value); return; }
    if (Addr < 0x4400) { VIDEO_RAM[Addr-0x4000] = Value; return; }
    if (Addr < 0x4800) { RAM2[Addr-0x4400] = Value; return; }
    if (Addr < 0x4C00) { goto unknown; }
    if (Addr < 0x5000) { RAM3[Addr-0x4C00] = Value; return; }
    if (Addr == 0x50C0) { /* watchdog */ return; }
unknown:
    fprintf(stdout, "[CPU][PC=%04x] unknown write at %04hx: %02hhx\n", cpu.PC.W-1, Addr, Value);
}

byte RdZ80(register word Addr)
{
    Addr &= 0x7FFF;
    if (Addr < 0x4000) return ROM[Addr];
    if (Addr < 0x4400) return VIDEO_RAM[Addr-0x4000];
    if (Addr < 0x4800) return RAM2[Addr-0x4400];
    if (Addr < 0x4C00) { goto unknown; }
    if (Addr < 0x5000) return RAM3[Addr-0x4C00];
unknown:
    fprintf(stdout, "[CPU][PC=%04x] unknown read at %04hx\n", cpu.PC.W-1, Addr);
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
    fprintf(stdout, "[MEM][PC=%04x] unknown I/O write at %04hx: %02hhx\n", cpu.PC.W-1, Port, Value);
}

byte InZ80(register word Port)
{
    Port &= 0xFF;
    fprintf(stdout, "[MEM][PC=%04x] unknown I/O read at %04hx\n", cpu.PC.W-1, Port);
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
        fprintf(stdout, "[CPU][PC=%04x] VSync\n", cpu.PC.W-1);

        IntZ80(&cpu, interrupt_vector);

        uint8_t *screen;
        int pitch;

        hw_beginframe(&screen, &pitch);
        gfx_draw(screen, pitch);
        hw_endframe();
    }

    return 0;
}
