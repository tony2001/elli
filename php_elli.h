/* elli extension for PHP */

#ifndef PHP_ELLI_H
# define PHP_ELLI_H

extern zend_module_entry elli_module_entry;
# define phpext_elli_ptr &elli_module_entry

# define PHP_ELLI_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_ELLI)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_ELLI_H */
