/* elli extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_elli.h"

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
