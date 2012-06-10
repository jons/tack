/**tack main header file
 *
 * @author jon <jon@wroth.org>
 * @version 0.2.1
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
#define PHP_TACK_VERSION "0.2.1"
#define PHP_TACK_EXTNAME "tack"


// multiply currently allocated output buffer by this size
// when reallocating. a higher value ensures that the parse
// function calls output_realloc() less often, but uses more
// memory and takes more time to perform a realloc.
//
// TODO: deprecate this and replace with an algorithm, no
// slower than O(n), which allocates the output buffer
// exactly once.
//
#define REALLOC_MULT_USED 1.8



// function declarations
//
PHP_MINIT_FUNCTION(tack);
PHP_MSHUTDOWN_FUNCTION(tack);
PHP_RINIT_FUNCTION(tack);
PHP_FUNCTION(tk_parse);

char *output_realloc(char **retval,
                     int *retval_len_total,
                     int retval_len_used,
                     const int source_len);


// entry point
//
extern zend_module_entry tack_entry;
#define phpext_tack_ptr &tack_entry

#endif
