#pragma once

#include "fsm/context.h"
#include "fsm/fsm.h"

Event handleFindHome(Context& ctx);
Event handleLocateLogs(Context& ctx);
Event handleReadPath(Context& ctx);
