#ifndef _MMARK13FRAME_H_
#define _MMARK13FRAME_H_

#include "tizenx.h"

class MMark13Frame
	: public Tizen::Ui::Controls::Frame
{
public:
	MMark13Frame(void);
	virtual ~MMark13Frame(void);

public:
	virtual result OnInitializing(void);
	virtual result OnTerminating(void);
};

#endif  //_MMARK13FRAME_H_
