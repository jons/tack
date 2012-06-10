/**Tack PHP extension: fast, flawless templating
 *
 * @author jon <jon@wroth.org>
 * @version 0.2.1
 *
 * a high-power, high-speed HTML-friendly
 * templating system for PHP.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * SYNOPSIS.
 *
 * string tk_parse(string in_template, array key_pairs[, boolean keep_unparsed_vars])
 *
 * PARAMETERS.
 *
 * in_template: a string of text containing any number of
 * "template variables". these must conform to the regex declared
 * in the header of tk_parse(), described below. a match on a
 * template variable that is not later found in key_pairs is
 * eliminated from the output string, unless keep_unparsed_vars
 * is true.
 *
 *   pattern ::= /\{\$[A-Z_]{1}[A-Z_0-9]*\}/
 *
 * 1. variables are encapsulated with brackets. { }
 * 2. variables begin with $.
 * 3. variables are entirely upper-case.
 * 4. the first letter of a variable is alphabetic only,
 * 5. following characters may be alphanumeric or an underscore.
 *
 *
 * key_pairs: an array of KEY => VALUE pairs where each KEY is
 * the name of a target template variable to be discovered in
 * the string in_template. unmatched targets are ignored.
 *
 *
 * keep_unparsed_vars: if set to true, tack will not ignore
 * template variables found in in_template which are not present
 * in key_pairs. instead, it will leave them in the output.
 * the default behavior (false) is to ignore these variables.
 *
 *
 * RETURN VALUES.
 *
 * returns a copy of in_template with all instances of matched
 * template variables replaced with their associated value from
 * key_pairs, or NULL if left unspecified.
 *
 * returns FALSE if input parameters are of the wrong type.
 *
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * TODO:
 *
 *  - as a key is substituted, cache its value locally for
 *    the duration of the function (worthwhile? overrides next
 *    item on list: cache of "KEY" is either "{$KEY}", NULL,
 *    or string value located in key_pairs[KEY]).
 *
 *  - reorganize code to shift rm_so to rm_eo when an array key
 *    is not found and keep_unparsed_vars is true (this avoids
 *    an extra set of calls to output_realloc and strncpy).
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



/**reallocate the output buffer. use provided multiplier
 * to "guess" at how much more space we might need.
 *
 * return a pointer into the new output buffer at the last
 * position where we can continue copying more values in.
 *
 */
char *output_realloc(char **retval,
                     int *retval_len_total,
                     int retval_len_used,
                     const int source_len)
{
  char *retval_buf_new;


#ifdef DEBUG
  php_printf("output_realloc(): used=%d old=%d ", retval_len_used, *retval_len_total);
#endif

  // calc new maximum length
  //
  *retval_len_total = floor(retval_len_used * REALLOC_MULT_USED) + source_len + 1;

#ifdef DEBUG
  php_printf("new=%d\n", *retval_len_total);
#endif

  // allocate new buffer and clear
  //
  retval_buf_new = (char *)emalloc(*retval_len_total);
  memset((void *)retval_buf_new, 0, *retval_len_total);


  // copy old buffer to new one
  //
  strncpy(retval_buf_new, *retval, retval_len_used);


  // free old buffer
  //
  efree(*retval);


  // update pointer
  //
  *retval = retval_buf_new;


  // return pointer into output buffer where we are now.
  //
  return *retval + retval_len_used;
}



/** parse input template.
 *  param1 string:  template string
 *  param2 array:   template vars=>values
 *  param3 boolean: 
 *  return parsed string (i like string)
 */
PHP_FUNCTION(tk_parse)
{
  static char regex_pattern[] = {"\\{\\$[A-Z_]{1}[A-Z_0-9]*\\}"};

  regex_t reg_state;

  regmatch_t reg_match;

  HashTable *arr_hash;

  HashPosition pointer;

  zend_bool keep_unparsed_vars = 0;

  zval **data,
       *arr_keypairs,
       arr_zvalue;

  char *arr_key,
       *buffer,
       *buf_ptr,
       *repl_buf_ptr,
       *retval_buf,
       *retval_buf_ptr;

  long longest_value_len;

  int i,
      reg_compile_retval,
      reg_execute_retval,
      retval_buf_len_used,
      retval_buf_len_total,
      arr_len,
      key_len,
      buffer_len,
      repl_buf_len,
      keypairs_len;


  // parse params
  //
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|b", &buffer, &buffer_len, &arr_keypairs, &keep_unparsed_vars) == FAILURE)
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


    // is there anything to add to the output buffer that comes before
    // the template variable?
    //
    if (reg_match.rm_so > 0)
    {
      // and can we fit these chars in the output buffer?
      //
      if ((retval_buf_len_used + reg_match.rm_so) >= retval_buf_len_total)
      {
        // no, out of output buffer space, reallocate.
        //
        retval_buf_ptr = output_realloc(&retval_buf,
                                        &retval_buf_len_total,
                                        retval_buf_len_used,
                                        reg_match.rm_so);
      }

      // copy data into output buffer
      //
      strncpy(retval_buf_ptr, buf_ptr, reg_match.rm_so);
      retval_buf_len_used += reg_match.rm_so;
      retval_buf_ptr += reg_match.rm_so;
    }


    // now get the matched key out of the input buffer
    //
    // extract key from position (buf_ptr+rm_so+2) with
    // length (rm_so - rm_eo):
    // {$ABCDEFG}0, add 1 to make space for trailing char
    // {$ABCDEFG}0, allocate len(this) chars, init to zero
    // 0000000000, copy len-5 chars to key
    // ABCDEFG000
    //
    key_len = (reg_match.rm_eo - reg_match.rm_so) + 1;
    arr_key = (char *)emalloc(key_len);
    memset((void *)arr_key, 0, key_len);
    strncpy(arr_key, (buf_ptr+reg_match.rm_so+2), key_len - 4);

#ifdef DEBUG
    php_printf("\nkey:^%s^%d^\n", arr_key, key_len);
#endif

    // look key up in arr_hash
    //
    if (zend_hash_find(arr_hash,
                       arr_key,
                       key_len - 3,
                       (void **)&data) == SUCCESS)
    {
#ifdef DEBUG
      php_printf("found key in array!\n");
#endif

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
#ifdef DEBUG
          php_printf("i can work with this type.\n");
#endif

          arr_zvalue = **data;
          zval_copy_ctor(&arr_zvalue);
          convert_to_string(&arr_zvalue);

          // add to output buffer here.
          //
          repl_buf_ptr = Z_STRVAL(arr_zvalue);
          repl_buf_len = Z_STRLEN(arr_zvalue);

#ifdef DEBUG
          php_printf("my string: ^");
          PHPWRITE(repl_buf_ptr, repl_buf_len);
          php_printf("^ len=%d\n", repl_buf_len);
#endif

          // again, reallocate if too long.
          //
          if ((retval_buf_len_used + repl_buf_len) >= retval_buf_len_total)
          {
            retval_buf_ptr = output_realloc(&retval_buf,
                                            &retval_buf_len_total,
                                            retval_buf_len_used,
                                            repl_buf_len);
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
    else
    {
      // the key wasn't in the input array. we're going to leave the
      // template variable hidden unless told otherwise.
      //
      // TODO: optimize: we don't ever want to look at that key again,
      // but we will want to do keep the unparsed var in the output again.
      //
      if (keep_unparsed_vars)
      {
#ifdef DEBUG
        php_printf("key not found. using var ");
#endif

        // ABCDEFG0000, shift key right two chars
        // ABABCDEFG00, set brackets and $
        // {$ABCDEFG}0
        //
        for(i = key_len-2; i > 1; i--)
          arr_key[i] = arr_key[i-2];

        arr_key[0] = '{';
        arr_key[1] = '$';
        arr_key[key_len-2] = '}';

#ifdef DEBUG
        php_printf("^%s^ %x\n", arr_key, arr_key[key_len-1]);
#endif

        // key is ready to be added to output, do we reallocate?
        //
        if ((retval_buf_len_used + key_len) >= retval_buf_len_total)
        {
          retval_buf_ptr = output_realloc(&retval_buf,
                                          &retval_buf_len_total,
                                          retval_buf_len_used,
                                          key_len);
        }

        // copy original template var into output buffer
        //
        strncpy(retval_buf_ptr, arr_key, key_len-1);
        retval_buf_len_used += (key_len-1);
        retval_buf_ptr += (key_len-1);
      }
    }

    // done with the key.
    //
    efree(arr_key);


    // move pointers forward in source buffer,
    // to start right after the template var ends.
    //
    buf_ptr += reg_match.rm_eo;
    buffer_len -= reg_match.rm_eo;
  }//loop


#ifdef DEBUG
  php_printf("leftover: old=%d new=%d\n", buffer_len, strlen(buf_ptr));
#endif


  // if there's input string left..
  //
  if (buffer_len > 0)
  {
    // ..see if we need more space for it..
    //
    if ((retval_buf_len_used + buffer_len) >= retval_buf_len_total)
    {
      // reallocate one last time. maybe.
      // this should really be an inline function.
      //
      retval_buf_ptr = output_realloc(&retval_buf,
                                      &retval_buf_len_total,
                                      retval_buf_len_used,
                                      buffer_len);
    }

    // ..then copy the last of it into output buffer.
    //
    strncpy(retval_buf_ptr, buf_ptr, buffer_len);
    retval_buf_len_used += buffer_len;
    retval_buf_ptr += buffer_len;
  }

  // report memory overhead
  //
#ifdef DEBUG
  php_printf("total buffer: %d\n", retval_buf_len_total);
  php_printf("used buffer:  %d\n", retval_buf_len_used);
#endif

  RETURN_STRING(retval_buf, 0);
}
