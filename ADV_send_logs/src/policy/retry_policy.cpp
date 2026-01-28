#include "policy/retry_policy.h"

#include "fsm/fsm.h"
#include "fsm/context.h"

Event handleRetryPolicy(Context& ctx) {
    ctx.increment_reconnect_counter();
    if (ctx.is_reconnect_count_limit_reached()) {
        return Event::Abort;
    }
    return Event::Reconnect;
}
