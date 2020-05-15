#pragma once

#include "definitions.h"

class Interrupt;
class GPU;

// A class to implement the 3 builtin timers in the PS1
class Timers {
public:
	// contains the 3 timers value (16-bit values)
	u32 counters[3];

	// contains the 3 target values that can be set for the timers (16-bit values)
	u32 targets[3];

	void init(Interrupt* interrupt, GPU* gpu);

	// Make the timers advance by the number of CPU steps given
	void step(int stepCount, int stepsSinceScanline);

	void setReg(u32 offset, u16 val);
	u16 getReg(u32 offset);

	// functions that are called by Emulator.cpp to update some timers states
	void newScanline();
	void vBlankReached();
	void newFrame();

private:
	// the GPU clock is a multiple of the dotclock depending on the horizontal res
	inline u32 getDotClockDivideFactor();

	enum struct WhenToReset : u8 {
		AfterOverflow = 0,
		AfterTarget = 1
	};

	enum struct RepeatMode : u8 {
		OneShot = 0,
		Repeatedly = 1
	};

	enum struct IRQPulseMode : u8 {
		// Bit10 is permanently set, except for a few clock cycles when an IRQ occurs
		ShortPulse = 0,
		// In Toggle mode, Bit10 is set after writing to the Mode register, 
		// and becomes inverted on each IRQ (in one-shot mode, it remains zero after the IRQ) 
		// (in repeat mode it inverts Bit10 on each IRQ, so IRQ4/5/6 are triggered only each 2nd time, 
		// ie. when Bit10 changes from 1 to 0).
		TogglePulse = 1
	};

	enum struct IRQRepeatMode : u8 {
		// In once mode, the IRQ is pulsed/toggled only once (one-shot mode doesn't stop the counter, 
		// it just suppresses any further IRQs until a new write to the Mode register occurs; 
		// if both IRQ conditions are enabled in Bit4-5, 
		// then one-shot mode triggers only one of those conditions; whichever occurs first).
		Once = 0,
		Repeat = 1
	};

	union {
		struct {
			bool doSynchronise : 1;
			u8 syncMode : 2;
			WhenToReset whenToReset : 1;
			bool irqWhenTargetReached : 1;
			bool irqWhenOverflow : 1;
			IRQRepeatMode repeatMode : 1;
			IRQPulseMode irqPulseMode : 1;
			u8 clockSource : 2;
			// (0=Yes, 1=No) (Set after Writing to 1)
			bool interruptRequest : 1;
			// Reset when reading
			bool hasReachedTarget : 1;
			// Reset when reading
			bool hasOverflowed : 1;

		} content;

		u16 val;
	} counterModes[3];

	static_assert(sizeof(counterModes[0]) == sizeof(u16));

	// specify if the timer is paused
	bool isPaused[3];

	// specify if an IRQ has already been triggered
	bool wasIRQTriggered[3];

	bool isVBlank;
	bool isHBlank;

	Interrupt* interrupt;
	GPU* gpu;
};