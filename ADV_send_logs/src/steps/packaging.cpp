#include "steps/packaging.h"

#include "fsm/context.h"
#include "fsm/fsm.h"
#include "infra/file_utils.h"

Event handleZipLogs(Context& ctx) {
    auto res = zipDir(ctx.logsDir, ctx.zipPath, false);
    if (res) {
        return Event::ZipOk;
    }
    return Event::ZipCantCreate;
}
