#include "fsm.h"

State transition(State s, Event e) {
    switch (s) {
        case State::Idle:
            if (e == Event::Start) return State::FindHome;
            break;

        case State::FindHome:
            if (e == Event::HomeSet) return State::LocateLogs;
            if (e == Event::HomeNotSet) return State::Error;
            break;

        case State::LocateLogs:
            if (e == Event::LogsPathSet) return State::ReadPath;
            if (e == Event::LogsPathNotSet) return State::Error;
            break;

        case State::ReadPath:
            if (e == Event::LogsOK) return State::ZipLogs;
            if (e == Event::NoReadRights) return State::Error;
            break;

        case State::ZipLogs:
            if (e == Event::ZipOk) return State::UploadToServer;
            if (e == Event::ZipFailed) return State::Error;
            if (e == Event::ZipCantCreate) return State::Error;
            break;

        case State::UploadToServer:
            // Preflight
            if (e == Event::Connected) return State::UploadToServer;
            if (e == Event::ErrSSLConnection) return State::Error;
            if (e == Event::ErrSSLHostnameVerif) return State::Error;
            if (e == Event::ErrSSLLoadCerts) return State::Error;
            if (e == Event::ErrSSLServerVerif) return State::Error;
            
            // Transport
            if (e == Event::ErrConnection) return State::RetryPolicy;
            if (e == Event::ErrConnectionTimeout) return State::RetryPolicy;
            if (e == Event::ErrRead) return State::RetryPolicy;
            if (e == Event::ErrWrite) return State::RetryPolicy;
            
            // ServerResponse
            if (e == Event::HTTP_OK) return State::Done;
            if (e == Event::ErrClient) return State::Error;
            if (e == Event::ErrServer) return State::RetryPolicy;
            break;
        
        case State::RetryPolicy:
            if (e == Event::Reconnect) return State::UploadToServer;
            if (e == Event::Abort) return State::Error;
            break;

        default:
            break;
        }
        
    return State::Error;
}
