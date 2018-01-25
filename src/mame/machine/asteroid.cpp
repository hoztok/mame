// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "machine/atari_vg.h"
#include "video/avgdvg.h"
#include "includes/asteroid.h"


INTERRUPT_GEN_MEMBER(asteroid_state::asteroid_interrupt)
{
	/* Turn off interrupts if self-test is enabled */
	if (!(ioport("IN0")->read() & 0x80))
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(asteroid_state::asterock_interrupt)
{
	/* Turn off interrupts if self-test is enabled */
	if ((ioport("IN0")->read() & 0x80))
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(asteroid_state::llander_interrupt)
{
	/* Turn off interrupts if self-test is enabled */
	if (ioport("IN0")->read() & 0x02)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

READ8_MEMBER(asteroid_state::asteroid_IN0_r)
{
	int res = ioport("IN0")->read();
	int bitmask = (1 << offset);

	if (res & bitmask)
		res = 0x80;
	else
		res = ~0x80;

	return res;
}


READ8_MEMBER(asteroid_state::asterock_IN0_r)
{
	int res = ioport("IN0")->read();
	int bitmask = (1 << offset);

	if (res & bitmask)
		res = ~0x80;
	else
		res = 0x80;

	return res;
}

/*
 * These 7 memory locations are used to read the player's controls.
 * Typically, only the high bit is used. This is handled by one input port.
 */

READ8_MEMBER(asteroid_state::asteroid_IN1_r)
{
	int res = ioport("IN1")->read();
	int bitmask = (1 << (offset & 0x7));

	if (res & bitmask)
		res = 0x80;
	else
		res = ~0x80;

	return res;
}


READ8_MEMBER(asteroid_state::asteroid_DSW1_r)
{
	int res;
	int res1;

	res1 = ioport("DSW1")->read();

	res = 0xfc | ((res1 >> (2 * (3 - (offset & 0x3)))) & 0x3);
	return res;
}


WRITE8_MEMBER(asteroid_state::asteroid_bank_switch_w)
{
	// 76------  not used
	// --5-----  coin counter right coin
	// ---4----  coin counter center coin
	// ----3---  coin counter left coin
	// -----2--  ramsel
	// ------1-  start2 led
	// -------0  start1 led

	start1_led_w(BIT(data, 0));
	start2_led_w(BIT(data, 1));

	int bank = BIT(data, 2);
	m_ram1->set_entry(bank);
	m_ram2->set_entry(bank);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 3));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 4));
	machine().bookkeeping().coin_counter_w(2, BIT(data, 5));
}


WRITE_LINE_MEMBER(asteroid_state::start1_led_w)
{
	output().set_led_value(0, state ? 0 : 1);
}

WRITE_LINE_MEMBER(asteroid_state::start2_led_w)
{
	output().set_led_value(1, state ? 0 : 1);
}

void asteroid_state::machine_start()
{
	/* configure RAM banks if present (not on llander) */
	if (m_ram1.target() != nullptr)
	{
		uint8_t *ram1 = reinterpret_cast<uint8_t *>(memshare("ram1")->ptr());
		uint8_t *ram2 = reinterpret_cast<uint8_t *>(memshare("ram2")->ptr());

		/* swapped */
		m_ram1->configure_entry(1, ram2);
		m_ram2->configure_entry(1, ram1);
		/* normal */
		m_ram1->configure_entry(0, ram1);
		m_ram2->configure_entry(0, ram2);
	}
}

void asteroid_state::machine_reset()
{
	m_dvg->reset_w(m_maincpu->space(AS_PROGRAM), 0, 0);

	/* reset RAM banks if present */
	if (m_ram1.target() != nullptr)
	{
		m_ram1->set_entry(0);
		m_ram2->set_entry(0);
	}
}
