#include <stdio.h>
#include <string.h>

#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/maple/keyboard.h>
#include <dc/maple/mouse.h>
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

static void handleController(maple_device_t *dev, int Cnt)
{
	Event ev;

	JoyState& state = (Cnt == 0) ? joy1state : joy2state;

	cont_cond_t *pad = (cont_cond_t *)maple_dev_status(dev);
  
	state.joyx = pad->joyx;
	state.joyy = pad->joyy;
	state.rtrigger = pad->rtrig;
	state.ltrigger = pad->ltrig;

	unsigned short button = pad->buttons & 0xffff;
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

static void handleKeyboard(maple_device_t *dev)
{
	Event ev;
	KeyState& state = keystate;

	kbd_cond_t *pad = (kbd_cond_t *)maple_dev_status(dev);

	unsigned char shift = pad->modifiers;
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

  
	newkey = pad->keys;

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

		begin = &pad->keys[0];
		end = &pad->keys[6];

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
  
	memcpy(state.key, pad->keys, sizeof(state.key));
}

void handleMouse(maple_device_t *dev)
{
	Event ev;

	MouseState& state = mousestate;

	mouse_cond_t *pad = (mouse_cond_t *)maple_dev_status(dev);

	state.axis1 = pad->dx;
	state.axis2 = pad->dy;
	state.axis3 = pad->dz;

	if (state.axis1 || state.axis2) {
		ev.type = EVENT_MOUSEMOTION;
		ev.motion.x = state.axis1;
		ev.motion.y = state.axis2;
		event->push(ev);
	}

	pad->dx = 0;
	pad->dy = 0;
	pad->dz = 0;

  
	int button = ~pad->buttons;
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

void handleInput()
{
	int  JoyCount = 0;
	int  KeyboardCount = 0;
	int  MouseCount = 0;
	maple_device_t *dev;

	for (int i=0; i<4; ++i) {
	  dev = maple_enum_dev(i, 0);
	  if(!dev) continue;
    
	  if (dev->info.functions & MAPLE_FUNC_CONTROLLER) {
	    if (JoyCount < 2) {
	      handleController(dev, JoyCount);
	    }
	    ++JoyCount;
	  } else if ((dev->info.functions & MAPLE_FUNC_KEYBOARD) && !KeyboardCount) {
	    handleKeyboard(dev);
	    ++KeyboardCount;
	  } else if ((dev->info.functions & MAPLE_FUNC_MOUSE) && !MouseCount) {
	    handleMouse(dev);
	    ++MouseCount;
	  }
	}
}

bool PollEvent(Event& ev)
{
	handleInput();
  
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

