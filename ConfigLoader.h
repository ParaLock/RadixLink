#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include "Utils.h"

class ConfigLoader {
private:
    std::map<std::string, std::vector<std::string>> m_lists;
    std::map<std::string, std::string>              m_scalers;
public:

    ConfigLoader(std::string fn) {

        std::string line;
        std::ifstream myfile (fn);

        std::vector<std::string> listValues;
        std::string              listKey;

        if (myfile.is_open()) {
            
            bool isList = false;

            while (getline (myfile,line))
            {
                if(line.size() > 0) {
                    
                    if(line[0] == '@') {

                        if(isList) {

                            m_lists.insert({listKey, listValues});
                            isList = false;
                            listKey    = "";
                            listValues.clear();
                        }

                        std::string              key    = line.substr(1, std::string::npos);
                        std::vector<std::string> keyVal = split(key, ' ');

                        if(keyVal.size() == 2) {
                            
                            m_scalers.insert({keyVal[0], keyVal[1]});

                            continue;

                        } else if(keyVal.size() == 1){

                            isList = true;
                            listKey    = keyVal[0];

                            continue;
                        }
                    }

                    listValues.push_back(line); 
                }
            }

            m_lists.insert({listKey, listValues});

            myfile.close();

        } else {
            std::cout << "Unable to open file";
        }
    }

    void getList(std::string name, std::vector<std::string>& out) {

        out = m_lists.at(name);
    }

    void getScaler(std::string name, std::string& val) {

        val = m_scalers.at(name);
    }

};