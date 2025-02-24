//
// Created by ulrich on 08.04.19.
//

#include <gui/widgets/XournalWidget.h>
#include "MouseInputHandler.h"
#include "InputContext.h"
#include <gui/XournalppCursor.h>

MouseInputHandler::MouseInputHandler(InputContext* inputContext) : PenInputHandler(inputContext)
{
	XOJ_INIT_TYPE(MouseInputHandler);
}

MouseInputHandler::~MouseInputHandler()
{
	XOJ_CHECK_TYPE(MouseInputHandler);

	XOJ_RELEASE_TYPE(MouseInputHandler);
}

bool MouseInputHandler::handleImpl(InputEvent* event)
{
	XOJ_CHECK_TYPE(MouseInputHandler);

	// Only handle events when there is no active gesture
	GtkXournal* xournal = inputContext->getXournal();
	if (xournal->view->getControl()->getWindow()->isGestureActive())
	{
		// Do not further relay events as they are of no interest
		return true;
	}

	// Determine the pressed states of devices and associate them to the current event
	setPressedState(event);

	/*
	 * Trigger start action
	 */
	// Trigger start of action when pen/mouse is pressed
	if (event->type == BUTTON_PRESS_EVENT)
	{
		this->actionStart(event);
		return true;
	}

	if (event->type == BUTTON_2_PRESS_EVENT || event->type == BUTTON_3_PRESS_EVENT)
	{
		this->actionPerform(event);
		return true;
	}

	/*
	 * Trigger motion actions
	 */
	// Trigger motion action when pen/mouse is pressed and moved
	if (event->type == MOTION_EVENT) //mouse or pen moved
	{
		if (this->deviceClassPressed)
		{
			this->actionMotion(event);
		}
		else
		{
			XournalppCursor* cursor = xournal->view->getCursor();
			cursor->setTempCursor(GDK_ARROW);
		}
		// Update cursor visibility
		xournal->view->getCursor()->setInvisible(false);
	}

	// Notify if mouse enters/leaves widget
	if (event->type == ENTER_EVENT)
	{
		this->actionEnterWindow(event);
	}
	if (event->type == LEAVE_EVENT)
	{
		//this->inputContext->unblockDevice(InputContext::TOUCHSCREEN);
		//this->inputContext->getView()->getHandRecognition()->unblock();
		this->actionLeaveWindow(event);
	}

	// Trigger end of action if mouse button is released
	if (event->type == BUTTON_RELEASE_EVENT)
	{
		this->actionEnd(event);
		return true;
	}

	// If we loose our Grab on the device end the current action
	if (event->type == GRAB_BROKEN_EVENT && this->deviceClassPressed)
	{
		// TODO: We may need to update pressed state manually here
		this->actionEnd(event);
		return true;
	}

	return false;
}

void MouseInputHandler::setPressedState(InputEvent* event)
{
	XOJ_CHECK_TYPE(MouseInputHandler);

	XojPageView* currentPage = getPageAtCurrentPosition(event);

	this->inputContext->getXournal()->view->getCursor()->setInsidePage(currentPage != nullptr);

	if (event->type == BUTTON_PRESS_EVENT) //mouse button pressed or pen touching surface
	{
		this->deviceClassPressed = true;

		switch (event->button)
		{
			case 2:
				this->modifier2 = true;
				break;
			case 3:
				this->modifier3 = true;
			default:
				break;
		}
	}
	if (event->type == BUTTON_RELEASE_EVENT) //mouse button released or pen not touching surface anymore
	{
		this->deviceClassPressed = false;

		switch (event->button)
		{
			case 2:
				this->modifier2 = false;
				break;
			case 3:
				this->modifier3 = false;
			default:
				break;
		}
	}
}

bool MouseInputHandler::changeTool(InputEvent* event)
{
	XOJ_CHECK_TYPE(MouseInputHandler);

	Settings* settings = this->inputContext->getSettings();
	ToolHandler* toolHandler = this->inputContext->getToolHandler();
	GtkXournal* xournal = this->inputContext->getXournal();

	ButtonConfig* cfg = nullptr;
	if (modifier2 /* Middle Button */ && !xournal->selection)
	{
		cfg = settings->getMiddleButtonConfig();
	}
	else if (modifier3 /* Right Button */ && !xournal->selection)
	{
		cfg = settings->getRightButtonConfig();
	}

	if (cfg && cfg->getAction() != TOOL_NONE)
	{
		toolHandler->copyCurrentConfig();
		cfg->acceptActions(toolHandler);
	}
	else
	{
		toolHandler->restoreLastConfig();
	}

	return false;
}

void MouseInputHandler::onBlock()
{
	XOJ_CHECK_TYPE(MouseInputHandler);
}
