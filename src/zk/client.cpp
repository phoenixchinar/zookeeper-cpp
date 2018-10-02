#include "client.hpp"
#include "acl.hpp"
#include "connection.hpp"
#include "multi.hpp"

#include <sstream>
#include <ostream>

namespace zk {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// client                                                                                                             //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

client::client(const connection_params &params) :
    client(connection::connect(params)) {}

client::client(string_view conn_string) :
    client(connection::connect(conn_string)) {}

client::client(std::shared_ptr<connection> conn) noexcept :
    _conn(std::move(conn)) {}

future<client> client::connect(string_view conn_string) {
  return connect(connection_params::parse(conn_string));
}

future<client> client::connect(connection_params conn_params) {
  try {
    auto conn = connection::connect(conn_params);
    auto state_change_fut = conn->watch_state();
    if (conn->state() == state::connected) {
      promise<client> p;
      p.setValue(client(std::move(conn)));
      return p.getFuture();
    } else {
      // TODO: Test if future::then can be relied on and use that instead of std::async
      return std::move(state_change_fut).thenValue([conn = std::move(conn)](const state && s) mutable -> client {
        if (s == state::connected)
          return client(conn);
        else
          throw std::runtime_error(std::string("Unexpected state: ") + to_string(s));
      });
//            std::async
//                   (
//                       std::launch::async,
//                       [state_change_fut = std::move(state_change_fut), conn = std::move(conn)] () mutable -> client
//                       {
//                         state s(state_change_fut.get());
//                         if (s == state::connected)
//                            return client(conn);
//                         else
//                             throw std::runtime_error(std::string("Unexpected state: ") + to_string(s));
//                       }
//                   );
    }
  }
  catch (...) {
    promise<client> p;
    p.setException(folly::exception_wrapper::from_exception_ptr(std::current_exception()));
    return p.getFuture();
  }
}

client::~client() noexcept = default;

void client::close() {
  _conn->close();
}

future<get_result> client::get(string_view path) const {
  return _conn->get(path);
}

future<watch_result> client::watch(string_view path) const {
  return _conn->watch(path);
}

future<get_children_result> client::get_children(string_view path) const {
  return _conn->get_children(path);
}

future<watch_children_result> client::watch_children(string_view path) const {
  return _conn->watch_children(path);
}

future<exists_result> client::exists(string_view path) const {
  return _conn->exists(path);
}

future<watch_exists_result> client::watch_exists(string_view path) const {
  return _conn->watch_exists(path);
}

future<create_result> client::create(string_view path,
                                     const buffer &data,
                                     const acl &rules,
                                     create_mode mode
) {
  return _conn->create(path, data, rules, mode);
}

future<create_result> client::create(string_view path,
                                     const buffer &data,
                                     create_mode mode
) {
  return create(path, data, acls::open_unsafe(), mode);
}

future<set_result> client::set(string_view path, const buffer &data, version check) {
  return _conn->set(path, data, check);
}

future<get_acl_result> client::get_acl(string_view path) const {
  return _conn->get_acl(path);
}

future<promise_void> client::set_acl(string_view path, const acl &rules, acl_version check) {
  return _conn->set_acl(path, rules, check);
}

future<promise_void> client::erase(string_view path, version check) {
  return _conn->erase(path, check);
}

future<promise_void> client::load_fence() const {
  return _conn->load_fence();
}

future<multi_result> client::commit(multi_op txn) {
  return _conn->commit(std::move(txn));
}

}
