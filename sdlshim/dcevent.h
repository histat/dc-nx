  
enum EventType {
	EVENT_KEYDOWN = 1,
	EVENT_KEYUP,
	EVENT_MOUSEMOTION,
	EVENT_MOUSEBUTTONDOWN,
	EVENT_MOUSEBUTTONUP,
	EVENT_JOYAXISMOTION,
	EVENT_JOYBUTTONDOWN,
	EVENT_JOYBUTTONUP,
	EVENT_QUIT
};

enum {
	EVENT_BUTTON_MIDDLE = 1 << 0,
	EVENT_BUTTON_RIGHT = 1 << 1,
	EVENT_BUTTON_LEFT = 1 << 2
};

enum {
	KBD_LCTRL =  0x90,
	KBD_LSFT,
	KBD_LALT,
	KBD_S1,
	KBD_RCTRL,
	KBD_RSFT,
	KBD_RALT,
	KBD_S2
};

enum {
	EVENT_PRESSED = 1,
	EVENT_RELEASED,
};

enum {
	JOY_C			= 1 << 0,
	JOY_B			= 1 << 1,
	JOY_A			= 1 << 2,
	JOY_START		= 1 << 3,
	JOY_UP		= 1 << 4,
	JOY_DOWN		= 1 << 5,
	JOY_LEFT		= 1 << 6,
	JOY_RIGHT		= 1 << 7,
	JOY_Z			= 1 << 8,
	JOY_Y			= 1 << 9,
	JOY_X			= 1 << 10,
	JOY_D			= 1 << 11,
	JOY_RTRIGGER		= 1 << 12,
	JOY_LTRIGGER		= 1 << 13,
};


struct EventKeyboardEvent {
	EventType	type;
	unsigned char keycode;
};

struct EventMouseMotionEvent {
	EventType	type;
	short x;
	short y;
};

struct EventMouseButtonEvent {
	EventType	type;
	unsigned char	button;
};

struct EventJoyAxisEvent {
	EventType	type;
	char which;
	char axis;
	short value;
};

struct EventJoyButtonEvent {
	EventType	type;
	char which;
	short button;
	short state;
};

union Event {
	EventType	type;
	EventKeyboardEvent		key;
	EventMouseMotionEvent	motion;
	EventMouseButtonEvent	button;
	EventJoyAxisEvent 	jaxis;
	EventJoyButtonEvent	jbutton;
};


enum JOYID {
	JOYSTICKID1 = 0,
	JOYSTICKID2,
};

struct JoyInfo {
	char x;
	char y;
	unsigned int button;
};



#ifdef __cplusplus
extern "C" {
#endif

	void handleInput(struct mapledev *pad);
	bool PollEvent(Event& ev);
	bool getMousePos(short *x, short *y);
	bool getMouseButton(char *button);
	bool getJoyAxis(JOYID id, char *x, char *y);
	bool getJoyButton(JOYID id, unsigned short *button);

#ifdef __cplusplus
}
#endif
