/**tack main header file
 *
 * @author jon <jon@wroth.org>
 * @version 0.1
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
#include <math.h>


// thread safety (begin)
//
#ifdef ZTS
#include "TSRM.h"
#endif


// module info
//
#define PHP_TACK_VERSION "0.1"
#define PHP_TACK_EXTNAME "tack"


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
