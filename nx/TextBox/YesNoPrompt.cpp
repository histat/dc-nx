
#include "../nx.h"
#include "YesNoPrompt.h"
#include "YesNoPrompt.fdh"

#ifdef STATE_WAIT
#undef STATE_WAIT
#endif

enum
{
	STATE_APPEAR,
	STATE_WAIT,
	STATE_YES_SELECTED,
	STATE_NO_SELECTED
};

#define YESNO_X				216
#define YESNO_Y				144
#define YESNO_POP_SPEED		4

/*
void c------------------------------() {}
*/

void TB_YNJPrompt::ResetState()
{
	fVisible = false;
	fAnswer = -1;
}

void TB_YNJPrompt::SetVisible(bool enable)
{
	fVisible = enable;
	
	if (fVisible)
	{
		fState = STATE_APPEAR;
		fCoords.y = YESNO_Y + (YESNO_POP_SPEED * 2);
		fAnswer = -1;
		
		sound(SND_MENU_PROMPT);
	}
}

/*
void c------------------------------() {}
*/

void TB_YNJPrompt::Draw()
{
	if (!fVisible)
		return;
	
	draw_sprite(YESNO_X, fCoords.y, SPR_YESNO, 0, 0);
	
	// draw hand selector
	if (fState == STATE_YES_SELECTED || \
		fState == STATE_NO_SELECTED)
	{
		int xoff = (fState == STATE_YES_SELECTED) ? -4 : 37;
		draw_sprite(YESNO_X+xoff, fCoords.y+12, SPR_YESNOHAND, 0, 0);
	}
	
	switch(fState)
	{
		case STATE_APPEAR:
		{
			fCoords.y -= YESNO_POP_SPEED;
			
			if (fCoords.y <= YESNO_Y)
			{
				fCoords.y = YESNO_Y;
				fState = STATE_WAIT;
				fTimer = 15;
			}
			else break;
		}
		case STATE_WAIT:
		{
			if (fTimer)
			{
				fTimer--;
				break;
			}
			
			fState = STATE_YES_SELECTED;
		}
		break;
		
		case STATE_YES_SELECTED:
		case STATE_NO_SELECTED:
		{
			if (justpushed(LEFTKEY) || justpushed(RIGHTKEY))
			{
				sound(SND_MENU_MOVE);
				
				fState = (fState == STATE_YES_SELECTED) ?
							STATE_NO_SELECTED : STATE_YES_SELECTED;
			}
			
			if (justpushed(JUMPKEY))
			{
				sound(SND_MENU_SELECT);
				lastinputs[JUMPKEY] = true;
				lastpinputs[JUMPKEY] = true;
				
				fAnswer = (fState == STATE_YES_SELECTED) ? YES : NO;
				SetVisible(false);
			}
		}
		break;
	}
}

/*
void c------------------------------() {}
*/

bool TB_YNJPrompt::ResultReady()
{
	return (fAnswer != -1);
}

int TB_YNJPrompt::GetResult()
{
	return fAnswer;
}

