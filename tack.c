/**Tack PHP extension: fast, flawless templating
 *
 * @author jon <jon@wroth.org>
 * @version 1.1
 *
 * a high-power, high-speed HTML-friendly
 * templating system for PHP.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * SYNOPSIS.
 *
 * string tk_parse(string in_template, array key_pairs)
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
 *   pattern ::= /\{\$[A-Z_0-9]+\}/
 *
 * 1. variables are encapsulated with brackets. { }
 * 2. variables begin with $.
 * 3. variables are entirely upper-case.
 * 3. variables are at least one alphanumeric or underscore.
 *
 *
 * key_pairs: an array of KEY => VALUE pairs where each KEY is
 * the name of a target template variable to be discovered in
 * the string in_template. unmatched targets are ignored.
 *
 *
 * RETURN VALUES.
 *
 * returns a copy of in_template with all instances of matched
 * template variables replaced with their associated value from
 * key_pairs, or NULL if left unspecified.
 *
 * returns FALSE if input parameters are of the wrong type.
 * returns NULL if in_template is zero-length.
 * returns in_template if key_pairs is zero-length.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_tack.h"


ZEND_DECLARE_MODULE_GLOBALS(tack)


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
  ZEND_INIT_MODULE_GLOBALS(tack, php_tack_init_globals, NULL);
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



static void php_tack_init_globals(zend_tack_globals *tack_globals)
{
  static char pattern[] = {"\\{\\$[A-Z_0-9]+\\}"};

  if (0 != regcomp(&(tack_globals->reg_state), pattern, REG_EXTENDED))
    php_printf("OH SHIT\n");

}



PHP_FUNCTION(tk_parse)
{
  static char pattern[] = {"\\{\\$[A-Z_0-9]+\\}"};

  regex_t state;

  regmatch_t match;

  HashTable *arr_hash;

  HashPosition arr_ptr;

  zval arr_string,
       *arr_key_pairs,        // this[key] = value
       *arr_key_pair_len,     // this[key] = strlen(value)
       *arr_parsed,           // this[] = output fragment
       *arr_parsed_len,       // this[] = strlen(fragment of same index)
       **arr_zvalue,          // value
       **arr_parsed_data_len;

  char *arr_key,
       *input_buf,            // template
       *output_buf;           // parsed data

  long i,
       arr_key_len,
       copy_dist,
       zval_type,
       regexec_ret,
       arr_string_len,
       arr_len,
       key_len,
       input_offset,
       input_buf_len,
       output_offset,
       output_buf_len;



  // check up on input data.
  //
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &input_buf, &input_buf_len, &arr_key_pairs) == FAILURE)
    RETURN_FALSE;

  if (input_buf_len == 0)
    RETURN_NULL();

  arr_hash = Z_ARRVAL_P(arr_key_pairs);
  arr_len = zend_hash_num_elements(arr_hash);

  if (arr_len == 0)
    RETURN_STRING(input_buf, 1);



  // init local stuff.
  //
  ALLOC_INIT_ZVAL(arr_key_pair_len);
  array_init(arr_key_pair_len);
  ALLOC_INIT_ZVAL(arr_parsed);
  array_init(arr_parsed);
  ALLOC_INIT_ZVAL(arr_parsed_len);
  array_init(arr_parsed_len);
  output_buf_len = input_buf_len;
  input_offset = 0;


  // compile regex. boohoo
  //
  //if (0 != regcomp(&state, pattern, REG_EXTENDED))
  //  RETURN_FALSE;


  while(1)
  {
    regexec_ret = regexec(&TACK_G(reg_state), input_buf + input_offset, 1, &match, 0);


    if ((regexec_ret == REG_NOMATCH) || (match.rm_so == -1))
    {
      // add what's left of the input string into the parsed array.
      //
      copy_dist = input_buf_len - input_offset;

#if 0
      fprintf(stderr, "finishing with %d chars\n", copy_dist);
#endif

      add_next_index_stringl(arr_parsed, (input_buf + input_offset), copy_dist, 1);
      add_next_index_long(arr_parsed_len, copy_dist);
      break;
    }

    // check if key is ok. get end_offset of input string to copy to.
    //   must exist in hash.
    //   must have string-convertible type.
    //
    copy_dist = match.rm_eo;
    arr_key_len = (match.rm_eo - match.rm_so) - 3;
    arr_key = (char *)emalloc(arr_key_len+1);
    memset((void *)arr_key, 0, arr_key_len+1);
    strncpy(arr_key, (input_buf + input_offset + match.rm_so + 2), arr_key_len);
#if 0
    fprintf(stderr,"match(%d):%s\n", arr_key_len, arr_key);
#endif
    if (zend_hash_find(arr_hash,
                        arr_key,
                        arr_key_len + 1,
                        (void **)&arr_zvalue) == SUCCESS)
    {
      zval_type = Z_TYPE_PP(arr_zvalue);

      switch(zval_type)
      {
        case IS_ARRAY:
        case IS_OBJECT:
        case IS_RESOURCE:
          break;

        default:
          copy_dist = match.rm_so;
          break;
      }

    }/*key exists?*/

    efree(arr_key);
#if 0
    fprintf(stderr, "copying %d chars\n", copy_dist);
#endif
    // put input_buf[0 .. end_at] into arr_parsed[].
    //
    add_next_index_stringl(arr_parsed, (input_buf + input_offset), copy_dist, 1);
    add_next_index_long(arr_parsed_len, copy_dist);


    // if the key exists,
    //
    if (copy_dist == match.rm_so)
    {
      // gotta copy arr_hash[arr_key] to cast it to a string.
      // add replacement value length to output_buf_len,
      // subtract the length of the key string.
      //  (arr_value_len[arr_key] - (rm_eo - rm_so) + 3)
      //
      //  TODO: cache arr_string_len with arr_key_pair_len<char *, long>
      //
      arr_string = **arr_zvalue;
      zval_copy_ctor(&arr_string);
      convert_to_string(&arr_string);
      arr_string_len = Z_STRLEN(arr_string);
#if 0
      fprintf(stderr, "old=%d ", output_buf_len);
#endif
      output_buf_len += (arr_string_len - (arr_key_len+3));
#if 0
      fprintf(stderr, "new=%d\n", output_buf_len);
#endif
      add_next_index_stringl(arr_parsed,  Z_STRVAL(arr_string), arr_string_len, 1);
      add_next_index_long(arr_parsed_len, arr_string_len);
      zval_dtor(&arr_string);

    }/*key exists?*/


    // jump ahead to get next result.
    //
    input_offset += match.rm_eo;

  }/*while*/


  // allocate output_buf_len chars.
  // implode arr_parsed[] into output_buf.
  //
#if 0
  fprintf(stderr, "allocating %d chars\n", output_buf_len+1);
#endif
  output_buf = (char *)emalloc(output_buf_len+1);
  memset((void *)output_buf, 0, output_buf_len+1);
  output_offset = 0;
  i = 0;

  // copy fragments into output buffer
  //
  for(zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arr_parsed), &arr_ptr);
      zend_hash_get_current_data_ex(Z_ARRVAL_P(arr_parsed), (void**)&arr_zvalue, &arr_ptr) == SUCCESS;
      zend_hash_move_forward_ex(Z_ARRVAL_P(arr_parsed), &arr_ptr))
  {
    // both output lists should be same length..
    //
    if (zend_hash_index_find(Z_ARRVAL_P(arr_parsed_len), i, (void **)&arr_parsed_data_len) == SUCCESS)
    {
      strncpy(output_buf + output_offset, Z_STRVAL_PP(arr_zvalue), Z_LVAL_PP(arr_parsed_data_len));
      output_offset += Z_LVAL_PP(arr_parsed_data_len);
    }
    i++;
  }

  // cleanup
  //
  regfree(&state);
  zval_dtor(arr_parsed);
  zval_dtor(arr_parsed_len);
  zval_dtor(arr_key_pair_len);


  // all set.
  //
  RETURN_STRING(output_buf, 0);
}
