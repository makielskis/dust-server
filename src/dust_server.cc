#include "dust-server/dust_server.h"

#include <boost/python.hpp>

#include "dust/document.h"

namespace dust_server {

BOOST_PYTHON_MODULE(Dust)
{
  using namespace boost::python;

  class_<dust::document>("Document", init<std::weak_ptr<dust::key_value_store>, std::string>())
    .def("__getitem__", &dust::document::operator[])
    .def("assign", &dust::document::assign)
    .def("index", &dust::document::index)
    .def("val", &dust::document::val)
    .def("exists", &dust::document::exists)
    .def("remove", &dust::document::remove)
    .def("is_composite", &dust::document::is_composite)
    .def("children", &dust::document::children);
}

dust_server::dust_server(boost::asio::io_service* io_service,
                         std::shared_ptr<dust::key_value_store> store) 
    : io_service_(std::move(io_service)),
      store_(std::move(store)) {

}

std::string dust_server::apply_script(std::string script) {
  try {
    Py_Initialize();
    initDust();

    boost::python::object main_module = boost::python::import("__main__");
    boost::python::object main_namespace = main_module.attr("__dict__");

    boost::python::exec(script.c_str(), main_namespace);

    dust::document root_doc(store_, "");
    boost::python::object return_obj = main_module.attr("run")(root_doc);

    return boost::python::extract<std::string>(return_obj);
  } catch(boost::python::error_already_set const &) {
    PyErr_Print();
  }

  return "undefined error";
}

}
