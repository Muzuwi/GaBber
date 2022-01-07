#include <gba.h>
#include <cstdlib>

constexpr const unsigned particle_count = 100;

struct Particle {
	int32_t x;
	int32_t y;
	int32_t velx;
	int32_t vely;
	uint16_t color;

	void update() {
		x = x + velx;
		y = y + vely;
		if (x < 0) {
			x = 0;
			velx = -1 * velx;
		} else if (x >= SCREEN_WIDTH) {
			x = SCREEN_WIDTH;
			velx = -1 * velx;
		}
		if (y < 0) {
			y = 0;
			vely = -1 * vely;
		} else if (y >= SCREEN_HEIGHT) {
			y = SCREEN_HEIGHT;
			vely = -1 * vely;
		}
	}
};

static Particle s_particles[particle_count] {};

void dma_screen_blank() {
	static uint32_t zero = 0x00000000;

	REG_DMA0SAD = reinterpret_cast<uint32_t>(&zero);
	REG_DMA0DAD = reinterpret_cast<uint32_t>(MODE3_FB);
	REG_DMA0CNT = DMA_ENABLE |
	              DMA_IMMEDIATE |
	              DMA32 |
	              DMA_DST_INC |
	              DMA_SRC_FIXED |
	              0x0;
	asm("nop");

	REG_DMA0SAD = reinterpret_cast<uint32_t>(&zero);
	REG_DMA0DAD = 0x6010000;
	REG_DMA0CNT = DMA_ENABLE |
	              DMA_IMMEDIATE |
	              DMA32 |
	              DMA_DST_INC |
	              DMA_SRC_FIXED |
	              0xB00;
	asm("nop");
}

void generate_particles(Particle particles[particle_count], unsigned int seed) {
	std::srand(seed);

	for(unsigned i = 0; i < particle_count; ++i) {
		Particle& particle = particles[i];

		particle.x = std::rand() % SCREEN_WIDTH;
		particle.y = std::rand() % SCREEN_HEIGHT;
		particle.velx = (int32_t)(std::rand() % 8) - 4;
		particle.vely = (int32_t)(std::rand() % 8) - 4;
		particle.color = std::rand() % 0x10000;
	}
}

int main() {
	irqInit();
	irqEnable(IRQ_VBLANK);
	REG_DISPCNT = MODE_3 | BG2_ENABLE;
	REG_TM0CNT_H = TIMER_START | 0b11;  //  11b -> timer at 16384 Hz

	generate_particles(s_particles, 0xba3c1d4f);

	while (true) {
		VBlankIntrWait();
		scanKeys();
		dma_screen_blank();

		for(auto& particle : s_particles) {
			particle.update();
			MODE3_FB[particle.y][particle.x] = particle.color;
		}

		const auto pressed = keysDown();
		if(pressed & KEY_A) {
			const uint32_t random_value = REG_TM0CNT;
			generate_particles(s_particles, random_value);
		}
	}
	return 0;
}
