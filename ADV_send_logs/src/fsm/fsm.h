#pragma once

enum class State {
    Idle,
    FindHome,
    LocateLogs,
    ReadPath,
    ZipLogs,
    Preflight,
    Transport,
    RetryPolicy,
    ServerResponse,
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
    /*--- ReadPath ---*/
    LogsFound,
    NoReadRights,
    LogsNotFound,
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
    UploadOk,
    ErrConnection,
    ErrConnectionTimeout,
    ErrRead,
    ErrWrite,
    /*--- RetryPolicy ---*/

    /*--- Connection undefined ---*/
    ErrUnknown,
    /*--- ServerResponse ---*/
    HTTP_OK,
    ErrClient,
    ErrServer
};

State transition(State current, Event event);
