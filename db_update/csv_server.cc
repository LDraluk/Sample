#include "Current/current.h"

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
      std::string const tmp_file_name = current::FileSystem::GenTmpFileName();
      auto const file_remover = current::FileSystem::ScopedRmFile(tmp_file_name, true);
      current::FileSystem::WriteStringToFile(csv_object.csv, tmp_file_name.c_str());
      // Do something here with the file. For example, print the output file size.
      std::cout << current::FileSystem::GetFileSize(tmp_file_name) << std::endl;
      // ...
      r("OK\n");
    } catch(TypeSystemParseJSONException const& e) {
      r(Printf("JSON parse error: %s\n", e.what()), HTTPResponseCode.BadRequest);
    } catch(current::FileException const& e) {
      r(Printf("FileSystem error: %s\n", e.what()), HTTPResponseCode.InternalServerError);
    } catch(current::Exception const& e) {
      r(Printf("Unexpected exception: %s\n", e.what()), HTTPResponseCode.InternalServerError);
    }
  }

 private:
  // Scoped HTTP route handlers. Deregisters handler on destruction.
  const HTTPRoutesScope http_scope;
};

int main() {
  int const port = 12345;
  CSVServer csv_server(port);
  std::cout << "CSV server started. Try POSTing to http://localhost:" << port << "/post_csv" << std::endl;
  HTTP(port).Join();  // Wait indefinitely.
  return 0;
}
