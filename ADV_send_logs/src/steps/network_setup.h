#pragma once

#include "fsm/fsm.h"
#include "fsm/context.h"
#include <httplib.h>

Event handleUploadToServer(Context& ctx);

Event map_httplib_error(const httplib::Error err);
Event map_httplib_http_response(int status);
