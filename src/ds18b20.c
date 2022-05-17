#include "display.h"
#include "ds18b20.h"
#include "event.h"
#include "onewire.h"

#define SKIP_ROM		0xCC
#define START_CONVERSION	0x44
#define READ_SCRATCHPAD		0xBE

static enum {
	STATE_IDLE,
	STATE_REQUEST_RESET,
	STATE_REQUEST_XFER,
	STATE_RESPONSE_RESET,
	STATE_RESPONSE_XFER,
} state = STATE_IDLE;

void ds18b20_request_start (void)
{
	state = STATE_REQUEST_RESET;
	onewire_reset_start();
}

void ds18b20_response_start (void)
{
	state = STATE_RESPONSE_RESET;
	onewire_reset_start();
}

static void request_xfer_start (void)
{
	static const uint8_t cmd[] = {
		SKIP_ROM,
		START_CONVERSION,
	};

	state = STATE_REQUEST_XFER;
	onewire_xfer_start(cmd, sizeof (cmd));
}

static void response_xfer_start (void)
{
	static const uint8_t cmd[] = {
		SKIP_ROM,
		READ_SCRATCHPAD,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
	};

	state = STATE_RESPONSE_XFER;
	onewire_xfer_start(cmd, sizeof (cmd));
}

static void response_xfer_end (void)
{
	static struct {
		uint8_t cmd[2];
		int16_t temp;
		uint8_t th;
		uint8_t tl;
		uint8_t config;
		uint8_t reserved[3];
		uint8_t crc;
	} __attribute__((packed)) reply;

	onewire_xfer_end(reply.cmd, sizeof (reply));
	display_temperature(reply.temp);
	state = STATE_IDLE;
}

void ds18b20_handle_events (void)
{
	switch (state) {
	case STATE_REQUEST_RESET:
		if (event_test_and_clear(EVENT_ONEWIRE_RESET_DONE))
			onewire_reset_end()
				? request_xfer_start()
				: display_flash(DISPLAY_FLASH_SENSOR);
		break;

	case STATE_RESPONSE_RESET:
		if (event_test_and_clear(EVENT_ONEWIRE_RESET_DONE))
			onewire_reset_end()
				? response_xfer_start()
				: display_flash(DISPLAY_FLASH_SENSOR);
		break;

	case STATE_RESPONSE_XFER:
		if (event_test_and_clear(EVENT_ONEWIRE_XFER_DONE))
			response_xfer_end();
		break;

	default:
		break;
	}
}
