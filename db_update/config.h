#pragma once

#include "Current/current.h"

struct InvalidConfigException : current::Exception {
    using current::Exception::Exception;
};

class Config {
 public:
    Config(std::string user) : user_(user) {};
    
    void BuildConfig() {
        // read file
        std::ifstream fs;
        fs.open(user_);
        if (!fs.good()) {
            CURRENT_THROW(InvalidConfigException(Printf("Build Config failed: Missing config for user '%s'",user_.c_str()))); //
            
        }
        std::string line;
        int i=0;
        while (fs >> line) {
            config_[line] = i;
            i++;
        }
        fs.close();
    }
    int size() {
        return config_.size();
    }
    int GetCSVPlaceForColumn(std::string column_name) {
        if (config_.find(column_name) == config_.end()) {
            return -1;
        }else{
            return config_[column_name];
        }
    }

 private:
    std::unordered_map<std::string,int> config_;
    std::string user_;

};

