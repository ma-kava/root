#include <filesystem>
#include <iostream>
// #include <sys/stat.h> // for file size

#include "fsm/fsm.h"
#include "fsm/context.h"

#include "steps/environment.h"
#include "steps/network_setup.h"
#include "steps/packaging.h"
#include "policy/retry_policy.h"

#define DEBUG

#ifdef DEBUG
#include "debug/log.hpp"
#endif

namespace fs = std::filesystem;


int main() {
    fs::path outputZip = "logs_dir.zip";
    Context ctx = Context("127.0.0.1", 5000, "/log"/* init LogUploader */, outputZip, 3, 3);
    State state = State::Idle;
    Event ev = Event::Start;

    while (state != State::Done && state != State::Error) {
        State next = transition(state, ev);
        #ifdef DEBUG
        log_fsm(state, ev, next);
        #endif
        state = next;

        switch (state)
        {
        case State::FindHome:
            ev = handleFindHome(ctx);
            break;

        case State::LocateLogs:
            ev = handleLocateLogs(ctx);
            break;

        case State::ReadPath:
            ev = handleReadPath(ctx);
            break;

        case State::ZipLogs:
            ev = handleZipLogs(ctx);
            break;

        case State::UploadToServer:
            ev = handleUploadToServer(ctx);
            break;

        case State::RetryPolicy:
            ev = handleRetryPolicy(ctx);
            break;

        default:
            std::cout << "main: DEFAULT\n";
            break;
        }
    }

    return 0;
}
