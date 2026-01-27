#include "steps/environment.h"

#include "fsm/context.h"
#include "fsm/fsm.h"
#include "infra/file_utils.h"

Event handleFindHome(Context& ctx) {
    fs::path path = get_home_path();

    if (path.c_str() == "") {
        return Event::HomeNotSet;
    }
    ctx.home = path;
    return Event::HomeSet;
}


Event handleLocateLogs(Context& ctx) {
#ifdef _WIN32

#else
    std::vector<fs::path> searchPaths = {
        fs::path(ctx.home),
        fs::path(ctx.home) / ".config"
    };
#endif

    fs::path result = findFolder(searchPaths, "PixetPro/logs");

    if (result.empty()) {
        return Event::LogsPathNotSet;
    }
    ctx.logsDir = fs::path(result);
    return Event::LogsPathSet;
}

Event handleReadPath(Context& ctx) {
    bool zip_ready = is_zip_ready(ctx.logsDir);
    if (zip_ready) {
        return Event::LogsOK;
    }
    return Event::NoReadRights;
}
