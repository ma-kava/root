#pragma once

#include <string>

enum class State {
    Idle,
    FindHome,
    LocateLogs,
    ReadPath,
    ZipLogs,
    UploadToServer, // {Preflight, Transport, ServerResponse}
    RetryPolicy,
    // Cleanup,
    Done,
    Error
};

enum class Event {
    /*--- Idle ---*/
    Start,
    /*--- FindHome ---*/
    HomeSet,
    HomeNotSet,
    /*--- LocateLogs ---*/
    LogsPathSet,
    LogsPathNotSet,
    /*--- ReadPath ---*/
    LogsOK,
    NoReadRights,
    /*--- ZipLogs ---*/
    ZipOk,
    ZipFailed,
    ZipCantCreate,
    /*--- Preflight ---*/
    Connected,
    ErrSSLConnection,
    ErrSSLHostnameVerif,
    ErrSSLLoadCerts,
    ErrSSLServerVerif,
    /*--- Transport ---*/
    // UploadOk,
    ErrConnection,
    ErrConnectionTimeout,
    ErrRead,
    ErrWrite,
    /*--- RetryPolicy ---*/
    Reconnect,
    Abort,
    /*--- Connection undefined ---*/
    ErrUnknown,
    /*--- ServerResponse ---*/
    HTTP_OK,
    ErrClient,
    ErrServer
};

State transition(State current, Event event);

inline const char* to_string(State state)
{
    switch (state)
    {
        case State::Idle:           return "Idle";
        case State::FindHome:       return "FindHome";
        case State::LocateLogs:     return "LocateLogs";
        case State::ReadPath:       return "ReadPath";
        case State::ZipLogs:        return "ZipLogs";
        case State::UploadToServer: return "UploadToServer";
        // case State::Preflight:      return "Preflight";
        // case State::Transport:      return "Transport";
        case State::RetryPolicy:    return "RetryPolicy";
        // case State::ServerResponse: return "ServerResponse";
        case State::Done:           return "Done";
        case State::Error:          return "Error";
    }
    return "State::<unknown>";
}

inline const char* to_string(Event event)
{
    switch (event)
    {
        case Event::Start:                return "Start";

        case Event::HomeSet:              return "HomeSet";
        case Event::HomeNotSet:           return "HomeNotSet";

        case Event::LogsPathSet:          return "LogsPathSet";
        case Event::LogsPathNotSet:       return "LogsPathNotSet";

        case Event::LogsOK:               return "LogsOK";
        case Event::NoReadRights:         return "NoReadRights";

        case Event::ZipOk:                return "ZipOk";
        case Event::ZipFailed:            return "ZipFailed";
        case Event::ZipCantCreate:        return "ZipCantCreate";

        case Event::Connected:            return "Connected";
        case Event::ErrSSLConnection:     return "ErrSSLConnection";
        case Event::ErrSSLHostnameVerif:  return "ErrSSLHostnameVerif";
        case Event::ErrSSLLoadCerts:      return "ErrSSLLoadCerts";
        case Event::ErrSSLServerVerif:    return "ErrSSLServerVerif";

        // case Event::UploadOk:             return "UploadOk";
        case Event::ErrConnection:        return "ErrConnection";
        case Event::ErrConnectionTimeout: return "ErrConnectionTimeout";
        case Event::ErrRead:              return "ErrRead";
        case Event::ErrWrite:             return "ErrWrite";

        case Event::Reconnect:            return "Reconnect";
        case Event::Abort:                return "Abort";

        case Event::HTTP_OK:              return "HTTP_OK";
        case Event::ErrClient:            return "ErrClient";
        case Event::ErrServer:            return "ErrServer";
    }
    return "Event::<unknown>";
}
