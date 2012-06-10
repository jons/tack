PHP_ARG_ENABLE(tack, whether to enable support for the tack template functions,
[ --enable-tack   enable tack support])

if test "$PHP_TACK" = "yes"; then
  AC_DEFINE(HAVE_TACK, 1, [whether you have tack])
  PHP_NEW_EXTENSION(tack, tack.c, $ext_shared)
fi
