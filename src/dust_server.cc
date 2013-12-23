#include "dust-server/dust_server.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

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

  class_< std::vector<dust::document> >("VectorOfDocument")
    .def(vector_indexing_suite< std::vector<dust::document> >() );

  register_exception_translator<boost::system::system_error>([] (const boost::system::system_error& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
  });

}

struct PythonInitializer {
 public:
  PythonInitializer() {
    Py_Initialize();
    initDust();
  }

  ~PythonInitializer() {
     Py_Finalize();
  }
} pythonInitializer;

dust_server::dust_server(boost::asio::io_service* io_service,
                         std::shared_ptr<dust::key_value_store> store) 
    : io_service_(std::move(io_service)),
      store_(std::move(store)) {

}

std::string dust_server::apply_script(std::string script) {
  try {
    boost::python::object main_module = boost::python::import("__main__");
    boost::python::object main_namespace = main_module.attr("__dict__");

    boost::python::exec(script.c_str(), main_namespace);

    dust::document root_doc(store_, "");
    boost::python::object return_obj = main_module.attr("run")(root_doc);

    return boost::python::extract<std::string>(return_obj);
  } catch(boost::python::error_already_set const &) {
    using namespace boost::python;

    // Get traceback and error string.
    PyObject* ptype, *pvalue, *ptrace;
    PyErr_Fetch(&ptype, &pvalue, &ptrace);

    boost::python::handle<> hptype(ptype),
                            hpvalue(allow_null(pvalue)),
                            hptrace(allow_null(ptrace));

    boost::python::object traceback(import("traceback"));
    boost::python::object format_exception(traceback.attr("format_exception"));
    boost::python::object formatted = boost::python::str("")
      .join(format_exception(hptype, hpvalue, hptrace));
    std::string details = boost::python::extract<std::string>(formatted);

    std::cerr << details;

    return details;
  }

  return "undefined error";
}

}
