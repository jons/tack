/**Tack PHP extension: fast, flawless templating
 *
 * @author jonathan shusta
 * @version 0.1
 *
 * a high-power, high-speed HTML-friendly
 * templating system for PHP.
 *
 * contact: <jon|veganmeansnospam.wrothorg>
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * SYNOPSIS.
 *
 * string tk_parse(string in_template, array key_pairs)
 *
 * PARAMETERS.
 *
 * in_template: a string of text containing any number of
 * "template variables". these must conform to the regex
 * declared in the header of tk_parse(), described below.
 * a match on a template variable that is not later found
 * in key_pairs is eliminated from the output string (see
 * TODO).
 *
 *   pattern ::= /\{\$[A-Z_]{1}[A-Z_0-9]*\}/
 *
 * 1. variables are encapsulated with brackets. { }
 * 2. variables begin with $.
 * 3. variables are entirely upper-case.
 * 4. the first letter of a variable is alphabetic only,
 * 5. following characters may be alphanumeric or an
 *    underscore.
 *
 * key_pairs: an array of KEY => VALUE pairs where each KEY
 * is the name of a target template variable to be discovered
 * in the string in_template. unmatched targets are ignored.
 *
 *
 * RETURN VALUES.
 *
 * returns a copy of in_template with all instances of
 * matched template variables replaced with their associated
 * value from key_pairs, or NULL if left unspecified.
 *
 * returns FALSE if input parameters are of the wrong type.
 *
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * TODO:
 *  - do not replace {$FOO} if FOO is not found in array.
 *    (optional? moves code around)
 *
 *  - as a key is substituted, cache its value locally for
 *    the duration of the function. (worthwhile?)
 *
 *
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_tack.h"


static function_entry tack_functions[] = {
    PHP_FE(tk_parse, NULL)
    {NULL, NULL, NULL}
};


zend_module_entry tack_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_TACK_EXTNAME,
    tack_functions,
    PHP_MINIT(tack),
    PHP_MSHUTDOWN(tack),
    PHP_RINIT(tack),
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_TACK_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_TACK
ZEND_GET_MODULE(tack)
#endif



PHP_MINIT_FUNCTION(tack)
{
  return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(tack)
{
  return SUCCESS;
}


PHP_RINIT_FUNCTION(tack)
{
  return SUCCESS;
}


PHP_FUNCTION(tk_parse)
{
  static char regex_pattern[] = {"\\{\\$[A-Z_]{1}[A-Z_0-9]*\\}"};

  regex_t reg_state;

  regmatch_t reg_match;

  HashTable *arr_hash;

  HashPosition pointer;

  zval **data,
       *arr_keypairs,
       arr_zvalue;

  char *arr_key,
       *buffer,
       *buf_ptr,
       *repl_buf_ptr,
       *retval_buf,
       *retval_buf_new,
       *retval_buf_ptr;

  long longest_value_len;

  int reg_compile_retval,
      reg_execute_retval,
      retval_buf_len_used,
      retval_buf_len_total,
      arr_len,
      key_len,
      buffer_len,
      repl_buf_len,
      keypairs_len,
      buf_ptr_offset;


  // parse params
  //
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &buffer, &buffer_len, &arr_keypairs) == FAILURE)
    RETURN_FALSE;


  // setup:
  //  get hash ref to arr_keypairs
  //  get length of arr_keypairs
  //  compile regex state
  //  init buffer pointer
  //  zero length counters
  //
  arr_hash = Z_ARRVAL_P(arr_keypairs);
  arr_len = zend_hash_num_elements(arr_hash);
  reg_compile_retval = regcomp(&reg_state, regex_pattern, REG_EXTENDED);
  buf_ptr = buffer;
  buf_ptr_offset = 0;
  longest_value_len = 0;
  retval_buf_len_used = 0;


  // guess at the size of the output buffer
  //
  for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer);
      zend_hash_get_current_data_ex(arr_hash, (void**)&data, &pointer) == SUCCESS;
      zend_hash_move_forward_ex(arr_hash, &pointer))
    if (Z_TYPE_PP(data) == IS_STRING)
      if (Z_STRLEN_PP(data) > longest_value_len)
        longest_value_len = Z_STRLEN_PP(data);


  // allocate the output buffer
  //
  retval_buf_len_total = buffer_len + (longest_value_len * arr_len);
  retval_buf = (char *)emalloc(retval_buf_len_total);
  memset((void *)retval_buf, 0, retval_buf_len_total);
  retval_buf_ptr = retval_buf;


  // start parsing the input string for template variables.
  //
  while(1)
  {
    reg_execute_retval = regexec(&reg_state, buf_ptr, 1, &reg_match, 0);

    if (reg_execute_retval == REG_NOMATCH)
      break;

    if (reg_match.rm_so == -1)
      break;


    // add everything up to reg_match.rm_so to the output buffer
    // increment the output buffer length
    // (implicit: decrement the output buffer remaining length)
    //
    if (reg_match.rm_so > 0)
    {
      if ((retval_buf_len_used + reg_match.rm_so) >= retval_buf_len_total)
      {
        // out of output buffer space! reallocate. sigh.
        //
        retval_buf_len_total = floor(retval_buf_len_used * 1.5) + reg_match.rm_so;
        retval_buf_new = (char *)emalloc(retval_buf_len_total);
        memset((void *)retval_buf_new, 0, retval_buf_len_total);
        strncpy(retval_buf_new, retval_buf, retval_buf_len_used);
        efree(retval_buf);
        retval_buf = retval_buf_new;
        retval_buf_ptr = (retval_buf + retval_buf_len_used);
      }

      // copy pre-template crap into output buffer
      //
      strncpy(retval_buf_ptr, buf_ptr, reg_match.rm_so);
      retval_buf_len_used += reg_match.rm_so;
      retval_buf_ptr += reg_match.rm_so;
    }



    // extract key from position (buf_ptr+rm_so+2),
    // with length (rm_so - rm_eo)
    // {$ABCDEFG}, subtract 2 to eliminate brackets:
    // $ABCDEFG, allocate len(this) chars, init to zero
    // 00000000, copy len-1 chars to key
    // ABCDEFG\0
    //
    key_len = (reg_match.rm_eo - reg_match.rm_so) - 2;
    arr_key = (char *)emalloc(key_len);
    memset((void *)arr_key, 0, key_len);
    strncpy(arr_key, (buf_ptr+reg_match.rm_so+2), key_len-1);

    php_printf("\nkey:^%s^%d^\n", arr_key, key_len);


    // look key up in arr_hash
    //
    if (zend_hash_find(arr_hash,
                       arr_key,
                       key_len /* + 1 */,
                       (void **)&data) == SUCCESS)
    {
      php_printf("found key in array!\n");

      // if we found a value we can easily convert
      // to a string, do so, and add it to the
      // output buffer. otherwise silently ignore it.
      //
      switch (Z_TYPE_PP(data))
      {
        case IS_ARRAY:
        case IS_RESOURCE:
        case IS_OBJECT:
          break;

        default:
          php_printf("i can work with this type.\n");

          arr_zvalue = **data;
          zval_copy_ctor(&arr_zvalue);
          convert_to_string(&arr_zvalue);

          // add to output buffer here.
          //
          repl_buf_ptr = Z_STRVAL(arr_zvalue);
          repl_buf_len = Z_STRLEN(arr_zvalue);

          php_printf("my string: ^");
          PHPWRITE(repl_buf_ptr, repl_buf_len);
          php_printf("^ len=%d\n", repl_buf_len);

          // again, reallocate if too long.
          //
          if ((retval_buf_len_used + repl_buf_len) >= retval_buf_len_total)
          {
            retval_buf_len_total = floor(retval_buf_len_used * 1.5) + repl_buf_len;
            retval_buf_new = (char *)emalloc(retval_buf_len_total);
            memset((void *)retval_buf_new, 0, retval_buf_len_total);
            strncpy(retval_buf_new, retval_buf, retval_buf_len_used);
            efree(retval_buf);
            retval_buf = retval_buf_new;
            retval_buf_ptr = (retval_buf + retval_buf_len_used);
          }

          // copy template var replacement into output buffer
          //
          strncpy(retval_buf_ptr, repl_buf_ptr, repl_buf_len);
          retval_buf_len_used += repl_buf_len;
          retval_buf_ptr += repl_buf_len;

          zval_dtor(&arr_zvalue);
          break;
      }
    }
    efree(arr_key);


    // move pointers forward in source buffer,
    // to start right after the template var ends.
    //
    buf_ptr += reg_match.rm_eo;
    buf_ptr_offset += reg_match.rm_eo;
  }


  // concat the last of buf_ptr into the output string
  //
  if (strlen(buf_ptr) > 0)
  {
    if ((retval_buf_len_used + strlen(buf_ptr)) >= retval_buf_len_total)
    {
      // reallocate one last time. maybe.
      // this should really be an inline function.
      //
      retval_buf_len_total = retval_buf_len_used + strlen(buf_ptr) + 1;
      retval_buf_new = (char *)emalloc(retval_buf_len_total);
      memset((void *)retval_buf_new, 0, retval_buf_len_total);
      strncpy(retval_buf_new, retval_buf, retval_buf_len_used);
      efree(retval_buf);
      retval_buf = retval_buf_new;
      retval_buf_ptr = (retval_buf + retval_buf_len_used);
    }

    // copy last of template into output buffer
    //
    strncpy(retval_buf_ptr, buf_ptr, strlen(buf_ptr));
  }

  RETURN_STRING(retval_buf, 0);
}
