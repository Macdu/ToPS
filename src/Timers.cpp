#include "Timers.h"

#include "Interrupt.h"
#include "gpu/GPU.h"

void Timers::init(Interrupt* interrupt, GPU* gpu)
{
	this->interrupt = interrupt;
	this->gpu = gpu;
}

void Timers::step(int stepCount, int stepsSinceScanline)
{
	u32 cpuCount = (u32)stepCount;
	u32 gpuCount = (u32)(cpuCount * 11) / 7;

	// an HBlank triggers about after 75% of the CPU cycles of the scanline are spent
	// regardless of the horizontal resolution
	constexpr int limitForHBlank = (GPU::cyclesPerScanline * 7 * 3) / (11 * 4);
	if (stepsSinceScanline >= limitForHBlank
		&& (stepsSinceScanline - stepCount) < limitForHBlank) {
		// we just entered HBlank
		isHBlank = true;
		// this can have an effect on timer0 and timer1
		if (counterModes[0].content.doSynchronise) {
			switch (counterModes[0].content.syncMode)
			{
			case 0:
				isPaused[0] = true;
				break;
			case 1:
			case 2:
				counters[0] = 0;
				break;
			case 3:
				// Pause until Hblank occurs once, then switch to Free Run
				isPaused[0] = false;
				counterModes[0].content.doSynchronise = false;
				break;
			}
		}

		// timer1 can have his clock based on HBlanks
		u8 source = counterModes[1].content.clockSource;
		if (!isPaused[1]
			&& (source == 1 || source == 3)) {
			counters[1]++;
		}
	}

	for (int i = 0; i < 3; i++) {
		if (isPaused[i])continue;
		auto mode = &counterModes[i].content;
		u8 source = mode->clockSource;
		if (source == 0
			|| (i != 3 && source == 2)
			|| (i == 3 && source == 3)) {
			// clock source is system clock
			counters[i] += cpuCount;
		}
		else {
			switch (i)
			{
			case 0:
				// dotClock
				counters[1] += gpuCount / getDotClockDivideFactor();
				break;
			case 1:
				// HBlank, do nothing here
				break;
			case 2:
				// systemClock / 8
				counters[i] += cpuCount / 8;
				break;
			}
		}
		bool shouldIRQ = false;
		// might lead to problems if the counter is manually set to a value above 
		// the target
		if (counters[i] > targets[i]) {
			if (!mode->hasReachedTarget
				&& mode->irqWhenTargetReached) {
				mode->hasReachedTarget = true;
				shouldIRQ = true;
			}

			if (mode->whenToReset == WhenToReset::AfterTarget) {
				// reset the counter after it reached the target
				counters[i] %= targets[i];
			}
			else if(counters[i] > 0xFFFF) {
				if (!mode->hasOverflowed
					&& mode->irqWhenTargetReached) {
					shouldIRQ = true;
				}

				mode->hasOverflowed = true;
				counters[i] &= 0xFFFF;
			}
		}
		// if one of the conditons to trigger an IRQ was fulfilled
		shouldIRQ = shouldIRQ 
			&& (mode->repeatMode == IRQRepeatMode::Repeat || !wasIRQTriggered[i]);
		if (shouldIRQ) {
			if (mode->interruptRequest) {
				mode->interruptRequest = false;
				wasIRQTriggered[i] = true;
				InterruptType type = (InterruptType)((u8)InterruptType::iTMR0 + i);
				interrupt->requestInterrupt(type);

			} else if (mode->irqPulseMode == IRQPulseMode::TogglePulse) {
				// Still flip the bit
				mode->interruptRequest = true;
			}
		}
	}
}

void Timers::setReg(u32 offset, u16 val)
{
	u32 timer = (offset >> 4) & 3;
	assert(timer < 3);
	switch ((offset >> 2) & 3)
	{
	case 0:
		// // Current Counter Value
		counters[timer] = (u32)val;
		break;
	case 1: {
		// Counter Mode
		counterModes[timer].val = (u32)val;
		// set interrupt bit to 1 on write
		counterModes[timer].content.interruptRequest = true;
		// reset for oneShot mode
		wasIRQTriggered[timer] = false;
		// reset the counter
		counters[timer] = 0;
		// manually update the isPaused value
		if (!counterModes[timer].content.doSynchronise) {
			isPaused[timer] = false;
		}
		else {
			u8 syncMode = counterModes[timer].content.syncMode;
			switch (timer)
			{
			case 0:
				// isPaused depends whether or not we are inside of HBlank
				switch (syncMode)
				{
				case 0:
					isPaused[0] = isHBlank;
					break;
				case 1:
					isPaused[0] = false;
					break;
				case 2:
					isPaused[0] = !isHBlank;
					break;
				case 3:
					isPaused[0] = true;
				}
				break;
			case 1:
				// isPaused depends whether or not we are inside of VBlank
				switch (syncMode)
				{
				case 0:
					isPaused[1] = isVBlank;
					break;
				case 1:
					isPaused[1] = false;
					break;
				case 2:
					isPaused[1] = !isVBlank;
					break;
				case 3:
					isPaused[1] = true;
					break;
				}
				break;
			case 2:
				switch (syncMode)
				{
				case 0:
				case 3:
					isPaused[2] = true;
					break;
				default:
					isPaused[2] = false;
					break;
				}
				break;
			}
		}
		break;
	}
	case 2:
		// Counter target value
		targets[timer] = (u32)val;
		break;
	default:
		throw_error("Undefined timer register write!");
		break;
	}
}

u16 Timers::getReg(u32 offset)
{
	u32 timer = (offset >> 4) & 3;
	assert(timer != 3);
	switch ((offset >> 2) & 3) {
	case 0:
		// Current Counter Value
		return (u16)counters[timer];
	case 1: {
		// Counter Mode
		u16 res = counterModes[timer].val;
		// reset reached values
		counterModes[timer].content.hasOverflowed = false;
		counterModes[timer].content.hasReachedTarget = false;
		return res;
	}
	case 3:
		// Counter target value
		return (u16)targets[timer];
	default:
		throw_error("Undefined timer register access!");
	}
}

void Timers::newScanline()
{
	isHBlank = false;
	// timer0 is affected by this event
	if (!counterModes[0].content.doSynchronise)return;
	switch (counterModes[0].content.syncMode)
	{
	case 0:
		// paused during HBlank
		isPaused[0] = false;
		break;
	case 1:
		// paused outside of HBlank
		isPaused[0] = true;
		break;
	}
}

void Timers::vBlankReached()
{
	isVBlank = true;
	// timer1 is affected by this event
	if (!counterModes[1].content.doSynchronise)return;
	switch (counterModes[1].content.syncMode) {
	case 0:
		isPaused[1] = true;
		break;
	case 1:
	case 2:
		counters[1] = 0;
		break;
	case 3:
		// Pause until Vblank occurs once, then switch to Free Run
		isPaused[1] = false;
		counterModes[1].content.doSynchronise = false;
		break;
	}
}

void Timers::newFrame()
{
	isVBlank = false;
	// timer1 is affected by this event
	if (!counterModes[1].content.doSynchronise)return;
	switch (counterModes[1].content.syncMode)
	{
	case 0:
		// paused during VBlank
		isPaused[1] = false;
		break;
	case 2:
		// paused ouside VBlank
		isPaused[1] = true;
		break;
	}
}

inline u32 Timers::getDotClockDivideFactor()
{
	switch (gpu->getHorizontalRes())
	{
	case 320:
		return 8;
	case 640:
		return 4;
	case 256:
		return 10;
	case 512:
		return 5;
	case 368:
		return 7;
	}
}

