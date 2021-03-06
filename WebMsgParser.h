#pragma once

#include <map>
#include <string>

#include "Utils.h"
#include "Buffer.h"

class WebMsgParser {
private:

    std::map<std::string, std::vector<std::string>> m_lists;
    std::map<std::string, std::string>              m_scalers;

    std::string msg;
public:

    WebMsgParser() {

        msg = "";
    }

     void parse(Buffer& msg) {   

        std::string incomingMsg = std::string(msg.getBase(), msg.getSize());

        std::cout << "WebMsgParser: message: " << incomingMsg << std::endl;

        std::vector<std::string> tokens = split(incomingMsg, ';');
        
        for(int i = 0; i < tokens.size(); i++) {

            std::vector<std::string> keyVals = split(tokens[i], '=');
            std::vector<std::string> listVals = split(keyVals[1], '-');
            
            if(listVals.size() > 1) {

                std::vector<std::string> list;

                for(int j = 0; j < listVals.size(); j++) {

                    std::cout << "WebMsgParser: list val: " << listVals[j] << std::endl;

                    list.push_back(listVals[j]);
                }

                m_lists.insert({keyVals[0], list});

            } else {

                std::cout << "WebMsgParser: Scaler (key, val): " << keyVals[0] << ", " << keyVals[1] << std::endl;

                m_scalers.insert({keyVals[0], keyVals[1]});

            }

        }

    }

    void encode(std::string name, std::string val) {

        msg += name + "=" + val + ";";
    }

    void encode(std::string name, std::vector<std::string> values) {

        msg += name + "=";

        for(int i = 0; i < values.size(); i++) {

            msg += values[i];

            if(i != values.size() - 1) {

                 msg += "-";
            }
        }

        msg += ";";
    }

    void reset() {
        
        msg = "";

        m_lists.clear();
        m_scalers.clear();
    }

    std::string getMessage() {

        std::string temp = msg;

        if(temp.size() != 0 && temp[temp.size() - 1] == ';')
            temp.pop_back();

        return temp;
    }

    std::vector<std::string> get(std::string name) {

        std::vector<std::string> vals;

        auto itr = m_scalers.find(name);

        if(itr != m_scalers.end()) {

            vals.push_back(m_scalers.at(name));

            return vals;

        } else {

            std::cout << "WebMsgParser: Unknown key: " << name << std::endl;
        }

        auto itr_scaler = m_lists.find(name);

        if(itr_scaler != m_lists.end()) {

            vals = m_lists.at(name);
        } else {

            std::cout << "WebMsgParser: Unknown key: " << name << std::endl;
        }

        return vals;
    }

};

std::string getScaler(std::vector<std::string> list, std::string def) {

    for(int i = 0; i < list.size(); i++) {

        return list[i];
    }

    return def;
}

