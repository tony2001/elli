/* elli extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_elli.h"

#include <elli.h>

static int le_elli;

typedef struct _php_elli_ctx {
	elli_ctx_t *ctx;
} php_elli_ctx_t;

/* {{{ resource elli_ctx_create(string curve))
 */
PHP_FUNCTION(elli_ctx_create)
{
	char *curve, *err_str;
	size_t curve_len;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STRING(curve, curve_len)
	ZEND_PARSE_PARAMETERS_END();

	elli_ctx_t *ctx = elli_ctx_create(curve, &err_str);
	if (!ctx) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to initialize Elli context: %s", err_str);
		free(err_str);
		RETURN_FALSE;
	}

	php_elli_ctx_t *php_ctx = emalloc(sizeof(php_elli_ctx_t));
	php_ctx->ctx = ctx;

	RETURN_RES(zend_register_resource(php_ctx, le_elli));
}
/* }}} */

/* {{{ string elli_encrypt(resource ctx, string public_key, string data)
 */
PHP_FUNCTION(elli_encrypt)
{
	zval *ctx;
	char *key, *data;
	size_t key_len, data_len;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_RESOURCE(ctx)
		Z_PARAM_STRING(key, key_len)
		Z_PARAM_STRING(data, data_len)
	ZEND_PARSE_PARAMETERS_END();

	php_elli_ctx_t *php_ctx = (php_elli_ctx_t *)zend_fetch_resource(Z_RES_P(ctx), "elli context", le_elli);
	if (php_ctx == NULL) {
		RETURN_FALSE;
	}

	char *encrypted = elli_encrypt(php_ctx->ctx, key, data, &data_len);
	if (!encrypted) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to encrypt data: %s", elli_ctx_last_error(php_ctx->ctx));
		RETURN_FALSE;
	}
	RETVAL_STRINGL(encrypted, data_len);
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

static void elli_resource_dtor(zend_resource *rsrc) /* {{{ */
{
	php_elli_ctx_t *php_ctx = (php_elli_ctx_t *)rsrc->ptr;

	elli_ctx_free(php_ctx->ctx);
	efree(php_ctx);
}
/* }}} */

/* PHP_MINIT_FUNCTION {{{
 */
PHP_MINIT_FUNCTION(elli)
{
    le_elli = zend_register_list_destructors_ex(elli_resource_dtor, NULL, "elli context", module_number);
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
	PHP_FE(elli_ctx_create, NULL)
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
