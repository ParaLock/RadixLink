#pragma once

#include <map>
#include <functional>

#include "Buffer.h"
#include "Encoder.h"

class DataSegmenter {
private:
    std::map<std::string, std::function<void(Buffer&, std::vector<Buffer>&)>> m_handlers;
public:

    void registerHandler(std::string dataType, std::function<void(Buffer& input, std::vector<Buffer>&)> callback) {

        m_handlers.insert({dataType, callback});
    }

    bool run(std::string fn, std::string dataType, std::vector<Buffer>& segments) {

        auto callback = m_handlers.at(dataType);
        Buffer data;

        if(Encoder::run(fn, data)) {

            callback(data, segments);


            return true;

        } else {

            return false;
        }
    }
};