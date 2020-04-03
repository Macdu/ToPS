#include "Controller.h"

#include "Emulator.h"

void Controller::setJoyControl(u16 value) {

	bool wasTransferEnabled = joyControl.content.isTransferEnabled;

	joyControl.val = value;
	if (joyControl.content.acknowledIRQ) {
		joyStat.content.hasRXParityError = false;
		joyStat.content.hasIRQ7 = false;
		joyControl.content.acknowledIRQ = false;
	}

	if (!joyControl.content.joyPadSelect) {
		joyControl.content.isSecondPad = false;
	}

	if (isTransferBufferFilled
		&& !wasTransferEnabled
		&& joyControl.content.isTransferEnabled) {
		// if a byte was ready to be sent :
		sendTransferBuffer();
	}

	if (joyControl.content.resetRegs) {
		joyControl.val = 0;
		joyStat.val = 0;
		joyStat.content.isTransferReadyOrStarted = true;
		joyStat.content.isTransferReadyOrFinished = true;
	}
}

void Controller::sendTransferBuffer()
{
	assert(isTransferBufferFilled);
	assert(joyControl.content.isTransferEnabled);
	isTransferBufferFilled = false;
	isExchanging = true;
	if (receptionBuffer.empty()) {
		// start new emission
		if (transferBuffer == 0x01 && !joyControl.content.isSecondPad) {
			// controller transfer
			receptionBuffer.push(0xAA);
			receptionBuffer.push(0x41);
			receptionBuffer.push(0x5A);
			// 0 = pressed, 1 = released
			receptionBuffer.push(~controllerState.val);
			receptionBuffer.push(~(controllerState.val >> 8));
		}
		else if (transferBuffer == 0x81) {
			//throw_error("Memory card access not implemented!");
			// no memory card connected
			receptionBuffer.push(0xFF);
			receptionBuffer.push(0xFF);
		}
		else {
			// return some default value
			receptionBuffer.push(0xFF);
		}
	}
	// waits 400 cpu cycles before setting an IRQ
	// it is necessary to wait because this behavior is expected by the BIOS
	nextCycleIRQ = (*clockCycle) + 400;
}

void Controller::handleInput(ControllerKey key, bool isPressed)
{
	//printf("%d -> %d\n", (int)key, (int)isPressed);
	u16 correspondingBit = (1 << ((int)key));
	controllerState.val &= ~correspondingBit;
	controllerState.val |= correspondingBit * isPressed;
}

void Controller::checkIRQ()
{
	if (nextCycleIRQ < *clockCycle) {
		// we have an IRQ happening
		// reset the clock
		nextCycleIRQ = std::numeric_limits<u64>::max();
		// we have a byte in the FIFO
		joyStat.content.isFifoNotEmpty = true;
		if (receptionBuffer.size() != 1 && !joyStat.content.hasIRQ7 && joyControl.content.isACKInterruptEnabled) {
			// if the previous IRQ was acknowledged and IRQ are enabled
			joyStat.content.hasIRQ7 = true;
			interrupt->requestInterrupt(InterruptType::iController);
		}
	}
}

u8 Controller::readData()
{
	if (receptionBuffer.size() == 0 || !joyStat.content.isFifoNotEmpty) {
		// the program makes sure the reception buffer is flushed
		// (I think)
		return 0;
	}
	// Suppose there is only one byte at a time in the FIFO
	joyStat.content.isFifoNotEmpty = false;
	u8 data = receptionBuffer.front();
	receptionBuffer.pop();
	return data;
}

void Controller::sendData(u8 data)
{
	transferBuffer = data;
	isTransferBufferFilled = true;
	if (joyControl.content.isTransferEnabled) {
		sendTransferBuffer();
	}
}

void Controller::init(Emulator* emu)
{
	controllerState.content.always1 = 0b11;
	// transfer is instantaneous on this emulator
	joyStat.content.isTransferReadyOrStarted = true;
	joyStat.content.isTransferReadyOrFinished = true;
	clockCycle = &emu->getCPU()->clockCycle;
	nextCycleIRQ = std::numeric_limits<u64>::max();
	interrupt = emu->getInterrupt();
}