tack
====

<p>tack is the <i>correct</i> engine implementation for <i>any</i> templating
engine or substitution language.</p>

the problem
-----------

<p>since this is a PHP project, the explanation will stick to PHP
terminology.</p>

<p>in short, repeated calls to <code><b>str_replace</b></code>, or the use of
arrays for search and replace variables, suffers from what is noted as the
"replacement order gotcha" in the <a href="http://php.net/str_replace">PHP
documentation</a>.</p>

<p>suppose you have the text body <code>"cat"</code>, and you are to make two
substitutions:</p>

> b =&gt; m
>
> c =&gt; b

<p>with <code><b>str_replace<?b></code> your output is subject to implementation
detail because the order in which these substitutions are run on the text body
will affect the outcome. if the order is known then the algorithm can be taken
advantage of to do clever things, and all Real Programmers understand that this
is Good.</p>

<p>however, neither being clever nor being lazy is the correct behavior of a
templating engine, because is a particular use of general-purpose string
replacement.</p>


the solution
------------

<p>what is actually desired of a templating engine is behavior identical to
conway's game of life, where all cell states are resolved before any are
changed. the output string in our example should <i>always</i> be
<code>"bat"</code>, because the usual point of templating involves producing
markup which may clash with the templating language itself. <i>ordered
replacement may result in an intermediate state of the output which appears to
contain instructions to perform a substition which was not called for.</i></p>

<p>to accomplish this in v0.1 i have written the most basic regex replacement
algorithm which 1) does the work, 2) does it in a fashion adaptable to other
languages, 3) has no gimmicks; can be easily cache-accelerated. however this
is relatively old code, and only contains project management files sufficient
for PHP extensions at the time. good luck using it, i have no idea if it
can still be compiled as such.</p>
