tack
====

<p>tack is the <i>correct</i> engine implementation for <i>any</i> templating
engine or substitution language.</p>

<ul>
<li>v1.1 - fix leaks, and misc. cleanup</li>
<li>v1.0 - allocate exact output buffer size in constant time</li>
<li>v0.2 - improved memory reallocation, option to keep unparsed variables</li>
<li>v0.1 - proof of concept</li></ul>

the problem
-----------

<p>since this is a php project, the explanation will stick to php
terminology.</p>

<p>in short, repeated calls to <code><b>str_replace</b></code>, or the use of
arrays for search and replace variables, suffers from what is noted as the
"replacement order gotcha" in the <a href="http://php.net/str_replace">php
documentation</a>.</p>

<p>suppose you have the text body <code>"cat"</code>, and you are to make two
substitutions:</p>
<blockquote>b =&gt; m<br />c =&gt; b</blockquote>

<p>with <code><b>str_replace</b></code> your output is subject to implementation
detail because the order in which these substitutions are run on the text body
will affect the outcome. if the order is known then the algorithm can be taken
advantage of to do clever things, and all Real Programmers understand that this
is Good -- but it is not desired here.</p>


the solution
------------

<p>what is actually desired of a templating engine is behavior identical to
conway's game of life, where all cell states are resolved before any are
changed. the output string in our example should <i>always</i> be
<code>"bat"</code>, because the usual point of templating involves producing
markup which may clash with the templating language itself. <i>ordered
replacement may result in an intermediate state of the output which appears to
contain instructions to perform a substition which was not called for.</i></p>

<p>in our simple example, <code>"bat"</code> is actually an intermediate state
when substitutions are made last-to-first, and when the call returns the output
is <code>"mat"</code>. in practice, this will never be so benign; murphy's law
dictates that the output is a damaging code-generated script that erases your
hard disk, or hideous mangled html that gets you fired from your job as a guy
that doesn't create mangled html.</p>

<p>if you are here trying to find a better templating engine because templating
languages are too close to the domain language of the outputs you are dealing
with, or because you tried to substitute in the contents of a template itself,
you know exactly what i'm talking about.</p>

<p>this is relatively old code, and only contains project management files
sufficient for php extensions at the time. good luck using it, i have no idea
if it can still be compiled as such.</p>


a modest footnote
-----------------

<p>this project was conceived and created in january 2006. i am sure the authors
of other php templating systems have figured out by now that they had this
problem, and have corrected it, but at the time nobody had yet done so.</p>
