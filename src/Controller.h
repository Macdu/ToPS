#pragma once

#include "definitions.h"

#include <queue>

class Emulator;
class Interrupt;

enum ControllerKey {
	Select = 0,
	Start = 3,
	Up = 4,
	Right = 5,
	Down = 6,
	Left = 7,
	L2 = 8,
	R2 = 9,
	L1 = 10,
	R1 = 11,
	Triangle = 12,
	Circle = 13,
	Cross = 14,
	Square = 15
};

// Emulates a PSX digital controller
class Controller {

private:
	// points to the CPU clock cycle
	u64* clockCycle;

	Interrupt* interrupt;

	// JOY_STAT 32 bit register, read-only at 0x1F801044
	union 
	{
		struct {
			u32 isTransferReadyOrStarted : 1;
			u32 isFifoNotEmpty : 1;
			u32 isTransferReadyOrFinished : 1;
			u32 hasRXParityError : 1;
			u32 _useless1 : 3;
			u32 isACKInputLow : 1;
			u32 hasIRQ7 : 1;
			u32 _useless2 : 1;
			// 21bit timer, decrementing at 33MHz
			u32 baudrateTimer : 21; 
		} content;

		u32 val;
	} joyStat;

	static_assert(sizeof(joyStat) == sizeof(u32));

	// JOY_MOD 16 bit register, at 0x1F801048
	union {
		struct {
			// 1=MUL1, 2=MUL16, 3=MUL64 (or 0=MUL1, too)
			u8 baudrateReloadFactor : 2;
			// 0=5bits, 1=6bits, 2=7bits, 3=8bits
			u8 characterLength : 2;
			bool isParityEnabled : 1;
			bool isParityTypeOdd : 1;
			bool _useless : 2;
			bool isCLKOutputPolarityInverse : 1;
		} content;

		u16 val;
	} joyMode;

	static_assert(sizeof(joyMode) == sizeof(u16));

	// JOY_CTRL 16 bit register, at 0x1F80104A
	union
	{
		struct
		{
			bool isTransferEnabled : 1;
			bool joyPadSelect : 1;
			bool isRXNotNormal : 1;
			bool _useless1 : 1;
			bool acknowledIRQ : 1;
			bool _useless2 : 1;
			bool resetRegs : 1;
			bool _useless3 : 1;
			// 0..3 = IRQ when RX FIFO contains 1,2,4,8 bytes
			u8 interruptRXAmount : 2;
			bool isTransferInterruptEnabled : 1;
			bool isReceptionInterruptEnabled : 1;
			bool isACKInterruptEnabled : 1;
			// 0=/JOY1, 1=/JOY2 (set to LOW when Bit1=1)
			bool isSecondPad : 1;
		} content;

		u16 val;
	} joyControl;

	static_assert(sizeof(joyControl) == sizeof(u16));

	// contains the state of all button on the gamepad (0=released, 1=pushed)
	union
	{
		struct {
			bool select : 1;
			u8 always1 : 2;
			bool start : 1;
			bool up : 1;
			bool right : 1;
			bool left : 1;
			bool down : 1;
			bool l2 : 1;
			bool r2 : 1;
			bool l1 : 1;
			bool r1 : 1;
			bool triangle : 1;
			bool circle : 1;
			bool cross : 1;
			bool square : 1;
		} content;

		u16 val;
	} controllerState;

	static_assert(sizeof(controllerState) == sizeof(u16));

	// byte to be sent when transfer enabled
	u8 transferBuffer;
	bool isTransferBufferFilled;
	bool isExchanging;

	// contains the data wich will be sent back to the cpu
	std::queue<u8> receptionBuffer;

	// The controller needs to wait 100 cycles before receiving the data after a transfer was made
	// this variable contains the cycle to wait before throwing the IRQ
	u64 nextCycleIRQ;

	// function called when the transfer buffer is filled and transfer is enabled
	void sendTransferBuffer();

public:
	void init(Emulator* emu);
	void setJoyControl(u16 value);
	u16 getJoyControl() { return joyControl.val; };
	u32 getJoyStat() { 
		u32 res = joyStat.val; 
		joyStat.content.isACKInputLow = false;
		return res;
	};
	void handleInput(ControllerKey key, bool isPressed);
	// checks if an irq needs to be set
	void checkIRQ();
	// return data sent back from the controller
	u8 readData();
	// send data to the controller
	void sendData(u8 data);
};