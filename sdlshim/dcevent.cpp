#include <stdio.h>
#include <string.h>
#include <ronin/maple.h>
#include <ronin/dc_time.h>
#include "dcevent.h"


class EventQueue {
public:
	EventQueue() { reset(); };
	EventQueue* operator->() { return this; }

	Event pop() { if (head >= QUEUE_SIZE) head = 0; return events[head++]; }
	void push(const Event& ev) { if (tail >= QUEUE_SIZE) tail = 0; events[tail++] = ev;}
	bool empty() const { return head == tail; }

	int size() const { return head; }
private:
	enum { QUEUE_SIZE = 32 };
	void reset() { head = tail = 0; }
	int head;
	int tail;
	Event events[QUEUE_SIZE];
};

static EventQueue event;

struct JoyState {
	unsigned short button;
	char joyx;
	char joyy;
	unsigned char rtrigger;
	unsigned char ltrigger;
};

struct KeyState {
	unsigned char shift;
	unsigned char key[6];
};

struct MouseState {
	unsigned char button;
	short axis1;
	short axis2;
	short axis3;
};

static JoyState joy1state;
static JoyState joy2state;
static KeyState  keystate;
static MouseState  mousestate;

static void handleController(struct mapledev *pad, int Cnt)
{
	Event ev;

	JoyState& state = (Cnt == 0) ? joy1state : joy2state;
  
	state.joyx = pad->cond.controller.joyx-128;
	state.joyy = pad->cond.controller.joyy-128;
	state.rtrigger = pad->cond.controller.rtrigger;
	state.ltrigger = pad->cond.controller.ltrigger;

	unsigned short button = ~pad->cond.controller.buttons & 0xffff;
	button |= (state.rtrigger?1:0)<<12 | (state.ltrigger?1:0)<<13;

	if (button == 0x060e) {
		ev.type = EVENT_QUIT;
		event->push(ev);
		return;
	}

	if (state.joyx) {
		ev.type = EVENT_JOYAXISMOTION;
		ev.jaxis.which = Cnt;
		ev.jaxis.axis = 0;
		ev.jaxis.value = state.joyx/16;
		event->push(ev);
	}
  
	if (state.joyy) {
		ev.type = EVENT_JOYAXISMOTION;
		ev.jaxis.which = Cnt;
		ev.jaxis.axis = 1;
		ev.jaxis.value = state.joyy/16;
		event->push(ev);
	}
  
	int diff = (button^state.button)&0xffff;
  
	if (diff) {
		for (int i=0; i<16; ++i) {
			if (diff & (1<<i)) {
				ev.type = (button & (1<<i)) ? EVENT_JOYBUTTONDOWN : EVENT_JOYBUTTONUP;
				ev.jbutton.which = Cnt;
				ev.jbutton.button = (1<<i);
				ev.jbutton.state = (button & (1<<i)) ? EVENT_PRESSED : EVENT_RELEASED;
				event->push(ev);
			}
		}
		state.button = button;
	}
}

static void handleKeyboard(struct mapledev *pad)
{
	Event ev;
	KeyState& state = keystate;

	unsigned char shift = pad->cond.kbd.shift;
	int diff = (shift^state.shift)&0xff;
  
	if (diff) {
		for (int i=0; i<8; ++i) {
			if (diff & (1<<i)) {
				ev.type = (shift & (1<<i)) ? EVENT_KEYDOWN : EVENT_KEYUP;
				ev.key.keycode = KBD_LCTRL + i;
				event->push(ev);
			}
		}
		state.shift = shift;
	}

	unsigned char *newkey;
	unsigned char *oldkey;

	unsigned char *begin;
	unsigned char *end;

  
	newkey = pad->cond.kbd.key;

	for (int i=0; i<6; ++i, ++newkey) {
		if ((*newkey >= 2) && (*newkey <= 0x9f)) {
      
			begin = &state.key[0];
			end = &state.key[6];
			while (begin != end) {
				if (*begin == *newkey)
					break;

				++begin;
			}

			if (begin == end) {
				ev.type = EVENT_KEYDOWN;
				ev.key.keycode = *newkey;
				event->push(ev);
			}

		} else
			*newkey = 0;
	}

	oldkey = state.key;
  
	for (int i=0; i<6; ++i, ++oldkey) {

		begin = &pad->cond.kbd.key[0];
		end = &pad->cond.kbd.key[6];

		while (begin != end) {
			if (*begin == *oldkey)
				break;
	
			++begin;
		}
    
		if (begin == end) {
			ev.type = EVENT_KEYUP;
			ev.key.keycode = *oldkey;
			event->push(ev);
		}
	}
  
	memcpy(state.key, pad->cond.kbd.key, sizeof(state.key));
}

void handleMouse(struct mapledev *pad)
{
	Event ev;

	MouseState& state = mousestate;

	state.axis1 = pad->cond.mouse.axis1;
	state.axis2 = pad->cond.mouse.axis2;
	state.axis3 = pad->cond.mouse.axis3;

	if (state.axis1 || state.axis2) {
		ev.type = EVENT_MOUSEMOTION;
		ev.motion.x = state.axis1;
		ev.motion.y = state.axis2;
		event->push(ev);
	}

	pad->cond.mouse.axis1 = 0;
	pad->cond.mouse.axis2 = 0;
	pad->cond.mouse.axis3 = 0;

  
	int button = ~pad->cond.mouse.buttons;
	int diff = (button ^ state.button) & 0xff;
  
	if (diff) {
		for (int i=0; i<8; ++i) {
			if (diff & (1<<i)) {
				ev.type = (button & (1<<i)) ? EVENT_MOUSEBUTTONDOWN : EVENT_MOUSEBUTTONUP;
				ev.button.button = (1<<i);
				event->push(ev);
			}
		}
		state.button = button;
	}
}
#ifndef __SDCARD__
int vmu_present[4];
#endif

void handleInput(struct mapledev *pad)
{
	int  JoyCount = 0;
	int  KeyboardCount = 0;
	int  MouseCount = 0;

	for (int i=0; i<4; ++i, ++pad) {
    
		if (pad->func & MAPLE_FUNC_CONTROLLER) {
			if (JoyCount < 2) {
				handleController(pad, JoyCount);
			}
			++JoyCount;
		} else if ((pad->func & MAPLE_FUNC_KEYBOARD) && !KeyboardCount) {
      
			handleKeyboard(pad);
			++KeyboardCount;
		} else if ((pad->func & MAPLE_FUNC_MOUSE) && !MouseCount) {
      
			handleMouse(pad);
			++MouseCount;
		}
#ifndef __SDCARD__
		vmu_present[i] = pad[i].present;
#endif
	}
}

bool PollEvent(Event& ev)
{
#if 0
	static  unsigned int tick = 0;
	unsigned int  tm = Timer() - tick;
	if (tm >= USEC_TO_TIMER(1000000/60)) {
		int mask = getimask();
		setimask(15);
		handleInput(locked_get_pads());
		setimask(mask);
		tick += tm;
	}
#endif
  
	if (event->empty())
		return false;
  
	ev = event->pop();
	return true;
}

bool getMousePos(short *x, short *y)
{
	MouseState& state = mousestate;

	*x = state.axis1;
	*y = state.axis2;

	if (state.axis1 || state.axis2) {
		return true;
	}

	return false;
}

bool getMouseButton(char *button)
{
	MouseState& state = mousestate;
  
	*button = state.button;
  
	if (state.button != 0) {
		return true;
	}
  
	return false;
}

bool getJoyAxis(JOYID id, char *x, char *y)
{
	JoyState& state = (id == 0) ? joy1state : joy2state;

	int axis1 = state.joyx/16;
	int axis2 = state.joyy/16;
  
	*x = axis1;
	*y = axis2;
  
	if (axis1 != 0 || axis2 != 0) {
		return true;
	}

	return false;
}

bool getJoyButton(JOYID id, unsigned short *button)
{
	JoyState& state = (id == 0) ? joy1state : joy2state;
  
	*button = state.button & 0xffff;

	if(button != 0) {
		return true;
	}

	return false;
}

