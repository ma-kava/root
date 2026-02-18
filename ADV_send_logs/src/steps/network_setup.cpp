#include "steps/network_setup.h"

#include "fsm/fsm.h"
#include "fsm/context.h"

#include <httplib.h>
#include <fstream>
#include <vector>

Event handleUploadToServer(Context& ctx) {
    auto zipPath = ctx.zipPath.string();
    
    // load the zip file to buffer
    std::cout << "Loading the zip to buffer" << std::endl;
    std::ifstream file(zipPath, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading.";
        // TODO: add Event signaling this error
    }
    // read file size
    file.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    // load the zip into memory
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    httplib::Result res = ctx.uploader.sendBinary(buffer);
    Event ev;

    if (!res) {
        const auto err = res.error();
        ev = map_httplib_error(err);
    }

    if (res) { // server response
        ev = map_httplib_http_response(res->status);
    } 
    return ev;
}

Event map_httplib_error(const httplib::Error err) {
    switch (err) {
        case httplib::Error::SSLConnection:
            return Event::ErrSSLConnection;
        case httplib::Error::SSLServerHostnameVerification:
            return Event::ErrSSLHostnameVerif;
        case httplib::Error::SSLLoadingCerts:
            return Event::ErrSSLLoadCerts;
        case httplib::Error::SSLServerVerification:
            return Event::ErrSSLServerVerif;

        case httplib::Error::Connection:
            return Event::ErrConnection;
        case httplib::Error::ConnectionTimeout:
            return Event::ErrConnectionTimeout;
        case httplib::Error::Read:
            return Event::ErrRead;
        case httplib::Error::Write:
            return Event::ErrWrite;
    }

    return Event::ErrSSLConnection; // leads the FSM to the State::Error
}

Event map_httplib_http_response(int status) {
    if (status >= 200 && status < 300) {
        return Event::HTTP_OK;
    } else if (status >= 400 && status < 500) { // client error (no retry)
        return Event::ErrClient;
    } else if (status >= 500) {                 // server error (retry)
        return Event::ErrServer;
    }

    return Event::ErrClient; // leads the FSM to the State::Error
}
