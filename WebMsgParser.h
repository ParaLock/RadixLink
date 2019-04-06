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

        std::string incomingMsg = std::string(msg.getBase());

        std::cout << "WebMsgParser: message: " << incomingMsg << std::endl;

        std::vector<std::string> tokens = split(incomingMsg, ';');
        
        for(int i = 0; i < tokens.size(); i++) {

            std::vector<std::string> keyVals = split(tokens[i], '=');
            std::vector<std::string> listVals = split(keyVals[1], '-');
            
            if(listVals.size() > 1) {

                std::vector<std::string> list;

                for(int j = 0; j < listVals.size(); j++) {

                    list.push_back(listVals[i]);
                }

                m_lists.insert({keyVals[0], list});

            } else {

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

            // if(i != values.size() - 1) {

            //     msg += "-";
            // }
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
        temp.pop_back();

        return temp;
    }

    std::string getScaler(std::string name) {

        return m_scalers.at(name);
    } 

    std::vector<std::string> getList(std::string name) {

        return m_lists.at(name);
    }

};