<?php
/**
 * tpl_parse
 *
 * this was the prototype version of the tack engine.
 *
 * transmogrifies a templated string into the desired output given an array of
 * key-value pairs.
 * each key represents a template variable from the input string.
 * variables left unspecified as a key-pair are ignored.
 * keys not present in the template string are ignored.
 *
 * @author jon <jon@wroth.org>
 * @version 2.4
 *
 * @param $html  the template to process, usually HTML, so let's
 *               call it that.
 * @param $vars  associated array of key-value pairs.
 *
 * @returns      string output of a fully (and swiftly) parsed
 *               template.
 */
function tpl_parse($html, $vars)
{
  $data = null;
  $st = 0;

  if (strlen($html) == 0) return null;
  if (count($vars) == 0) return $html;

  // regex match on {$FOO} etc
  //
  $c = preg_match_all('/\{\$([A-Z_0-9]+)\}/',
                      $html,
                      $m,
                      PREG_PATTERN_ORDER|PREG_OFFSET_CAPTURE);


  // substitute each template variable with a provided
  // value from $vars[$name].
  //
  for ($x = 0; $x < $c; $x++)
  {
    // get the value, or put the var back if there isn't one.
    //
    $val = array_key_exists($m[1][$x][0], $vars) ? $vars[$m[1][$x][0]] : '{$'.$m[1][$x][0].'}';

    // grab the HTML string up to the match we are parsing,
    // add the parsed value to the end.
    //
    $data .= substr($html, $st, $m[0][$x][1] - $st) . $val;

    // update the position to grab HTML from to just after the
    // template variable which was just parsed.
    //
    $st = $m[0][$x][1] + strlen($m[0][$x][0]);
  }

  // return all parsed data concatenated with the remaining HTML.
  //
  return $data . substr($html, $st);
}
?>
