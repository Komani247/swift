#include "pool.h"

VALUE cSwiftPool;

static void pool_free(dbi::ConnectionPool *self) {
  if (self) delete self;
}

VALUE pool_alloc(VALUE klass) {
  dbi::ConnectionPool *pool = 0;
  return Data_Wrap_Struct(klass, 0, pool_free, pool);
}

static dbi::ConnectionPool* pool_handle(VALUE self) {
  dbi::ConnectionPool *pool;
  Data_Get_Struct(self, dbi::ConnectionPool, pool);
  if (!pool) rb_raise(eSwiftRuntimeError, "Invalid object, did you forget to call #super ?");
  return pool;
}

// TODO: Remove unnecessary assignments. See Adapter.
VALUE pool_init(VALUE self, VALUE n, VALUE options) {
  VALUE db       = rb_hash_aref(options, ID2SYM(rb_intern("db")));
  VALUE host     = rb_hash_aref(options, ID2SYM(rb_intern("host")));
  VALUE port     = rb_hash_aref(options, ID2SYM(rb_intern("port")));
  VALUE user     = rb_hash_aref(options, ID2SYM(rb_intern("user")));
  VALUE driver   = rb_hash_aref(options, ID2SYM(rb_intern("driver")));
  VALUE password = rb_hash_aref(options, ID2SYM(rb_intern("password")));
  VALUE zone     = rb_hash_aref(options, ID2SYM(rb_intern("timezone")));

  if (NIL_P(db))     rb_raise(eSwiftArgumentError, "Pool#new called without :db");
  if (NIL_P(driver)) rb_raise(eSwiftArgumentError, "#new called without :driver");

  user = NIL_P(user) ? rb_str_new2(getlogin()) : user;
  if (NUM2INT(n) < 1) rb_raise(eSwiftArgumentError, "Pool#new called with invalid pool size.");

  try {
    DATA_PTR(self) = new dbi::ConnectionPool(
      NUM2INT(n),
      CSTRING(driver),
      CSTRING(user),
      CSTRING(password),
      CSTRING(db),
      CSTRING(host),
      CSTRING(port)
    );
  }
  CATCH_DBI_EXCEPTIONS();
  return Qnil;
}

void pool_callback(dbi::AbstractResult *result) {
  VALUE callback = (VALUE)result->context;
  // NOTE ResultSet will be free'd by the underlying connection pool dispatcher ib dbic++
  ResultWrapper *handle = new ResultWrapper;
  handle->result  = result;
  handle->adapter = 0;
  handle->free    = false;
  if (!NIL_P(callback)) rb_proc_call(callback, rb_ary_new3(1, Data_Wrap_Struct(cSwiftResult, 0, result_free, handle)));
}

VALUE pool_execute(int argc, VALUE *argv, VALUE self) {
  int n;
  VALUE sql;
  VALUE bind_values;
  VALUE callback;
  VALUE request = Qnil;

  dbi::ConnectionPool *pool = pool_handle(self);
  rb_scan_args(argc, argv, "1*&", &sql, &bind_values, &callback);

  if (NIL_P(callback)) rb_raise(eSwiftArgumentError, "No block given in Pool#execute");

  try {
    Query query;
    query_bind_values(&query, bind_values);
    request = request_alloc(cSwiftRequest);
    DATA_PTR(request) = pool->execute(CSTRING(sql), query.bind, pool_callback, (void*)callback);
    return request;
  }
  CATCH_DBI_EXCEPTIONS();
}

void init_swift_pool() {
  VALUE mSwift = rb_define_module("Swift");
  VALUE mDB    = rb_define_module_under(mSwift, "DB");
  cSwiftPool   = rb_define_class_under(mDB, "Pool", rb_cObject);

  rb_define_alloc_func(cSwiftPool, pool_alloc);

  rb_define_method(cSwiftPool, "initialize",  RUBY_METHOD_FUNC(pool_init),     2);
  rb_define_method(cSwiftPool, "execute",     RUBY_METHOD_FUNC(pool_execute), -1);
}
