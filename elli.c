/* elli extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_elli.h"

#include <elli.h>

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
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to encrypt data: %s", elli_ctx_last_error(ctx));
		RETURN_FALSE;
	}
	RETURN_STRING(encrypted);
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
	PHP_FE_END
};
/* }}} */

/* {{{ elli_module_entry
 */
zend_module_entry elli_module_entry = {
	STANDARD_MODULE_HEADER,
	"elli",					/* Extension name */
	elli_functions,			/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
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
