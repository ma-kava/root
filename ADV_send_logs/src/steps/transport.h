#pragma once

#include "fsm/context.h"
#include "fsm/fsm.h"

Event handleTransport(Context& ctx);
Event handleServerResponse(Context& ctx);
