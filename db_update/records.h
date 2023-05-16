#pragma once

#include "Current/current.h"
#include "config.h"

struct InvalidInputException : current::Exception {
    using current::Exception::Exception;
};
struct DBFailureException : current::Exception {
    using current::Exception::Exception;
};

CURRENT_STRUCT(User) {
    CURRENT_FIELD(FirstName, std::string);
    CURRENT_FIELD(LastName, std::string);
    CURRENT_FIELD(MiddleInitial, std::string);
    CURRENT_FIELD(Email, std::string);
    CURRENT_FIELD(Address, std::string);
    CURRENT_FIELD(Zip, std::string);
    CURRENT_FIELD(DateJoined, std::string);
    CURRENT_FIELD(UUID, std::string);
    CURRENT_DEFAULT_CONSTRUCTOR(User) {}
    CURRENT_CONSTRUCTOR(User)(std::string fname,
                              std::string lname,
                              std::string middle_initial,
                              std::string email,
                              std::string address,
                              std::string zip,
                              std::string date,
                              std::string UUID)
    : FirstName(fname), LastName(lname),MiddleInitial(middle_initial), Email(email),Address(address),
    Zip(zip), DateJoined(date), UUID(UUID) {
    }
    
private:
    
};


class Records {
public:
    using records_t = std::vector<User>;
    
    void AddRecord(User user) {
        records_.push_back(std::move(user));
    }
    void BuildUserDataFromConfig(Config& config,const std::vector<std::string>& values) {
        int index = config.GetCSVPlaceForColumn("FirstName");
        std::string first = (index == -1 ? "" : values[index]);
        index = config.GetCSVPlaceForColumn("LastName");
        std::string last = (index == -1 ? "" : values[index]);
        index = config.GetCSVPlaceForColumn("MiddleInitial");
        std::string middle = (index == -1 ? "" : values[index]);
        index = config.GetCSVPlaceForColumn("Email");
        std::string email = (index == -1 ? "" : values[index]);
        index = config.GetCSVPlaceForColumn("Address");
        std::string address = (index == -1 ? "" : values[index]);
        index = config.GetCSVPlaceForColumn("Zip");
        std::string zip = (index == -1 ? "" : values[index]);
        index = config.GetCSVPlaceForColumn("Date");
        std::string date = (index == -1 ? "" : values[index]);
        index = config.GetCSVPlaceForColumn("UUID");
        std::string uuid = (index == -1 ? "" : values[index]);
        AddRecord(User(first,last,middle,email,address,zip,date,uuid));

    }
    void ParseCsv(std::string file,std::string user) {
        Config config(user);
        config.BuildConfig();
        int config_size = config.size();
        std::ifstream fs;
        fs.open(file);
        if (!fs.good()) {
            CURRENT_THROW(InvalidInputException("Not existing user configurations"));
        }
        std::string line;
        while (fs >> line) {
            std::vector<std::string> val = ParseLine(line);
            if (val.size() != config_size) {
                CURRENT_THROW(InvalidInputException("CSV columns don't match config size"));
            }

            BuildUserDataFromConfig(config,val);
        }
        fs.close();
    }
    void ParseCsvObject(std::string csv,std::string user) {
        Config config(user);
        config.BuildConfig();
        int config_size = config.size();
        std::istringstream iss(csv);
        std::vector<std::string> res;
        std::string token;
        while ( getline(iss, token) )
        {
            res.push_back(token);
        }
        for (std::string line : res){
            std::vector<std::string> val = ParseLine(line);
            if (val.size() != config_size) {
                CURRENT_THROW(InvalidInputException("CSV columns don't match config size"));
            }
            
            BuildUserDataFromConfig(config,val);
        }
     }
    void PopulateDB() {
        //prepare insert statement
        sqlite3 *db;
        int rc;
        
        rc = sqlite3_open("test.db", &db);
        
        if(rc) {
            CURRENT_THROW(DBFailureException(Printf("Open Connection failed: '%s'",sqlite3_errmsg(db)))); //
        }
        for (auto record : records_) {
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, "insert into users (first_name,last_name,middle_initial,\
                               email,address,zip,date_joined,uuid) \
                               values (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);", -1, &stmt, NULL);
            sqlite3_bind_text(stmt, 1, record.FirstName.c_str(),-1,SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, record.LastName.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, record.MiddleInitial.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, record.Email.c_str(),-1,SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, record.Address.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 6, record.Zip.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 7, record.DateJoined.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 8, record.UUID.c_str(), -1, SQLITE_STATIC);
            
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                CURRENT_THROW(DBFailureException(Printf("InsertFailed: '%s'",sqlite3_errmsg(db)))); //
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);  }
    
private:
    records_t records_;
    std::vector<std::string> ParseLine(std::string line) {
        std::istringstream iss(line);
        std::vector<std::string> res;
        std::string token;
        while ( getline(iss, token, ',') )
        {
            res.push_back(token);
        }
        return res;
    }
};
