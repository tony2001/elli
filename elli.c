/* elli extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"
#include "php_elli.h"

#include <elli.h>

static zend_class_entry *elli_ce;
static zend_object_handlers elli_handlers;

typedef struct _elli_obj {
	elli_ctx_t *ctx;
	char *curve;
	zend_object zo;
} elli_obj_t;

static inline elli_obj_t *php_elli_fetch_object(zend_object *obj) {
	return (elli_obj_t *)((char*)(obj) - XtOffsetOf(elli_obj_t, zo));
}

#define Z_ELLI_OBJ_P(zv) php_elli_fetch_object(Z_OBJ_P((zv)))

static zend_object *elli_create_object(zend_class_entry *class_type) /* {{{ */
{
	elli_obj_t *intern = zend_object_alloc(sizeof(elli_obj_t), class_type);

	zend_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);
	intern->zo.handlers = &elli_handlers;

	return &intern->zo;
}
/* }}} */

static void elli_free_object(zend_object *object) /* {{{ */
{
	elli_obj_t *intern = php_elli_fetch_object(object);

	if (intern->ctx) {
		elli_ctx_free(intern->ctx);
	}

	if (intern->curve) {
		efree(intern->curve);
	}

	zend_object_std_dtor(&intern->zo);
}
/* }}} */

/* {{{ string elli_encrypt(string curve, string public_key, string data)
 */
PHP_FUNCTION(elli_encrypt)
{
	char *curve, *key, *data;
	size_t curve_len, key_len, data_len;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_STRING(curve, curve_len)
		Z_PARAM_STRING(key, key_len)
		Z_PARAM_STRING(data, data_len)
	ZEND_PARSE_PARAMETERS_END();

	elli_ctx_t *ctx = elli_ctx_create(curve, NULL);
	if (!ctx) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to initialize Elli context");
		RETURN_FALSE;
	}

	char *encrypted = elli_encrypt(ctx, key, data, &data_len);
	if (!encrypted) {
		elli_ctx_free(ctx);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to encrypt data: %s", elli_ctx_last_error(ctx));
		RETURN_FALSE;
	}
	RETVAL_STRINGL(encrypted, data_len);
	elli_ctx_free(ctx);
	free(encrypted);
}
/* }}} */

/* {{{ string elli_decrypt(string curve, string private_key, string data)
 */
PHP_FUNCTION(elli_decrypt)
{
	char *curve, *key, *data;
	size_t curve_len, key_len, data_len;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_STRING(curve, curve_len)
		Z_PARAM_STRING(key, key_len)
		Z_PARAM_STRING(data, data_len)
	ZEND_PARSE_PARAMETERS_END();

	elli_ctx_t *ctx = elli_ctx_create(curve, NULL);
	if (!ctx) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to initialize Elli context");
		RETURN_FALSE;
	}

	char *decrypted = elli_decrypt(ctx, key, data, &data_len);
	if (!decrypted) {
		elli_ctx_free(ctx);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to decrypt data: %s", elli_ctx_last_error(ctx));
		RETURN_FALSE;
	}
	RETVAL_STRINGL(decrypted, data_len);
	elli_ctx_free(ctx);
	free(decrypted);
}
/* }}} */

/* Elli::__construct(string curve) {{{ */
PHP_METHOD(Elli, __construct)
{
	char *curve, *err_str;
	size_t curve_len;
	elli_obj_t *obj = Z_ELLI_OBJ_P(getThis());

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STRING(curve, curve_len)
	ZEND_PARSE_PARAMETERS_END();

	obj->ctx = elli_ctx_create(curve, &err_str);
	if (!obj->ctx) {
		zend_throw_exception_ex(zend_ce_exception, 0, "Failed to initialize Elli object: %s", err_str);
		free(err_str);
		return;
	}

	/* store the name of the curve for demo purposes */
	obj->curve = estrdup(curve);
}
/* }}} */

/* Elli::getCurve() {{{ */
PHP_METHOD(Elli, getCurve)
{
	elli_obj_t *obj = Z_ELLI_OBJ_P(getThis());

	ZEND_PARSE_PARAMETERS_NONE();

	if (!obj->curve) {
		RETURN_FALSE;
	}
	RETURN_STRING(obj->curve);
}
/* }}} */

/* Elli::encrypt() {{{ */
PHP_METHOD(Elli, encrypt)
{
	char *public_key, *data;
	size_t public_key_len, data_len;
	elli_obj_t *obj = Z_ELLI_OBJ_P(getThis());

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STRING(public_key, public_key_len)
		Z_PARAM_STRING(data, data_len)
	ZEND_PARSE_PARAMETERS_END();

	char *encrypted = elli_encrypt(obj->ctx, public_key, data, &data_len);
	if (!encrypted) {
		zend_throw_exception_ex(zend_ce_exception, 0, "Failed to encrypt data: %s", elli_ctx_last_error(obj->ctx));
		return;
	}
	RETVAL_STRINGL(encrypted, data_len);
	free(encrypted);
}
/* }}} */

static zend_function_entry elli_methods[] = {
	PHP_ME(Elli, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Elli, getCurve, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Elli, encrypt, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(elli)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Elli", elli_methods);

	//set custom obj create handler
	ce.create_object = elli_create_object;
	//initialize other handlers
	memcpy(&elli_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	//offset has to be set since we use a custom object struct
	elli_handlers.offset = XtOffsetOf(elli_obj_t, zo);
	//set custom obj dtor handler
	elli_handlers.free_obj = elli_free_object;

	//create class entry
	elli_ce = zend_register_internal_class(&ce);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(elli)
{
#if defined(ZTS) && defined(COMPILE_DL_ELLI)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(elli)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "elli support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ elli_functions[]
 */
static const zend_function_entry elli_functions[] = {
	PHP_FE(elli_encrypt, NULL)
	PHP_FE(elli_decrypt, NULL)
	PHP_FE_END
};
/* }}} */

/* {{{ elli_module_entry
 */
zend_module_entry elli_module_entry = {
	STANDARD_MODULE_HEADER,
	"elli",					/* Extension name */
	elli_functions,			/* zend_function_entry */
	PHP_MINIT(elli),		/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(elli),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(elli),			/* PHP_MINFO - Module info */
	PHP_ELLI_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ELLI
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(elli)
#endif
