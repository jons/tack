/**tack main header file
 *
 * @author jon <jon@wroth.org>
 * @version 1.0
 *
 * a high-power, high-speed HTML-friendly
 * templating system for PHP.
 *
 */

#ifndef PHP_TACK_H
#define PHP_TACK_H


// required reading
//
#include <sys/types.h>
#include <regex.h>


// thread safety (begin)
//
#ifdef ZTS
#include "TSRM.h"
#endif


// module info
//
#define PHP_TACK_VERSION "1.0"
#define PHP_TACK_EXTNAME "tack"


/* globals BAD
 *
ZEND_BEGIN_MODULE_GLOBALS(tack)
  regex_t reg_state;
ZEND_END_MODULE_GLOBALS(tack)


#ifdef ZTS
#define TACK_G(v) TSRMG(tack_globals_id, zend_tack_globals *, v)
#else
#define TACK_G(v) (tack_globals.v)
#endif
 *
 */


// function declarations
//
PHP_MINIT_FUNCTION(tack);
PHP_MSHUTDOWN_FUNCTION(tack);
PHP_RINIT_FUNCTION(tack);
PHP_FUNCTION(tk_parse);


// entry point
//
extern zend_module_entry tack_entry;
#define phpext_tack_ptr &tack_entry

#endif
