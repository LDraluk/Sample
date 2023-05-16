//
//  main.cpp
//  SmithRX
//
//  Created by Leah Draluk on 12/12/18.
//  Copyright © 2018 Leah Draluk. All rights reserved.
//

#include <iostream>
#include <sqlite3.h>
#include "Current/current.h"
#include "records.h"
#include "config.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for(i = 0; i<argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}
void prepare_table() {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    
    rc = sqlite3_open("test.db", &db);
    
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }
    const char* sql = "CREATE TABLE USERS("  \
    "FIRST_NAME     VARCHAR(50) NOT NULL," \
    "LAST_NAME      VARCHAR(50) NOT NULL," \
    "MIDDLE_INITIAL CHAR(1)," \
    "EMAIL TEXT PRIMARY_KEY ," \
    "ADDRESS        VARCHAR(50)," \
    "ZIP            CHAR(5),"\
    "DATE_JOINED    VARCHAR(50),"\
    "UUID           TEXT NOT NULL);";
    
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Table created successfully\n");
    }
    sqlite3_close(db);

}

CURRENT_STRUCT(CSVObject) {
    CURRENT_FIELD(author, std::string);
    CURRENT_FIELD(csv, std::string);
};

class CSVServer {
public:
    CSVServer(uint16_t port)
    : http_scope(HTTP(port)  // Use HTTP server at port `port`.
    .Register("/post_csv", // Register the endpoint "/post_csv".
              URLPathArgs::CountMask::None,  // No additional URL path arguments.
              [this](Request r) { ServePostCSV(std::move(r)); })) {}
private:
    void ServePostCSV(Request r) {
        
        if (r.method != "POST") {
            r(current::net::DefaultMethodNotAllowedMessage(),
              HTTPResponseCode.MethodNotAllowed,
              current::net::http::Headers(),
              current::net::constants::kDefaultHTMLContentType);
            return;
        }
        try {
            auto const csv_object = ParseJSON<CSVObject>(r.body);
            Records record;
            record.ParseCsvObject(csv_object.csv,csv_object.author);
            record.PopulateDB();
        } catch(TypeSystemParseJSONException const& e) {
            r(Printf("JSON parse error: %s\n", e.what()), HTTPResponseCode.BadRequest);
        } catch(current::FileException const& e) {
            r(Printf("FileSystem error: %s\n", e.what()), HTTPResponseCode.InternalServerError);
        } catch(InvalidInputException const& e) {
            r(Printf("Invalid Input error: %s\n", e.what()), HTTPResponseCode.BadRequest);
        } catch(InvalidConfigException const& e) {
            r(Printf("Invalid Config error: %s\n", e.what()), HTTPResponseCode.BadRequest);
        } catch(DBFailureException const& e) {
            r(Printf("DB operation failure: %s\n", e.what()), HTTPResponseCode.BadRequest);
        } catch(current::Exception const& e) {
            r(Printf("Unexpected exception: %s\n", e.what()), HTTPResponseCode.InternalServerError);
        }
    }
    
private:
    // Scoped HTTP route handlers. Deregisters handler on destruction.
    const HTTPRoutesScope http_scope;
};
int main(int argc, const char * argv[]) {
    prepare_table();
    int const port = 12345;
    CSVServer csv_server(port);
    std::cout << "CSV server started. Try POSTing to http://localhost:" << port << "/post_csv" << std::endl;
    HTTP(port).Join();  // Wait indefinitely.
    return 0;
}
