#include <memory>

#include "dust/storage/key_value_store.h"

#include "dust-server/http_service.h"
#include "dust-server/base64_decode.h"

#include "boost/asio/io_service.hpp"

#include "server.hpp"
#include "request.hpp"
#include "reply.hpp"

namespace dust_server {

namespace http_server = http::server4;

http_service::http_service(boost::asio::io_service* io_service,
                           std::shared_ptr<dust::key_value_store> store,
                           const std::string& username,
                           const std::string& password)
    : io_service_(io_service),
      lua_con_(store),
      username_(std::move(username)),
      password_(std::move(password)) {
}

void http_service::start_server() {
  // Create message handler.
  using http_server::request;
  using http_server::header;
  using http_server::reply;

  typedef boost::function<void(const request&, reply&)> handler;
  handler req_handler = [this](const request& req, reply& rep) {
    // Extract headers.
    bool urlencoded = false;
    std::string auth = "";

    for (const auto& h : req.headers) {
      if (h.name == "Content-Type"
          && h.value.find("urlencoded") != std::string::npos) {
        urlencoded = true;
      }

      if (h.name == "Authorization") {
        auth = h.value;
      }
    }

    // No authorization sent. -> STOP
    std::string debug =  auth.empty() ? auth : auth.substr(0, 5);
    std::cout << "auth: '" << debug << "'\n";
    if (auth.empty() || auth.substr(0, 5) != "Basic") {
      rep.content = "authorization required";
      rep.status = reply::unauthorized;
      rep.headers.resize(3);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = std::to_string(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = "text/plain";
      rep.headers[2].name = "WWW-Authenticate";
      rep.headers[2].value = "Basic realm=\"dustDB\"";

      return;
    }

    // Check authorization.
    std::string credentials = decode_base64(auth.substr(6));
    std::cout << "cedentials: " << credentials << "\n";
    size_t split = credentials.find_first_of(":");

    // Bad credentials format. -> STOP
    if (split == std::string::npos) {
      rep.content = "invalid authorization request";
      rep.status = reply::bad_request;
      rep.headers.resize(2);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = std::to_string(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = "text/plain";

      return;
    }

    std::string username = credentials.substr(0, split);
    std::string password = credentials.substr(split + 1, std::string::npos);

    std::cout << "username: " << username << "\n";
    std::cout << "password: " << password << "\n";

    // Invalid username/password. -> STOP
    if (username != username_ || password != password_) {
      rep.content = "access denied";
      rep.status = reply::unauthorized;
      rep.headers.resize(2);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = std::to_string(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = "text/plain";

      return;
    }

    // Decode content if required.
    std::string script;
    if (urlencoded) {
      std::cout << "script is url-encoded\n";
      url_decode(req.content, script);
    } else {
      script = req.content;
    }

    // Execute script.
    std::string result = lua_con_.apply_script(script);
    std::cout << "script: '" << script << "'\n";
    std::cout << "result: '" << result << "'\n";

    // Send result.
    rep.content = result;
    rep.status = reply::ok;
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = std::to_string(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "text/plain";
  };

  // Instantiate and run server.
  http_server::server(*io_service_, "0.0.0.0", "9091", req_handler)();
  io_service_->run();
}

bool http_service::url_decode(const std::string& in, std::string& out) {
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          out += static_cast<char>(value);
          i += 2;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else if (in[i] == '+') {
      out += ' ';
    } else {
      out += in[i];
    }
  }
  return true;
}

}
